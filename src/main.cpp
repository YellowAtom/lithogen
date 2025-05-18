#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <nfd_glfw3.h>
#include <numeric>
#include <stb_image.h>
#include "compilation.h"
#include "config.h"
#include "constants.h"
#include "image.h"
#include "render.h"

// A simple function to send the image data to the gpu and return the pointer.
GLuint LoadTexture(const stbi_uc* image, const int width, const int height) {
	GLuint imageTexture = 0;
	glGenTextures(1, &imageTexture);
	glBindTexture(GL_TEXTURE_2D, imageTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	return imageTexture;
}

void ImportButton(GLFWwindow* window, Image& image, Config* config) {
	constexpr nfdu8filteritem_t filters[1] = {
		{"Images", "jpg,jpeg,png,tga,bmp,psd,gif,hdr,pic"},
	};

	nfdopendialogu8args_t args = {};
	args.filterList = filters;
	args.filterCount = 1;

	NFD_GetNativeWindowFromGLFWWindow(window, &args.parentWindow);

	nfdu8char_t* outPath = nullptr;

	if (const nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args); result != NFD_OKAY) {
		if (result == NFD_ERROR) {
			std::cout << "Error: " << NFD_GetError() << '\n';
		}

		return;
	}

	if (image.data != nullptr) {
		stbi_image_free(image.data);
	}

	// Load the image from storage, it will automatically process any of the supported formats.
	image.data = stbi_load(outPath, &image.width, &image.height, nullptr, 4);

	if (image.data == nullptr) {
		std::cout << "Failed to load image!\n";
		return;
	}

	// Calculate the information required to gather aspect ratio based sizing.
	const int aspectGcd = std::gcd(image.width, image.height);
	image.aspectRatioW = image.width / aspectGcd;
	image.aspectRatioH = image.height / aspectGcd;

	// Ensure the default width is the correct aspect ratio and reset the size when a new image is
	// loaded.
	config->sliderHeight = 100.0F;
	config->sliderWidth = 100.0F * image.aspectRatioW / image.aspectRatioH;

	if (image.texture != 0) {
		glDeleteTextures(1, &image.texture); // Ensure the previous image has been cleaned up.
	}

	// Send the image to the GPU for the renderer to preview.
	image.texture = LoadTexture(image.data, image.width, image.height);
	// Clear the file path from memory as we are done with it.
	NFD_FreePathU8(outPath);
}

void ExportButton(GLFWwindow* window, const Model& model) {
	if (model.indices.empty()) {
		return;
	}

	nfdsavedialogu8args_t args = {};
	args.filterCount = 0;
	args.defaultName = "lithophane.stl";

	NFD_GetNativeWindowFromGLFWWindow(window, &args.parentWindow);

	nfdu8char_t* outPath = nullptr;

	if (const nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args); result != NFD_OKAY) {
		if (result == NFD_ERROR) {
			std::cout << "Error: " << NFD_GetError() << '\n';
		}

		return;
	}

	WriteModel(outPath, model);
	NFD_FreePathU8(outPath);
}

// GLFW callbacks.

struct glfwUserData {
	bool cursorWithinViewport = false;

	Config* config;
	Render* render;

	glfwUserData(Config* config, Render* render) : config(config), render(render) {}
};

void FramebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
	const auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));
	// Recalculate viewport size when the window is resized.
	data->render->CalcViewport(width, height);
}

void KeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
	const auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	if (!data->render->entity.HasModel()) {
		return;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		data->render->entity.ResetRotation();
	} else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		data->config->drawWireframe = !data->config->drawWireframe;
		data->render->UpdateWireframe();
	}
}

void CursorPosCallback(GLFWwindow* window, const double x, const double y) {
	auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	if (!data->render->entity.HasModel()) {
		return;
	}

	// TODO: Rotating the cube too much will make this inaccurate, find a way.

	// Store previous mouse location to calculate difference.
	// Static allows a variable to persist through multiple function calls.
	static double previousX = 0;
	static double previousY = 0;

	int width = 0;
	int height = 0;
	glfwGetWindowSize(window, &width, &height);

	data->cursorWithinViewport =
		width - x < data->render->GetViewportWidth() && height - y < data->render->GetViewportHeight();

	// Is left mouse down and is the cursor within the viewport.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GL_TRUE && data->cursorWithinViewport) {
		// Apply the difference between the previous and current mouse location to the entity's rotation.
		data->render->entity.Rotate(glm::vec3(-(y - previousY), x - previousX, 0));
	}

	previousX = x;
	previousY = y;
}

void ScrollCallback(GLFWwindow* window, const double x, const double y) {
	const auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	// Move 4mm per scroll.
	if (data->render->entity.HasModel() && data->cursorWithinViewport) {
		data->render->camera.Move(glm::vec3(0, 0, y * 4));
	}
}

int main(int argc, char* argv[]) {
	// Initialize the GLFW3 library.
	if (glfwInit() == 0) {
		return 1;
	}

	NFD_Init();

	// The target OpenGL version (4.0).
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Avoids old and backwards compatible OpenGL features.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Initialize the single GLFW3 window used by this application, this also initializes an OpenGL context.
	GLFWwindow* mainWindow =
		glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "LithoGen", nullptr, nullptr);

	if (mainWindow == nullptr) {
		std::cout << "Failed to create GLFW window!\n";
		return 1;
	}

	// Set the OpenGL context of this window as active for this thread.
	glfwMakeContextCurrent(mainWindow);

	// Load OpenGL for glad and avoid glad doing what GLFW3 has already done by attaching them.
	if (gladLoadGL(glfwGetProcAddress) == 0) {
		return 1;
	}

	// Don't draw the backside of a triangle.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Enable the depth buffer.
	glEnable(GL_DEPTH_TEST);

	// Set OpenGL clear render colour, the colour drawn if nothing else is.
	// Dark Gray to be more distinct than black.
	glClearColor(0.15F, 0.15F, 0.15F, 1.0F);

	// Must happen before ImGui initialisation.
	glfwSetFramebufferSizeCallback(mainWindow, FramebufferSizeCallback);
	glfwSetKeyCallback(mainWindow, KeyCallback);
	glfwSetCursorPosCallback(mainWindow, CursorPosCallback);
	glfwSetScrollCallback(mainWindow, ScrollCallback);

	// ImGui initialisation.
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(mainWindow, true); // Attach ImGui to GLFW3.
	ImGui_ImplOpenGL3_Init("#version 400"); // Configure GLSL version to match OpenGL 4.0.

	// ImGui configuration.
	ImGui::StyleColorsDark(); // Set ImGui colours to dark mode.
	ImGuiIO& io = ImGui::GetIO();
	(void)io; // Struct to access various ImGui features and configuration.
	io.IniFilename = nullptr; // Disable saving ImGui state, unneeded in this implementation.

	// It is better to let the kernel clean these up as the program will close faster.
	auto* config = new Config();
	auto* render = new Render(mainWindow, config);
	auto* glfwUser = new glfwUserData(config, render);

	// Make this object accessible from within any GLFW callback.
	glfwSetWindowUserPointer(mainWindow, glfwUser);

	// The currently mounted image and model.
	Image image;
	Model model;

	while (glfwWindowShouldClose(mainWindow) == 0) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color and depth buffer to avoid any junk.

		render->Draw();

		// Prepare ImGui at the start of a new frame.
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Menu bar
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Import")) {
					ImportButton(mainWindow, image, config);
				}
				if (ImGui::MenuItem("Export")) {
					ExportButton(mainWindow, model);
				}
				if (ImGui::MenuItem("Quit", "Alt+F4")) {
					glfwSetWindowShouldClose(mainWindow, GL_TRUE);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View")) {
				ImGui::MenuItem("Show Source", nullptr, &config->drawSource);
				ImGui::Separator();
				ImGui::MenuItem("Show Preview", nullptr, &config->drawPreview);
				if (ImGui::MenuItem("Wireframe Preview", "W", &config->drawWireframe)) {
					render->UpdateWireframe();
				}
				if (ImGui::MenuItem("Reset Preview", "R")) {
					render->entity.ResetRotation();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("About")) {
					config->aboutOpened = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Gather the window height.
		int width = 0;
		int height = 0;
		glfwGetWindowSize(mainWindow, &width, &height);

		// Push the sidepanel down to make room for the menu bar.
		ImGui::SetNextWindowPos(ImVec2(0, GUI_MENUBAR_HEIGHT));
		ImGui::SetNextWindowSize(ImVec2(GUI_SIDEPANEL_WIDTH, height - GUI_MENUBAR_HEIGHT));

		ImGui::Begin("SidePanel", nullptr,
		             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
		                 ImGuiWindowFlags_NoTitleBar);

		if (image.data == nullptr) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5F);
		}

		ImGui::SeparatorText("Image Processing");

		// TODO: Implement difference kinds of grayscale processing. Currently we are only doing luminance.
		ImGui::Text("Grayscale Preference");
		ImGui::SliderFloat("Red", &config->sliderGsPref[0], 0.0F, 1.0F, SLIDER_FLOAT_FORMAT,
		                   ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Green", &config->sliderGsPref[1], 0.0F, 1.0F, SLIDER_FLOAT_FORMAT,
		                   ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Blue", &config->sliderGsPref[2], 0.0F, 1.0F, SLIDER_FLOAT_FORMAT,
		                   ImGuiSliderFlags_AlwaysClamp);

		ImGui::Text("Alpha Thickness");
		ImGui::SliderFloat("Alpha", &config->sliderGsPref[3], 0.0F, 1.0F, SLIDER_FLOAT_FORMAT,
		                   ImGuiSliderFlags_AlwaysClamp);

		ImGui::SeparatorText("Mesh Configuration");

		ImGui::Text("Dimensions");

		// TODO: The forced ratio does not clamp.
		if (ImGui::SliderFloat("Width", &config->sliderWidth, SLIDER_WIDTH_MIN, SLIDER_WIDTH_MAX,
		                       SLIDER_FLOAT_FORMAT_MM)) {
			config->sliderHeight = config->sliderWidth * image.aspectRatioH / image.aspectRatioW;
		}

		if (ImGui::SliderFloat("Height", &config->sliderHeight, SLIDER_HEIGHT_MIN, SLIDER_HEIGHT_MAX,
		                       SLIDER_FLOAT_FORMAT_MM)) {
			config->sliderWidth = config->sliderHeight * image.aspectRatioW / image.aspectRatioH;
		}

		ImGui::Text("Thickness");

		if (ImGui::SliderFloat("Min", &config->sliderThickMin, SLIDER_THICK_MIN, SLIDER_THICK_MAX,
		                       SLIDER_FLOAT_FORMAT_MM)) {
			if (config->sliderThickMin > config->sliderThickMax) {
				config->sliderThickMax = config->sliderThickMin;
			}
		}

		if (ImGui::SliderFloat("Max", &config->sliderThickMax, SLIDER_THICK_MIN, SLIDER_THICK_MAX,
		                       SLIDER_FLOAT_FORMAT_MM)) {
			if (config->sliderThickMax < config->sliderThickMin) {
				config->sliderThickMin = config->sliderThickMax;
			}
		}

		ImGui::Spacing();
		if (ImGui::Button("Compile")) {
			CompileModel(model, config, image);
			render->entity.LoadModel(model);

			// Offset the position by the centre offset.
			render->entity.SetPosition(-model.centerOffset);

			// TODO: Add visual error if compile fails.
		}

		if (image.data == nullptr) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::End();

		if (config->drawSource && image.texture != 0) {
			// Place the source preview in the bottom right corner.
			// Add 10 pixels of padding to the window edge.
			float previewWidth = 128.0F;
			float previewHeight = 128.0F;

			// Scale around the larger side.
			if (image.aspectRatioW < image.aspectRatioH) {
				previewHeight = 128.0F * image.aspectRatioH / image.aspectRatioW;
			} else {
				previewWidth = 128.0F * image.aspectRatioW / image.aspectRatioH;
			}

			ImGui::SetNextWindowPos(ImVec2(width - previewWidth - 10.0F, height - previewHeight - 10.0F));
			ImGui::SetNextWindowSize(ImVec2(previewWidth, previewHeight));

			// Remove padding as it is only a problem for this panel.
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));

			ImGui::Begin("Source Preview", nullptr,
			             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
			                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
			ImGui::Image(image.texture, ImVec2(previewWidth, previewHeight));
			ImGui::End();

			ImGui::PopStyleVar();
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center);

		if (config->aboutOpened) {
			ImGui::Begin("About", &config->aboutOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::Text("Version: 1.0");
			ImGui::End();
		}

		// Trigger an ImGui render.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Push the prepared frame buffer to the screen.
		glfwSwapBuffers(mainWindow);
		// Process the OS's window events, in other words, gathering inputs and window state from the OS.
		glfwPollEvents();
	}

	// Cleanup
	glfwTerminate();
	NFD_Quit();

	return 0;
}