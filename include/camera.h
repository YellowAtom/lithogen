#pragma once
#include "glm/glm.hpp"

class Camera {
public:
	Camera() = default;
	void ApplyMatrix(glm::mat4& mvp, float aspectRatio) const;

	void SetPosition(const glm::vec3& position);
	void Move(const glm::vec3& movement);
	void SetTargetPos(const glm::vec3& target);
	void SetUpDirection(const glm::vec3& up);

	void SetFov(float fov);
	void SetNearZ(float nearZ);
	void SetFarZ(float farZ);
private:
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

	float m_fov = 90.0f;
	float m_nearZ = 0.1f;
	float m_farZ = 100.0f;
};