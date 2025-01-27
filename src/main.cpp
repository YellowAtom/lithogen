
#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include "glm/vec3.hpp"

#ifdef OS_WINDOWS
#include <windows.h>
#include <dwmapi.h>
#include "glfw/glfw3native.h"
#endif

// Constants
#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600
#define GUI_SIDEPANEL_WIDTH 230
#define GUI_MENUBAR_HEIGHT 18

bool InitializeShaders(const unsigned int& shaderProgram) {
	// The GLSL source code, is compiled to SPIR-V and used at runtime. TODO: Make this compile at compile time not runtime.
	const char* vertShaderSrc =
		"#version 460 core\n"
		"layout (location = 0) in vec3 in_position;\n"
		"layout (location = 1) in vec3 in_color;\n"
		"layout (location = 0) out vec3 frag_color;\n"
		"void main() {\n"
		"	frag_color = in_color;\n"
		"	gl_Position = vec4(in_position, 1.0f);\n"
		"}\0";

	const char* fragShaderSrc =
		"#version 460 core\n"
		"layout (location = 0) in vec3 flag_color;\n"
		"layout (location = 0) out vec4 out_color;\n"
		"void main() {\n"
		"	out_color = vec4(flag_color, 1.0f);\n"
		"}\0";

	int success;

	// Compile shader.
	unsigned int vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertShaderSrc, nullptr);
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
		std::cout << "Failed to compile vertex shader: \n" << infoLog << std::endl;
		return false;
	}

	// Compile shader.
	unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderSrc, nullptr);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
		std::cout << "Failed to compile fragment shader: \n" << infoLog << std::endl;
		return false;
	}

	// Attach the finalised shaders to a shader program to be used by the rest of the program.
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "Failed to link shader program: \n" << infoLog << std::endl;
		return false;
	}

	// Clean up shaders now they are inside the shader program.
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;
}

void CalculateViewport(GLFWwindow* window, int width, int height) {
	// Place the renderer viewport to the right of the sidepanel and below the menu bar.
	glViewport(GUI_SIDEPANEL_WIDTH, 0, width - GUI_SIDEPANEL_WIDTH, height - GUI_MENUBAR_HEIGHT);
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
	GLFWwindow* window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "LithoGen", nullptr, nullptr);

	if (!window)
		return 1;

#ifdef OS_WINDOWS
	// Gather the windows native window handlers.
	HWND hWindow = glfwGetWin32Window(window);
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	// Force the window title bar to dark mode through the dwmapi.
	// TODO: Figure out how to implement responsive dark mode while using GLFW3.
	constexpr BOOL value = true;
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

	// Vertex optimisation, don't draw the back side of a triangle.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Configure the OpenGL viewport size when the window is resized, and run that calculation once to start.
	glfwSetFramebufferSizeCallback(window, CalculateViewport);
	CalculateViewport(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

	// Shader initialisation.
	unsigned int shaderProgram = glCreateProgram();

	if (!InitializeShaders(shaderProgram))
		return 1;

	// ImGui initialisation.
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true); // Attach ImGui to GLFW3.
	ImGui_ImplOpenGL3_Init("#version 460"); // Configure GLSL version to match OpenGL 4.6.

	// ImGui configuration.
	ImGui::StyleColorsDark(); // Set ImGui colours to dark mode.
	ImGuiIO& io = ImGui::GetIO(); (void)io; // Struct to access various ImGui features and configuration.
	io.IniFilename = nullptr; // Disable saving ImGui state, unneeded in this implementation.

	// Demo triangle vertices.
	constexpr glm::vec3 triangleVertices[] = {
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, -1.0f, 0.0f),
	};

	// Demo triangle normalised rgb values for each vertex.
	constexpr glm::vec3 triangleColors[] = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	};

	// Create a Vertex Array Object and make it active / bound.
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create a Vertex Buffer Object and make it active / bound.
	unsigned int vboVertices;
	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);

	// Push the vertices into the buffer and therefor into the GPU for later.
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	unsigned int vboColors;
	glGenBuffers(1, &vboColors);
	glBindBuffer(GL_ARRAY_BUFFER, vboColors);

	// Do the same for the color data.
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColors), triangleColors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	// Unbind both objects.
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// ImGui button state.
	bool drawPicture = true;
	bool renderMesh = true;
	bool exampleCheckbox = true;

	while (!glfwWindowShouldClose(window)) {
		// Set OpenGL clear render colour, the colour drawn if nothing else is.
		glClearColor(0.341f, 0.349f, 0.431f, 1.0f); // Normalised RGB values, #57596e.
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw Demo Triangle.
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);

		if (renderMesh)
			glDrawArrays(GL_TRIANGLES, 0, 3);

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
				ImGui::MenuItem("Display Picture", nullptr, &drawPicture);
				ImGui::MenuItem("Render Mesh", nullptr, &renderMesh);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Gather the window height.
		int height; glfwGetWindowSize(window, nullptr, &height);

		// Push the sidepanel down to make room for the menu bar.
		ImGui::SetNextWindowPos(ImVec2(0, GUI_MENUBAR_HEIGHT));
		ImGui::SetNextWindowSize(ImVec2(GUI_SIDEPANEL_WIDTH, height - GUI_MENUBAR_HEIGHT));

		ImGui::Begin("SidePanel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
			// A test demonstration of ImGui features.
			ImGui::Checkbox("Example Checkbox", &exampleCheckbox);
		ImGui::End();

		// Trigger an ImGui render.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Push the prepared frame buffer to the screen.
		glfwSwapBuffers(window);

		// Process the OS's window events, in other words, gathering inputs and window state from the OS.
		glfwPollEvents();
	}

	// OpenGL cleanup.
	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &vboVertices);
	glDeleteBuffers(1, &vboColors);
	glDeleteVertexArrays(1, &VAO);

	// ImGui cleanup.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// GLFW3 cleanup.
	glfwTerminate();

	return 0;
}
