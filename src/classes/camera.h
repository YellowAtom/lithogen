#pragma once

#include "glm/glm.hpp"

class Camera {
public:
	Camera() = default;
	void ApplyViewMatrix(glm::mat4& mvp) const;

	void SetPosition(const glm::vec3& position);
	void Move(const glm::vec3& movement);
	void SetTargetPos(const glm::vec3& target);
	void SetUpDirection(const glm::vec3& up);
private:
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
};
