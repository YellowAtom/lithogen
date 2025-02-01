
#include <fstream>
#include <iostream>
#include <string>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "constants.h"
#include "menu.h"

extern MenuConfig g_config;
extern GLFWwindow* g_mainWindow;
unsigned int g_shaderProgram;
unsigned int g_demoVAO;
int g_viewportWidth;
int g_viewportHeight;
float g_aspectRatio;

// A struct to contain data for each vertex.
struct Vertex {
	glm::vec3 position{};
	glm::vec3 color{};

	Vertex() = default;

	Vertex(float x, float y, float z) {
		position = glm::vec3(x, y, z);

		// Color is temporally randomised for testing.
		color = glm::vec3(
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX)
		);
	}
};

void CalcViewport(GLFWwindow* window, int width, int height) {
	// Place the renderer viewport to the right of the sidepanel and below the menu bar.
	// The render calculations need access to this data also.
	g_viewportWidth = width - GUI_SIDEPANEL_WIDTH;
	g_viewportHeight = height - GUI_MENUBAR_HEIGHT;
	g_aspectRatio = static_cast<float>(g_viewportWidth) / static_cast<float>(g_viewportHeight);

	glViewport(GUI_SIDEPANEL_WIDTH, 0, g_viewportWidth, g_viewportHeight);
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

void InitShaders() {
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

void CreateObject() {
	// Template data for a cube.
	const Vertex vertices[] = {
		Vertex(0.5f, 0.5f, 0.5f),
		Vertex(-0.5f, 0.5f, -0.5f),
		Vertex(-0.5f, 0.5f, 0.5f),
		Vertex(0.5f, -0.5f, -0.5f),
		Vertex(-0.5f, -0.5f, -0.5f),
		Vertex(0.5f, 0.5f, -0.5f),
		Vertex(0.5f, -0.5f, 0.5f),
		Vertex(-0.5f, -0.5f, 0.5f)
	};

	const unsigned int indices[] = {
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
	};

	glGenVertexArrays(1, &g_demoVAO);
	glBindVertexArray(g_demoVAO);

	// Allocate and configure the buffers.
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int IBO;
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Bind the buffers to the VAO.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// Tell the driver how to read position from the buffer.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

	// Tell the driver how to read color from the buffer.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));

	// Unbind.
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CreateProjection(glm::mat4& mvp, const float fov, const float nearZ, const float farZ) {
	const float tanHalfFOV = tanf(glm::radians(fov / 2));
	const float d = 1.0f / tanHalfFOV;

	float zRange = nearZ - farZ;
	float a = (-farZ - nearZ) / zRange;
	float b = 2.0f * farZ * nearZ / zRange;

	// The projection matrix. calculated form aspect ratio, fov and clip planes.
	mvp *= glm::mat4(
		d / g_aspectRatio, 0.0f, 0.0f, 0.0f,
		0.0f, d, 0.0f, 0.0f,
		0.0f, 0.0f, a, 1.0f,
		0.0f, 0.0f, b, 0.0f
	);
}

void CreateView(glm::mat4& mvp, const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
	glm::vec3 n = normalize(target);
	glm::vec3 u = normalize(cross(normalize(up), n));
	glm::vec3 v = cross(n, u);

	mvp *= glm::mat4(
		u.x, v.x, n.x, 0.0f,
		u.y, v.y, n.y, 0.0f,
		u.z, v.z, n.z, 0.0f,
		-position.x, -position.y, -position.z, 1.0f
	);
}

void CreateModel(glm::mat4& mvp, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
	// Apply scale to the matrix.
	glm::mat4 s(
		scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Apply translation / position to the matrix.
	glm::mat4 t(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		position.x, position.y, position.z, 1.0f
	);

	// Apply rotation to the matrix.
	float rotateX = glm::radians(rotation.x);
	float rotateY = glm::radians(rotation.y);
	float rotateZ = glm::radians(rotation.z);

	glm::mat4 rx(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosf(rotateX), sinf(rotateX), 0.0f,
		0.0f, -sinf(rotateX), cosf(rotateX), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::mat4 ry(
		cosf(rotateY), 0.0f, sinf(rotateY), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sinf(rotateY), 0.0f, cosf(rotateY), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::mat4 rz(
		cosf(rotateZ), 0.0f, sinf(rotateZ), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sinf(rotateZ), 0.0f, cosf(rotateZ), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Combine the matrices into the world matrix in the correct order.
	mvp *= t * (rz * ry * rx) * s;
}

void RenderObject() {
	static float rotation = 0.0f;
	rotation += 0.5f;

	// The model view projection matrix.
	glm::mat4 mvp(1.0f);

	CreateProjection(mvp, 90.0f, 1.0f, 10.0f);

	CreateView(mvp,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	CreateModel(mvp,
		glm::vec3(0.0f, 0.0f, 2.0f),
		glm::vec3(rotation, rotation, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
	);

	// Pass the completed matrix to the GPU to be applied in the shader to the vector position.
	glUniformMatrix4fv(glGetUniformLocation(g_shaderProgram, "mvp"), 1, GL_FALSE, value_ptr(mvp));

	// Bind the VAO referencing the vertex and indices buffers.
	glBindVertexArray(g_demoVAO);

	// Draw the given data to the screen.
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

	// Unbind the VAO to ensure a clear OpenGL state.
	glBindVertexArray(0);
}

void DisplayInit() {
	// Vertex optimisation, don't draw the back side of a triangle.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Set OpenGL clear render colour, the colour drawn if nothing else is.
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Dark Gray to be more distinct than black.

	// Configure the OpenGL viewport size when the window is resized, and run that calculation once to start.
	glfwSetFramebufferSizeCallback(g_mainWindow, CalcViewport);
	CalcViewport(g_mainWindow, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

	InitShaders();
	CreateObject();
}

void DisplayDraw() {
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer / frame to avoid any junk.

	if (g_config.drawPreview) {
		RenderObject();
	}
}
