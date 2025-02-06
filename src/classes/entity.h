#pragma once

#include "glm/glm.hpp"
#include "model.h"

class Entity {
public:
	explicit Entity(const Model& model, unsigned int shaderProgram);
	void Draw(glm::mat4 mvp) const;

	void SetPosition(const glm::vec3& position);
	void SetRotation(const glm::vec3& rotation);
	void AddRotation(const glm::vec3& rotation);
	void SetScale(const glm::vec3& scale);
private:
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	unsigned int m_vao;
	int m_mvpLoc;
};
