#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <iostream>
#include <nfd_glfw3.h>
#include <numeric>
#include "config.h"
#include "constants.h"
#include "image.h"
#include "interface.h"
#include "render.h"

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
		data->render->camera.ResetRotation();
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
		// Apply the difference between the previous and current mouse location to the camera's rotation.
		data->render->camera.RotateHorizontal((x - previousX) / 20);
		data->render->camera.RotateVertical((y - previousY) / 20);
	}

	previousX = x;
	previousY = y;
}

void ScrollCallback(GLFWwindow* window, const double x, const double y) {
	const auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	// Move 4mm per scroll.
	if (data->render->entity.HasModel() && data->cursorWithinViewport) {
		data->render->camera.Zoom(y * 4);
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
	// As we use grayscale to indicate topology, the background colour should be distinct from that.
	glClearColor(0, 0.102F, 0.188F, 1.0F);

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

		RenderInterface(mainWindow, image, config, model, render);

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