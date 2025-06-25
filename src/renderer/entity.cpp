// SPDX-License-Identifier: GPL-3.0
#include "entity.h"
#include <algorithm>
#include <battery/embed.hpp>
#include <climits>
#include <cstring>
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include "../declarations/structures.h"

// Shader initialization functions.

GLuint LoadShader(const char* pFileContent, const GLenum shaderType)
{
	const GLuint shader = glCreateShader(shaderType);

	if (shader == 0) {
		std::cout << "Error creating shader type!\n";
		exit(1);
	}

	const char* shaders[1] = {pFileContent};
	const size_t length = strlen(pFileContent);

	if (length > INT_MAX) {
		std::cout << "Shader file \"" << shaderType << "\" too large!\n";
		exit(1);
	}

	const GLint lengths[1] = {static_cast<GLint>(length)};

	GLint success = 0;

	glShaderSource(shader, 1, shaders, lengths);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == 0) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cout << "Failed to compile shader type \"" << shaderType << "\":\n" << infoLog << '\n';
		exit(1);
	}

	return shader;
}

GLuint InitShaders()
{
	const GLuint shaderProgram = glCreateProgram();

	if (shaderProgram == 0) {
		std::cout << "Error creating shader program!\n";
		exit(1);
	}

	const GLuint vertShader = LoadShader(b::embed<"res/shaders/vertex.glsl">().data(), GL_VERTEX_SHADER);
	const GLuint fragShader = LoadShader(b::embed<"res/shaders/fragment.glsl">().data(), GL_FRAGMENT_SHADER);

	// Attach the finalised shaders to a shader program to be used by the rest of the program.
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);

	GLint success = 0;

	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (success == 0) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "Failed to link shader program:\n" << infoLog << '\n';
		exit(1);
	}

	glValidateProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);

	if (success == 0) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "Failed to validate shader program:\n" << infoLog << '\n';
		exit(1);
	}

	glUseProgram(shaderProgram);

	// Clean up shaders now they are inside the shader program.
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return shaderProgram;
}

Entity::Entity(const Model& model)
{
	LoadModel(model);
}

void Entity::Draw(glm::mat4 mvp) const
{
	// Apply translation / position to the matrix.
	mvp = glm::translate(mvp, glm::vec3(m_position.x, m_position.y, m_position.z));

	// Apply rotation to the matrix.
	mvp = glm::rotate(mvp, glm::radians(m_rotation.x), glm::vec3(1.0F, 0.0F, 0.0F));
	mvp = glm::rotate(mvp, glm::radians(m_rotation.y), glm::vec3(0.0F, 1.0F, 0.0F));
	mvp = glm::rotate(mvp, glm::radians(m_rotation.z), glm::vec3(0.0F, 0.0F, 1.0F));

	// Apply scale to the matrix.
	mvp = glm::scale(mvp, glm::vec3(m_scale.x, m_scale.y, m_scale.z));

	// Pass the completed matrix to the GPU to be applied in the shader to the vector position.
	glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, &mvp[0][0]);

	// Bind the VAO referencing the vertex and indices buffers.
	glBindVertexArray(m_vao);

	// Draw the given data to the screen.
	glDrawElements(GL_TRIANGLES, m_indicesCount, GL_UNSIGNED_INT, nullptr);

	// Unbind the VAO to ensure a clear OpenGL state.
	glBindVertexArray(0);
}

void Entity::LoadModel(const Model& model)
{
	const GLuint shaderProgram = InitShaders();

	// Clear out the previous model if it exists.
	if (HasModel()) {
		glDeleteVertexArrays(1, &m_vao);
	}

	m_mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
	m_indicesCount = model.indices.size();

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Allocate, configure and bind the buffers.
	GLuint VBO = 0;
	GLuint IBO = 0;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(model.vertices.size() * sizeof(Vertex)),
	             model.vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(model.indices.size() * sizeof(uint32_t)),
	             model.indices.data(), GL_STATIC_DRAW);

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

bool Entity::HasModel() const
{
	return m_vao != 0;
}

void Entity::SetPosition(const glm::vec3& position)
{
	m_position = position;
}

void Entity::SetRotation(const glm::vec3& rotation)
{
	// Disallow setting rotations higher than 360 degrees.
	assert(rotation.x < 360.0F || rotation.y < 360.0F || rotation.z < 360.0F);

	m_rotation = rotation;
}

void Entity::Rotate(const glm::vec3& rotation)
{
	m_rotation += rotation;

	// Ensure rotation does not go over 360 degrees to avoid precision loss.
	m_rotation.x = m_rotation.x >= 360.0F ? m_rotation.x - 360.0F : m_rotation.x;
	m_rotation.y = m_rotation.y >= 360.0F ? m_rotation.y - 360.0F : m_rotation.y;
	m_rotation.z = m_rotation.z >= 360.0F ? m_rotation.z - 360.0F : m_rotation.z;
}

void Entity::SetScale(const glm::vec3& scale)
{
	m_scale = scale;
}

void Entity::ResetRotation()
{
	SetRotation(glm::vec3(0.0F, 0.0F, 0.0F)); // Reset object rotation.
}