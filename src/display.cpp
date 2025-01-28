
#include <fstream>
#include <iostream>
#include <string>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include "glm/glm.hpp"
#include "constants.h"
#include "menu.h"

extern MenuConfig g_config;
extern GLFWwindow* g_mainWindow;
unsigned int g_shaderProgram;
unsigned int g_demoVAO;

void CalcViewport(GLFWwindow* window, int width, int height) {
	// Place the renderer viewport to the right of the sidepanel and below the menu bar.
	glViewport(GUI_SIDEPANEL_WIDTH, 0, width - GUI_SIDEPANEL_WIDTH, height - GUI_MENUBAR_HEIGHT);
}

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

void CompileShaders() {
	g_shaderProgram = glCreateProgram();

	if (g_shaderProgram == 0) {
		std::cout << "Error creating shader program" << std::endl;
		exit(1);
	}

	unsigned int vertShader = LoadShader("./shaders/vertex.glsl", GL_VERTEX_SHADER);
	unsigned int fragShader = LoadShader("./shaders/fragment.glsl", GL_FRAGMENT_SHADER);

	// Attach the finalised shaders to a shader program to be used by the rest of the program.
	glAttachShader(g_shaderProgram, vertShader);
	glAttachShader(g_shaderProgram, fragShader);

	int success;

	glLinkProgram(g_shaderProgram);
	glGetProgramiv(g_shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(g_shaderProgram, 512, nullptr, infoLog);
		std::cout << "Failed to link shader program: \n" << infoLog << std::endl;
		exit(1);
	}

	glValidateProgram(g_shaderProgram);
	glGetProgramiv(g_shaderProgram, GL_VALIDATE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(g_shaderProgram, 512, nullptr, infoLog);
		std::cout << "Failed to validate shader program: \n" << infoLog << std::endl;
		exit(1);
	}

	glUseProgram(g_shaderProgram);

	// Clean up shaders now they are inside the shader program.
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

void VAOInit() {
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
	glGenVertexArrays(1, &g_demoVAO);
	glBindVertexArray(g_demoVAO);

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
}

void DisplayInit() {
	// Vertex optimisation, don't draw the back side of a triangle.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Configure the OpenGL viewport size when the window is resized, and run that calculation once to start.
	glfwSetFramebufferSizeCallback(g_mainWindow, CalcViewport);
	CalcViewport(g_mainWindow, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

	VAOInit();
	CompileShaders();
}

void DisplayDraw() {
	// Set OpenGL clear render colour, the colour drawn if nothing else is.
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Dark Gray to be more distinct than black.
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer / frame to avoid any junk.

	// Draw Demo Triangle.
	glBindVertexArray(g_demoVAO);

	if (g_config.renderMesh)
		glDrawArrays(GL_TRIANGLES, 0, 3);
}
