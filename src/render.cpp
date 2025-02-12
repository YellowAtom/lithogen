
#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <battery/embed.hpp>

#include "render.h"
#include "constants.h"
#include "model.h"

// GLFW callback functions.

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	auto* render = static_cast<Render*>(glfwGetWindowUserPointer(window));
	// Recalculate viewport size when the window is resized.
	render->CalcViewport(width, height);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto* render = static_cast<Render*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		render->entity.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f)); // Reset rotation.
	}
}

void CursorPosCallback(GLFWwindow* window, const double x, const double y) {
	auto* render = static_cast<Render*>(glfwGetWindowUserPointer(window));

	// TODO: Rotating the cube too much will make this inaccurate, find a way.

	// Store previous mouse location to calculate difference.
	// Static allows a variable to persist through multiple function calls.
	static double previousX = 0;
	static double previousY = 0;

	int width, height = 0;
	glfwGetWindowSize(window, &width, &height);

	// Is left mouse down and is the cursor within the viewport.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GL_TRUE && width - x < render->GetViewportWidth() && height - y < render->GetViewportHeight()) {
		// Apply the difference between the previous and current mouse location to the entity's rotation.
		render->entity.Rotate(glm::vec3(-(y - previousY), x - previousX, 0));
	}

	previousX = x;
	previousY = y;
}

void ScrollCallback(GLFWwindow* window, double x, double y) {
	auto* render = static_cast<Render*>(glfwGetWindowUserPointer(window));

	render->camera.Move(glm::vec3(0, 0, y / 10));
}

// Shader initialization functions.

unsigned int LoadShader(const char* pFileContent, const int shaderType) {
	unsigned int shader = glCreateShader(shaderType);

	if (shader == 0) {
		std::cout << "Error creating shader type" << std::endl;
		exit(1);
	}

	const char* shaders[1];
	shaders[0] = pFileContent;

	int lengths[1];
	lengths[0] = strlen(pFileContent);

	int success;

	glShaderSource(shader, 1, shaders, lengths);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cout << "Failed to compile shader type \"" << shaderType << "\": \n" << infoLog << std::endl;
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

	const unsigned int vertShader = LoadShader(b::embed<"res/shaders/vertex.glsl">().data(), GL_VERTEX_SHADER);
	const unsigned int fragShader = LoadShader(b::embed<"res/shaders/fragment.glsl">().data(), GL_FRAGMENT_SHADER);

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

Render::Render(GLFWwindow* window, Config* config, const Model& model)
	: entity(model, InitShaders()), m_window(window), m_config(config)
{
	// Ensure our OpenGL configurations will affect the correct context.
	glfwMakeContextCurrent(window);

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
	ImGuiIO& io = ImGui::GetIO();
	(void)io; // Struct to access various ImGui features and configuration.
	io.IniFilename = nullptr; // Disable saving ImGui state, unneeded in this implementation.

	// Move the starting position of the entity.
	entity.SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));

	// Call this during init as the resize callback is not called on startup.
	CalcViewport(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

	// Make this object accessible from within any GLFW callback.
	glfwSetWindowUserPointer(window, this);
}

void Render::Draw() {
	if (m_config->drawPreview) {
		// This section of the MVP will be used by all objects, if there are multiple.
		glm::mat4 mvp(1.0f);

		camera.ApplyMatrix(mvp, GetAspectRatio());
		entity.Draw(mvp);
	}

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
				glfwSetWindowShouldClose(m_window, GL_TRUE);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Show Source", nullptr, &m_config->drawSource);
			ImGui::Separator();
			ImGui::MenuItem("Show Preview", nullptr, &m_config->drawPreview);
			if (ImGui::MenuItem("Wireframe Preview", nullptr, &m_config->drawWireframe)) {
				if (m_config->drawWireframe) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE); // Render back side of wireframe faces.
				glPolygonMode(GL_FRONT_AND_BACK, m_config->drawWireframe ? GL_LINE : GL_FILL); // Toggle line rendering.
			}
			if (ImGui::MenuItem("Reset Preview", "R")) {
				entity.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f)); // Reset object rotation.
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("About")) {
				m_config->aboutOpened = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Gather the window height.
	int height; glfwGetWindowSize(m_window, nullptr, &height);

	// Push the sidepanel down to make room for the menu bar.
	ImGui::SetNextWindowPos(ImVec2(0, GUI_MENUBAR_HEIGHT));
	ImGui::SetNextWindowSize(ImVec2(GUI_SIDEPANEL_WIDTH, static_cast<float>(height) - GUI_MENUBAR_HEIGHT));

	ImGui::Begin("SidePanel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	// A test demonstration of ImGui features.
	ImGui::Checkbox("Example Checkbox", &m_config->exampleCheckbox);
	ImGui::End();

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center);

	if (m_config->aboutOpened) {
		ImGui::Begin("About", &m_config->aboutOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Version: 1.0");
		ImGui::End();
	}

	// Trigger an ImGui render.
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Render::CalcViewport(int width, int height) {
	// Place the renderer viewport to the right of the sidepanel and below the menu bar.
	// The render calculations need access to this data also.
	m_viewportWidth = width - GUI_SIDEPANEL_WIDTH;
	m_viewportHeight = height - GUI_MENUBAR_HEIGHT;
	m_aspectRatio = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);

	glViewport(GUI_SIDEPANEL_WIDTH, 0, m_viewportWidth, m_viewportHeight);
}

int Render::GetViewportWidth() const {
	return m_viewportWidth;
}

int Render::GetViewportHeight() const {
	return m_viewportHeight;
}

float Render::GetAspectRatio() const {
	return m_aspectRatio;
}
