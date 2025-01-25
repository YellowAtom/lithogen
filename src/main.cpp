
#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <glfw/glfw3.h>

#ifdef OS_WINDOWS
#include <windows.h>
#include <dwmapi.h>
#include "glfw/glfw3native.h"
#endif

int main(int argc, char* argv[]) {
	// Initialize the GLFW3 library.
	if (!glfwInit())
		return 1;

	// Initialize the single GLFW3 window used by this application, this also initializes an OpenGL context.
	GLFWwindow* window = glfwCreateWindow(800, 600, "LithoGen", nullptr, nullptr);

	if (!window)
		return 1;

#ifdef OS_WINDOWS
	// Gather the windows native window handlers.
	HWND hWindow = glfwGetWin32Window(window);
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	// Force the window title bar to dark mode through the dwmapi.
	// TODO: Figure out how to implement responsive dark mode while using GLFW3.
	const BOOL value = true;
	DwmSetWindowAttribute(hWindow, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

	// Set the .ico from the resource file as the window icon.
	HICON hIcon = LoadIcon(hInstance, "LithoGenIcon");
	SendMessage(hWindow, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
	SendMessage(hWindow, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
#endif

	// Set the OpenGL context of this window as active for this thread.
	glfwMakeContextCurrent(window);

	// Load OpenGL for glad and avoid glad doing what GLFW3 has already done by attaching them.
	if (gladLoadGL(glfwGetProcAddress) == 0)
		return 1;

	// ImGui initialisation.
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true); // Attach ImGui to GLFW3.
	ImGui_ImplOpenGL3_Init("#version 460"); // Configure GLSL version to match OpenGL 4.6.

	// ImGui configuration.
	ImGui::StyleColorsDark(); // Set ImGui colours to dark mode.
	ImGuiIO& io = ImGui::GetIO(); (void)io; // Struct to access various ImGui features and configuration.
	io.IniFilename = nullptr; // Disable saving ImGui state, unneeded in this implementation.

	// ImGui button state.
	bool showHello = true;

	while (!glfwWindowShouldClose(window)) {
		// Set OpenGL clear render colour, the colour drawn if nothing else is.
		glClearColor(0.341f, 0.349f, 0.431f, 1.0f); // Normalised RGB values, #57596e.
		glClear(GL_COLOR_BUFFER_BIT);

		// Prepare ImGui at the start of a new frame.
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Gather the window height.
		int height; glfwGetWindowSize(window, nullptr, &height);

		// The following two functions both treat 0 and max as 1 pixel from the edge.
		// Set both the position and size of the side panel.
		ImGui::SetNextWindowPos(ImVec2(-1, 0));
		ImGui::SetNextWindowSize(ImVec2(230, height + 1));

		ImGui::Begin("SidePanel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
			// A test demonstration of ImGui features.
			ImGui::Checkbox("Show Hello", &showHello);

			if (showHello)
				ImGui::Text("Hello, world!");
		ImGui::End();

		// Trigger an ImGui render.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);

		// Trigger GLFW3 to ask the OS about input and window state.
		glfwPollEvents();
	}

	// ImGui cleanup.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// GLFW3 cleanup.
	glfwTerminate();

	return 0;
}
