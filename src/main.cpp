
#include <random>
#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <nfd_glfw3.h>

#include "constants.h"
#include "render.h"
#include "config.h"

#ifdef OS_WINDOWS
#include <windows.h>
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

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		data->render->entity.ResetRotation();
	} else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		data->config->drawWireframe = !data->config->drawWireframe;
		data->render->UpdateWireframe();
	}
}

void CursorPosCallback(GLFWwindow* window, const double x, const double y) {
	auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	// TODO: Rotating the cube too much will make this inaccurate, find a way.

	// Store previous mouse location to calculate difference.
	// Static allows a variable to persist through multiple function calls.
	static double previousX = 0;
	static double previousY = 0;

	int width, height = 0;
	glfwGetWindowSize(window, &width, &height);

	// Is left mouse down and is the cursor within the viewport.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GL_TRUE && width - x < data->render->GetViewportWidth() && height - y < data->render->GetViewportHeight()) {
		// Apply the difference between the previous and current mouse location to the entity's rotation.
		data->render->entity.Rotate(glm::vec3(-(y - previousY), x - previousX, 0));
	}

	previousX = x;
	previousY = y;
}

void ScrollCallback(GLFWwindow* window, double x, double y) {
	auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	data->render->camera.Move(glm::vec3(0, 0, y / 10));
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
		// Gather the windows native window handlers.
		HWND hWindow = glfwGetWin32Window(mainWindow);
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		// Set the .ico from the resource file as the window icon.
		HICON hIcon = LoadIcon(hInstance, "LithoGenIcon");
		SendMessage(hWindow, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
		SendMessage(hWindow, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
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

	// Configure a randomness generator for the demo colours.
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	// TODO: Make sure this gets cleaned up after its used.
	// Temporary demo mesh, will be replaced with the lithophane generation pipeline.
	Model cubeModel(
		{
			Vertex(
				glm::vec3(0.5f, 0.5f, 0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(-0.5f, 0.5f, -0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(-0.5f, 0.5f, 0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(0.5f, -0.5f, -0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(-0.5f, -0.5f, -0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(0.5f, 0.5f, -0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(0.5f, -0.5f, 0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(-0.5f, -0.5f, 0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			)
		},
		{
			0, 1, 2,
			1, 3, 4,
			5, 6, 3,
			7, 3, 6,
			2, 4, 7,
			0, 7, 6,
			0, 5, 1,
			1, 5, 3,
			5, 0, 6,
			7, 4, 3,
			2, 1, 4,
			0, 2, 7
		}
	);

	// It is better to let the kernel clean these up as the program will close faster.
	auto* config = new Config();
	auto* render = new Render(mainWindow, config, cubeModel);
	auto* glfwUser = new glfwUserData(config, render);

	// Make this object accessible from within any GLFW callback.
	glfwSetWindowUserPointer(mainWindow, glfwUser);

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
						std::cout << result << std::endl;
						NFD_FreePathU8(result);
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
		int height; glfwGetWindowSize(mainWindow, nullptr, &height);

		// Push the sidepanel down to make room for the menu bar.
		ImGui::SetNextWindowPos(ImVec2(0, GUI_MENUBAR_HEIGHT));
		ImGui::SetNextWindowSize(ImVec2(GUI_SIDEPANEL_WIDTH, static_cast<float>(height) - GUI_MENUBAR_HEIGHT));

		ImGui::Begin("SidePanel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		// A test demonstration of ImGui features.
		ImGui::Checkbox("Example Checkbox", &config->exampleCheckbox);
		ImGui::End();

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center);

		if (config->aboutOpened) {
			ImGui::Begin("About", &config->aboutOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
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
