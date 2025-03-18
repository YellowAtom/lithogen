#include <algorithm>
#include <iostream>
#include <glad/gl.h>
#include <battery/embed.hpp>
#include "vertex.h"
#include "entity.h"
#include <cstring>

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

Entity::Entity(const Model& model) {
	LoadModel(model);
}

void Entity::Draw(glm::mat4 mvp) const {
	// Apply scale to the matrix.
	const glm::mat4 s(
		m_scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, m_scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, m_scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Apply translation / position to the matrix.
	const glm::mat4 t(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		m_position.x, m_position.y, m_position.z, 1.0f
	);

	// Apply rotation to the matrix.
	const float rotateX = glm::radians(m_rotation.x);
	const float rotateY = glm::radians(m_rotation.y);
	const float rotateZ = glm::radians(m_rotation.z);

	const glm::mat4 rx(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosf(rotateX), sinf(rotateX), 0.0f,
		0.0f, -sinf(rotateX), cosf(rotateX), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	const glm::mat4 ry(
		cosf(rotateY), 0.0f, sinf(rotateY), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sinf(rotateY), 0.0f, cosf(rotateY), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	const glm::mat4 rz(
		cosf(rotateZ), 0.0f, sinf(rotateZ), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sinf(rotateZ), 0.0f, cosf(rotateZ), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Combine the matrices into the world matrix in the correct order.
	mvp *= t * (rz * ry * rx) * s;

	// Pass the completed matrix to the GPU to be applied in the shader to the vector position.
	glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, &mvp[0][0]);

	// Bind the VAO referencing the vertex and indices buffers.
	glBindVertexArray(m_vao);

	// Draw the given data to the screen.
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

	// Unbind the VAO to ensure a clear OpenGL state.
	glBindVertexArray(0);
}

void Entity::LoadModel(const Model& model) {
	const unsigned int shaderProgram = InitShaders();

	// Clear out the previous model if it exists.
	if (HasModel()) {
		glDeleteVertexArrays(1, &m_vao);
	}

	m_mvpLoc = glGetUniformLocation(shaderProgram, "mvp");

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Allocate, configure and bind the buffers.
	unsigned int VBO, IBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vertex), &model.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(Vertex), &model.indices[0], GL_STATIC_DRAW);

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

bool Entity::HasModel() const {
	return m_vao != 0;
}

void Entity::SetPosition(const glm::vec3& position) {
	m_position = position;
}

void Entity::SetRotation(const glm::vec3& rotation) {
	// Disallow setting rotations higher than 360 degrees.
	assert(rotation.x < 360.0f || rotation.y < 360.0f || rotation.z < 360.0f);

	m_rotation = rotation;
}

void Entity::Rotate(const glm::vec3& rotation) {
	m_rotation += rotation;

	// Ensure rotation does not go over 360 degrees to avoid precision loss.
	m_rotation.x = m_rotation.x >= 360.0f ? m_rotation.x - 360.0f : m_rotation.x;
	m_rotation.y = m_rotation.y >= 360.0f ? m_rotation.y - 360.0f : m_rotation.y;
	m_rotation.z = m_rotation.z >= 360.0f ? m_rotation.z - 360.0f : m_rotation.z;
}

void Entity::SetScale(const glm::vec3& scale) {
	m_scale = scale;
}

void Entity::ResetRotation() {
	SetRotation(glm::vec3(0.0f, 0.0f, 0.0f)); // Reset object rotation.
}