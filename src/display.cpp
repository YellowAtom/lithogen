
#include <iostream>
#include <imgui.h>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include "glm/vec3.hpp"
#include "constants.h"
#include "menu.h"

extern MenuConfig g_config;
extern GLFWwindow* g_mainWindow;
unsigned int g_shaderProgram;
unsigned int g_demoVAO;

void DisplayCalcViewport(GLFWwindow* window, int width, int height) {
	// Place the renderer viewport to the right of the sidepanel and below the menu bar.
	glViewport(GUI_SIDEPANEL_WIDTH, 0, width - GUI_SIDEPANEL_WIDTH, height - GUI_MENUBAR_HEIGHT);
}

bool DisplayShaderInit() {
	g_shaderProgram = glCreateProgram();

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
	glAttachShader(g_shaderProgram, vertShader);
	glAttachShader(g_shaderProgram, fragShader);
	glLinkProgram(g_shaderProgram);
	glGetProgramiv(g_shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(g_shaderProgram, 512, nullptr, infoLog);
		std::cout << "Failed to link shader program: \n" << infoLog << std::endl;
		return false;
	}

	// Clean up shaders now they are inside the shader program.
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;
}

void DisplayVAOInit() {
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

bool DisplayInit() {
	// Vertex optimisation, don't draw the back side of a triangle.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Configure the OpenGL viewport size when the window is resized, and run that calculation once to start.
	glfwSetFramebufferSizeCallback(g_mainWindow, DisplayCalcViewport);
	DisplayCalcViewport(g_mainWindow, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

	DisplayVAOInit();

	if (!DisplayShaderInit())
		return false;

	return true;
}

void DisplayDraw() {
	// Set OpenGL clear render colour, the colour drawn if nothing else is.
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Dark Gray to be more distinct than black.
	glClear(GL_COLOR_BUFFER_BIT); // Clear the frame to avoid any junk.

	// Draw Demo Triangle.
	glUseProgram(g_shaderProgram);
	glBindVertexArray(g_demoVAO);

	if (g_config.renderMesh)
		glDrawArrays(GL_TRIANGLES, 0, 3);
}
