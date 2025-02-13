
#include <iostream>

#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <battery/embed.hpp>

#include "render.h"
#include "constants.h"
#include "model.h"

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

	// Move the starting position of the entity.
	entity.SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));

	// Call this during init as the resize callback is not called on startup.
	CalcViewport(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

void Render::Draw() const {
	if (m_config->drawPreview) {
		// This section of the MVP will be used by all objects, if there are multiple.
		glm::mat4 mvp(1.0f);
		camera.ApplyMatrix(mvp, GetAspectRatio());

		entity.Draw(mvp);
	}
}

void Render::UpdateWireframe() const {
	if (m_config->drawWireframe) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE); // Render back side of wireframe faces.
	glPolygonMode(GL_FRONT_AND_BACK, m_config->drawWireframe ? GL_LINE : GL_FILL); // Toggle line rendering.
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
