
#include <fstream>
#include <iostream>
#include <string>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "constants.h"
#include "matrix_math.h"
#include "classes/display_data.h"
#include "classes/menu_config.h"
#include "classes/entity.h"
#include "classes/model.h"
#include "classes/vertex.h"

// GLFW callback functions.

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	auto* display = static_cast<DisplayData*>(glfwGetWindowUserPointer(window));
	// Recalculate viewport size when the window is resized.
	display->CalcViewport(window, width, height);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto* display = static_cast<DisplayData*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		display->entity.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f)); // Reset rotation.
	}
}

void CursorPosCallback(GLFWwindow* window, double x, double y) {
	auto* display = static_cast<DisplayData*>(glfwGetWindowUserPointer(window));

	// TODO: Rotating the cube too much will make this inaccurate, find a way.

	// Store previous mouse location to calculate difference.
	// Static allows a variable to persist through multiple function calls.
	static double previousX = 0;
	static double previousY = 0;

	int width, height = 0;
	glfwGetWindowSize(window, &width, &height);

	// Is left mouse down and is the cursor within the viewport.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == true && width - x < display->GetViewportWidth() && height - y < display->GetViewportHeight()) {
		// Apply the difference between the previous and current mouse location to the entity's rotation.
		display->entity.AddRotation(glm::vec3(-(y - previousY), x - previousX, 0));
	}

	previousX = x;
	previousY = y;
}

void ScrollCallback(GLFWwindow* window, double x, double y) {
	auto* display = static_cast<DisplayData*>(glfwGetWindowUserPointer(window));

	display->camera.Move(glm::vec3(0, 0, y / 10));
}

// Shader initialization functions.

unsigned int LoadShader(const char* pFilePath, const int shaderType) {
	std::ifstream shaderFile(pFilePath);

	if (!shaderFile.is_open()) {
		std::cout << "Error opening shader file \"" << pFilePath << "\"" << std::endl;
		exit(1);
	}

	std::string shaderStr, line;

	while (getline(shaderFile, line)) {
		shaderStr.append(line);
		shaderStr.append("\n");
	}

	const char* shaderSrc = shaderStr.data();
	unsigned int shader = glCreateShader(shaderType);

	if (shader == 0) {
		std::cout << "Error creating shader type" << std::endl;
		exit(1);
	}

	const char* shaders[1];
	shaders[0] = shaderSrc;

	int lengths[1];
	lengths[0] = strlen(shaderSrc);

	int success;

	glShaderSource(shader, 1, shaders, lengths);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cout << "Failed to compile shader \"" << pFilePath << "\": \n" << infoLog << std::endl;
		exit(1);
	}

	return shader;
}

unsigned int InitShaders() {
	const unsigned int shaderProgram = glCreateProgram();

	if (shaderProgram == 0) {
		std::cout << "Error creating shader program" << std::endl;
		exit(1);
	}

	unsigned int vertShader = LoadShader("./shaders/vertex.glsl", GL_VERTEX_SHADER);
	unsigned int fragShader = LoadShader("./shaders/fragment.glsl", GL_FRAGMENT_SHADER);

	// Attach the finalised shaders to a shader program to be used by the rest of the program.
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);

	int success;

	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "Failed to link shader program: \n" << infoLog << std::endl;
		exit(1);
	}

	glValidateProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "Failed to validate shader program: \n" << infoLog << std::endl;
		exit(1);
	}

	glUseProgram(shaderProgram);

	// Clean up shaders now they are inside the shader program.
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return shaderProgram;
}

// Temporary random colour function.
glm::vec3 RandomColor() {
	return {
		static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
		static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
		static_cast<float>(rand()) / static_cast<float>(RAND_MAX)
	};
}

DisplayData* DisplayInit(GLFWwindow* window) {
	// Vertex optimisation, don't draw the back side of a triangle.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Set OpenGL clear render colour, the colour drawn if nothing else is.
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Dark Gray to be more distinct than black.

	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	// ImGui initialisation.
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true); // Attach ImGui to GLFW3.
	ImGui_ImplOpenGL3_Init("#version 460"); // Configure GLSL version to match OpenGL 4.6.

	// ImGui configuration.
	ImGui::StyleColorsDark(); // Set ImGui colours to dark mode.
	ImGuiIO& io = ImGui::GetIO(); (void)io; // Struct to access various ImGui features and configuration.
	io.IniFilename = nullptr; // Disable saving ImGui state, unneeded in this implementation.

	// Initialize OpenGL shader program and pass it into the struct.
	unsigned int shaderProgram = InitShaders();

	auto entity = Entity(
		Model(
			{
				Vertex(glm::vec3(0.5f, 0.5f, 0.5f), RandomColor()),
				Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), RandomColor()),
				Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), RandomColor()),
				Vertex(glm::vec3(0.5f, -0.5f, -0.5f), RandomColor()),
				Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), RandomColor()),
				Vertex(glm::vec3(0.5f, 0.5f, -0.5f), RandomColor()),
				Vertex(glm::vec3(0.5f, -0.5f, 0.5f), RandomColor()),
				Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), RandomColor())
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
		),
		shaderProgram
	);

	auto camera = Camera();

	glm::vec3 entityPos(0.0f, 0.0f, 2.0f);

	camera.SetTargetPos(entityPos);
	entity.SetPosition(entityPos);

	// This is never cleared up because the kernel will do that when the program closes.
	// It can be optionally cleaned up in the function init is called.
	auto* display = new DisplayData(
		camera,
		entity,
		MenuConfig(),
		shaderProgram
	);

	// Call this during init as the resize callback is not called on startup.
	display->CalcViewport(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

	// Make this object accessible from within any GLFW callback.
	glfwSetWindowUserPointer(window, display);

	return display;
}

void DrawImGui(DisplayData* display, GLFWwindow* window) {
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
				glfwSetWindowShouldClose(window, GL_TRUE);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Show Source", nullptr, &display->config.drawSource);
			ImGui::MenuItem("Show Preview", nullptr, &display->config.drawPreview);
			ImGui::Separator();
			if (ImGui::MenuItem("Reset Preview", "R")) {
				display->entity.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f)); // Reset object rotation.
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("About")) {
				display->config.aboutOpened = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Gather the window height.
	int height; glfwGetWindowSize(window, nullptr, &height);

	// Push the sidepanel down to make room for the menu bar.
	ImGui::SetNextWindowPos(ImVec2(0, GUI_MENUBAR_HEIGHT));
	ImGui::SetNextWindowSize(ImVec2(GUI_SIDEPANEL_WIDTH, static_cast<float>(height) - GUI_MENUBAR_HEIGHT));

	ImGui::Begin("SidePanel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	// A test demonstration of ImGui features.
	ImGui::Checkbox("Example Checkbox", &display->config.exampleCheckbox);
	ImGui::End();

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center);

	if (display->config.aboutOpened) {
		ImGui::Begin("About", &display->config.aboutOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Version: 1.0");
		ImGui::End();
	}

	// Trigger an ImGui render.
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DisplayDraw(DisplayData* display, GLFWwindow* window) {
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer / frame to avoid any junk.

	if (display->config.drawPreview) {
		// This section of the MVP will be used by all objects, if there are multiple.
		glm::mat4 mvp(1.0f);

		display->camera.ApplyMatrix(mvp, display->GetAspectRatio());

		display->entity.Draw(mvp);
	}

	DrawImGui(display ,window);
}
