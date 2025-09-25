// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

class Camera {
public:
	Camera() = default;
	void ApplyMatrix(glm::mat4& mvp, float aspectRatio) const;

	void SetTargetPos(const glm::vec3& target);
	void SetUpDirection(const glm::vec3& up);

	void RotateHorizontal(float radians);
	void RotateVertical(float radians);
	void ResetRotation();
	void Zoom(float amount);
	void SetZoom(float amount);

	void SetFov(float fov);
	void SetNearZ(float nearZ);
	void SetFarZ(float farZ);
private:
	glm::vec3 m_target = glm::vec3(0.0F, 0.0F, 0.0F);
	glm::vec3 m_up = glm::vec3(0.0F, 1.0F, 0.0F);

	float m_defaultAzimuthAngle = glm::radians(-90.0F); // Ensure default rotation is aligned with the z axis.

	float m_radius = 0.0F;
	float m_minRadius = 0.0F;
	float m_azimuthAngle = m_defaultAzimuthAngle;
	float m_polarAngle = 0.0F;

	float m_fov = 90.0F;
	float m_nearZ = 0.1F;
	float m_farZ = 8000.0F; // Z Clip after 4000mm.
};