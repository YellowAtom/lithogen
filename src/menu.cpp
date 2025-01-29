
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glfw/glfw3.h>
#include "constants.h"
#include "menu.h"

extern GLFWwindow* g_mainWindow;
MenuConfig g_config = {};

void MenuInit() {
	// ImGui initialisation.
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(g_mainWindow, true); // Attach ImGui to GLFW3.
	ImGui_ImplOpenGL3_Init("#version 460"); // Configure GLSL version to match OpenGL 4.6.

	// ImGui configuration.
	ImGui::StyleColorsDark(); // Set ImGui colours to dark mode.
	ImGuiIO& io = ImGui::GetIO(); (void)io; // Struct to access various ImGui features and configuration.
	io.IniFilename = nullptr; // Disable saving ImGui state, unneeded in this implementation.
}

bool openMenu = false;

void MenuDraw() {
	// Prepare ImGui at the start of a new frame.
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Menu bar
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Import")) {}
			if (ImGui::MenuItem("Export")) {}
			if (ImGui::MenuItem("Quit", "Alt+F4")) {
				glfwSetWindowShouldClose(g_mainWindow, GL_TRUE);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Display Picture", nullptr, &g_config.drawPicture);
			ImGui::MenuItem("Render Mesh", nullptr, &g_config.renderMesh);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("About")) {
				openMenu = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Gather the window height.
	int height; glfwGetWindowSize(g_mainWindow, nullptr, &height);

	// Push the sidepanel down to make room for the menu bar.
	ImGui::SetNextWindowPos(ImVec2(0, GUI_MENUBAR_HEIGHT));
	ImGui::SetNextWindowSize(ImVec2(GUI_SIDEPANEL_WIDTH, static_cast<float>(height) - GUI_MENUBAR_HEIGHT));

	ImGui::Begin("SidePanel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	// A test demonstration of ImGui features.
	ImGui::Checkbox("Example Checkbox", &g_config.exampleCheckbox);
	ImGui::End();

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center);

	if (openMenu) {
		ImGui::Begin("About", &openMenu, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Version: 1.0");
		ImGui::End();
	}

	// Trigger an ImGui render.
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MenuShutdown() {
	// ImGui cleanup.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
