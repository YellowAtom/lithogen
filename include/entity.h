#pragma once

#include "glm/glm.hpp"
#include "model.h"

class Entity {
public:
	explicit Entity(const Model& model, unsigned int shaderProgram);
	void Draw(glm::mat4 mvp) const;

	void SetPosition(const glm::vec3& position);
	void SetRotation(const glm::vec3& rotation);
	void Rotate(const glm::vec3& rotation);
	void SetScale(const glm::vec3& scale);
	void ResetRotation();
private:
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

	unsigned int m_vao = 0;
	int m_mvpLoc;
};
