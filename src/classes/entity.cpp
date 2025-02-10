
#include <glad/gl.h>
#include "vertex.h"
#include "../matrix_math.h"
#include "entity.h"

#include <iostream>

Entity::Entity(const Model& model, unsigned int shaderProgram) {
	m_mvpLoc = glGetUniformLocation(shaderProgram, "mvp");

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Allocate and configure the buffers.
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vertex), &model.vertices[0], GL_STATIC_DRAW);

	unsigned int IBO;
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(Vertex), &model.indices[0], GL_STATIC_DRAW);

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

void Entity::Draw(glm::mat4 mvp) const {
	CreateModelMatrix(mvp, m_position, m_rotation, m_scale);

	// Pass the completed matrix to the GPU to be applied in the shader to the vector position.
	glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, &mvp[0][0]);

	// Bind the VAO referencing the vertex and indices buffers.
	glBindVertexArray(m_vao);

	// Draw the given data to the screen.
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

	// Unbind the VAO to ensure a clear OpenGL state.
	glBindVertexArray(0);
}

void Entity::SetPosition(const glm::vec3& position) {
	m_position = position;
}

void Entity::SetRotation(const glm::vec3& rotation) {
	// Disallow setting rotations higher than 360 degrees.
	assert(rotation.x > 360.0f || rotation.y > 360.0f || rotation.z > 360.0f);

	m_rotation = rotation;
}

void Entity::AddRotation(const glm::vec3& rotation) {
	m_rotation += rotation;

	// Ensure rotation does not go over 360 degrees to avoid precision loss.
	m_rotation.x = m_rotation.x >= 360.0f ? m_rotation.x - 360.0f : m_rotation.x;
	m_rotation.y = m_rotation.y >= 360.0f ? m_rotation.y - 360.0f : m_rotation.y;
	m_rotation.z = m_rotation.z >= 360.0f ? m_rotation.z - 360.0f : m_rotation.z;
}

void Entity::SetScale(const glm::vec3& scale) {
	m_scale = scale;
}
