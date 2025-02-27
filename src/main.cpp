
#include <iostream>
#include <numeric>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <nfd_glfw3.h>
#include <stb_image.h>

#include "constants.h"
#include "render.h"
#include "config.h"
#include "image.h"
#include "compilation.h"

#ifdef OS_WINDOWS
#include <windows.h>
#include <dwmapi.h>
#include <versionhelpers.h>
#include "glfw/glfw3native.h"
#endif

nfdu8char_t* FileSelectImage(GLFWwindow* window) {
	NFD_Init();

	constexpr nfdu8filteritem_t filters[1] = {
		{ "Images", "jpg,jpeg,png,tga,bmp,psd,gif,hdr,pic" },
	};

	nfdopendialogu8args_t args = {};
	args.filterList = filters;
	args.filterCount = 1;

	NFD_GetNativeWindowFromGLFWWindow(window, &args.parentWindow);

	nfdu8char_t* outPath;
	const nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

	if (result == NFD_ERROR)  {
		std::cout << "Error: " << NFD_GetError() << std::endl;
	}

	NFD_Quit();
	return result == NFD_CANCEL ? nullptr : outPath;
}

// A simple function to send the image data to the gpu and return the pointer.
unsigned int LoadTexture(unsigned char* image, int width, int height) {
	GLuint imageTexture;
	glGenTextures(1, &imageTexture);
	glBindTexture(GL_TEXTURE_2D, imageTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	return imageTexture;
}

// GLFW callbacks.

struct glfwUserData {
	Config* config;
	Render* render;

	glfwUserData(Config* config, Render* render) : config(config), render(render) {}
};

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));
	// Recalculate viewport size when the window is resized.
	data->render->CalcViewport(width, height);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

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

bool cursorWithinViewport = false;

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

	int width, height = 0;
	glfwGetWindowSize(window, &width, &height);

	cursorWithinViewport = width - x < data->render->GetViewportWidth() && height - y < data->render->GetViewportHeight();

	// Is left mouse down and is the cursor within the viewport.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GL_TRUE && cursorWithinViewport) {
		// Apply the difference between the previous and current mouse location to the entity's rotation.
		data->render->entity.Rotate(glm::vec3(-(y - previousY), x - previousX, 0));
	}

	previousX = x;
	previousY = y;
}

void ScrollCallback(GLFWwindow* window, double x, double y) {
	auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	if (data->render->entity.HasModel() && cursorWithinViewport) {
		data->render->camera.Move(glm::vec3(0, 0, y / 10));
	}
}

int main(int argc, char* argv[]) {
	// Initialize the GLFW3 library.
	if (!glfwInit())
		return 1;

	// The required OpenGL version (4.6).
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	// Avoids old and backwards compatible OpenGL features.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Initialize the single GLFW3 window used by this application, this also initializes an OpenGL context.
	GLFWwindow* mainWindow = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "LithoGen", nullptr, nullptr);

	if (!mainWindow)
		return 1;

#ifdef OS_WINDOWS
	// Give code its own scape so the variables get garbage collected in program lifetime.
	{
		// Set the windows title bar to dark as the application is always dark.
		// Below should use DWMWA_USE_IMMERSIVE_DARK_MODE but that is 20 on win11 and 19 on win10, 19 seems to also work on win11.
		constexpr BOOL darkMode = true;
		DwmSetWindowAttribute(glfwGetWin32Window(mainWindow), 19, &darkMode, sizeof(darkMode));
	}
#endif

	// Set the OpenGL context of this window as active for this thread.
	glfwMakeContextCurrent(mainWindow);

	// Load OpenGL for glad and avoid glad doing what GLFW3 has already done by attaching them.
	if (gladLoadGL(glfwGetProcAddress) == 0)
		return 1;

	// Don't draw the backside of a triangle.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Enable the depth buffer.
	glEnable(GL_DEPTH_TEST);

	// Set OpenGL clear render colour, the colour drawn if nothing else is.
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Dark Gray to be more distinct than black.

	// Must happen before ImGui initialisation.
	glfwSetFramebufferSizeCallback(mainWindow, FramebufferSizeCallback);
	glfwSetKeyCallback(mainWindow, KeyCallback);
	glfwSetCursorPosCallback(mainWindow, CursorPosCallback);
	glfwSetScrollCallback(mainWindow, ScrollCallback);

	// ImGui initialisation.
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(mainWindow, true); // Attach ImGui to GLFW3.
	ImGui_ImplOpenGL3_Init("#version 460"); // Configure GLSL version to match OpenGL 4.6.

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

	// The currently mounted image.
	Image image;

	while (!glfwWindowShouldClose(mainWindow)) {
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
					if (nfdu8char_t* result = FileSelectImage(mainWindow)) {
						// Load the image from storage, it will automatically process any of the supported formats.
						image.data = stbi_load(result, &image.width, &image.height, nullptr, 4);

						// Calculate the information required to gather aspect ratio based sizing.
						const int aspectGcd = std::gcd(image.width, image.height);
						image.aspectRatioW = image.width / aspectGcd;
						image.aspectRatioH = image.height / aspectGcd;

						// Ensure the default width is the correct aspect ratio and reset the size when a new image is loaded.
						config->sliderHeight = 100.0f;
						config->sliderWidth = 100.0f * image.aspectRatioW / image.aspectRatioH;

						if (image.data == nullptr) {
							std::cout << "Failed to load image!" << std::endl;
						} else {
							if (image.texture != 0) {
								glDeleteTextures(1, &image.texture); // Ensure the previous image has been cleaned up.
							}

							// Send the image to the GPU for the renderer to preview.
							image.texture = LoadTexture(image.data, image.width, image.height);
							// Clear the file path from memory as we are done with it.
							NFD_FreePathU8(result);
						}
					}
				}
				if (ImGui::MenuItem("Export")) {}
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
		int width, height; glfwGetWindowSize(mainWindow, &width, &height);

		// Push the sidepanel down to make room for the menu bar.
		ImGui::SetNextWindowPos(ImVec2(0, GUI_MENUBAR_HEIGHT));
		ImGui::SetNextWindowSize(ImVec2(GUI_SIDEPANEL_WIDTH, height - GUI_MENUBAR_HEIGHT));

		ImGui::Begin("SidePanel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		if (!image.data) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::SeparatorText("Image Processing");

		// TODO: Implement difference kinds of grayscale processing. Currently we are only doing luminance.
		ImGui::Text("Grayscale Preference");
		ImGui::SliderFloat("Red", &config->sliderGsPref[0], 0.0f, 1.0f, SLIDER_FLOAT_FORMAT, ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Green", &config->sliderGsPref[1], 0.0f, 1.0f, SLIDER_FLOAT_FORMAT, ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Blue", &config->sliderGsPref[2], 0.0f, 1.0f, SLIDER_FLOAT_FORMAT, ImGuiSliderFlags_AlwaysClamp);

		ImGui::Text("Alpha Thickness");
		ImGui::SliderFloat("Alpha", &config->sliderGsPref[3], 0.0f, 1.0f, SLIDER_FLOAT_FORMAT, ImGuiSliderFlags_AlwaysClamp);

		ImGui::SeparatorText("Mesh Configuration");

		ImGui::Text("Dimensions");

		// TODO: The forced ratio does not clamp.
		if (ImGui::SliderFloat("Width", &config->sliderWidth, SLIDER_WIDTH_MIN, SLIDER_WIDTH_MAX, SLIDER_FLOAT_FORMAT_MM)) {
			config->sliderHeight = config->sliderWidth * image.aspectRatioH / image.aspectRatioW;
		}

		if (ImGui::SliderFloat("Height", &config->sliderHeight, SLIDER_HEIGHT_MIN, SLIDER_HEIGHT_MAX, SLIDER_FLOAT_FORMAT_MM)) {
			config->sliderWidth = config->sliderHeight * image.aspectRatioW / image.aspectRatioH;
		}

		ImGui::Text("Thickness");

		if (ImGui::SliderFloat("Min", &config->sliderThickMin, SLIDER_THICK_MIN, SLIDER_THICK_MAX, SLIDER_FLOAT_FORMAT_MM)) {
			if (config->sliderThickMin > config->sliderThickMax) {
				config->sliderThickMax = config->sliderThickMin;
			}
		}

		if (ImGui::SliderFloat("Max", &config->sliderThickMax, SLIDER_THICK_MIN, SLIDER_THICK_MAX, SLIDER_FLOAT_FORMAT_MM)) {
			if (config->sliderThickMax < config->sliderThickMin) {
				config->sliderThickMin = config->sliderThickMax;
			}
		}

		ImGui::Spacing();
		if (ImGui::Button("Compile")) {
			Model model;
			CompileModel(model, config, image);
			render->entity.LoadModel(model);

			// TODO: Add visual error if compile fails.
		}

		if (!image.data) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::End();

		if (config->drawSource && image.texture != 0) {
			// Place the source preview in the bottom right corner.
			// Add 10 pixels of padding to the window edge.
			float previewWidth = 128.0f;
			float previewHeight = 128.0f;

			// Scale around the larger side.
			if (image.aspectRatioW < image.aspectRatioH) {
				previewHeight = 128.0f * image.aspectRatioH / image.aspectRatioW;
			} else {
				previewWidth = 128.0f * image.aspectRatioW / image.aspectRatioH;
			}

			ImGui::SetNextWindowPos(ImVec2(width - previewWidth - 10.0f, height - previewHeight - 10.0f));
			ImGui::SetNextWindowSize(ImVec2(previewWidth, previewHeight));

			// Remove padding as it is only a problem for this panel.
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::Begin("Source Preview", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
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

		glfwSwapBuffers(mainWindow); // Push the prepared frame buffer to the screen.
		glfwPollEvents(); // Process the OS's window events, in other words, gathering inputs and window state from the OS.
	}

	// Cleanup
	glfwTerminate();

	return 0;
}
