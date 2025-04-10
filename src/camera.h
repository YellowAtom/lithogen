#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

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
	glm::vec3 m_position = glm::vec3(0.0F, 0.0F, 0.0F);
	glm::vec3 m_target = glm::vec3(0.0F, 0.0F, 1.0F);
	glm::vec3 m_up = glm::vec3(0.0F, 1.0F, 0.0F);

	float m_fov = 90.0F;
	float m_nearZ = 0.1F;
	float m_farZ = 100.0F;
};