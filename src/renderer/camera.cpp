// SPDX-License-Identifier: GPL-3.0
#include "camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <numbers>

void Camera::ApplyMatrix(glm::mat4& mvp, const float aspectRatio) const
{
	// The projection matrix.
	mvp *= glm::perspective(glm::radians(m_fov), aspectRatio, m_nearZ, m_farZ);

	const float cosinePolar = cos(m_polarAngle);

	// Calculate eye position.
	const glm::vec3 eye(m_target.x + m_radius * cosinePolar * cos(m_azimuthAngle),
	                    m_target.y + m_radius * sin(m_polarAngle),
	                    m_target.z + m_radius * cosinePolar * sin(m_azimuthAngle));

	// The view matrix.
	mvp *= glm::lookAt(eye, m_target, m_up);
}

void Camera::RotateHorizontal(const float radians)
{
	m_azimuthAngle += radians;

	// Keep azimuth angle within range <0..2PI) - it's not necessary, but can help in certain scenarios.
	constexpr float fullCircle = 2.0F * std::numbers::pi;

	m_azimuthAngle = fmodf(m_azimuthAngle, fullCircle);

	if (m_azimuthAngle < 0.0F) {
		m_azimuthAngle = fullCircle + m_azimuthAngle;
	}
}

void Camera::RotateVertical(const float radians)
{
	m_polarAngle += radians;

	// Check if the angle hasn't exceeded quarter of a circle to prevent flip, add a bit of epsilon like 0.001 radians
	constexpr float polarCap = std::numbers::pi / 2.0F - 0.001F;

	m_polarAngle = std::min(m_polarAngle, polarCap);
	m_polarAngle = std::max(m_polarAngle, -polarCap);
}

void Camera::ResetRotation()
{
	m_azimuthAngle = m_defaultAzimuthAngle;
	m_polarAngle = 0.0F;
}

void Camera::Zoom(const float amount)
{
	m_radius -= amount;
	m_radius = std::max(m_radius, m_minRadius);
}

void Camera::SetZoom(const float amount)
{
	m_radius = amount;
}

void Camera::SetTargetPos(const glm::vec3& target)
{
	m_target = target;
}

void Camera::SetUpDirection(const glm::vec3& up)
{
	m_up = up;
}

void Camera::SetFov(const float fov)
{
	m_fov = fov;
}

void Camera::SetNearZ(const float nearZ)
{
	m_nearZ = nearZ;
}

void Camera::SetFarZ(const float farZ)
{
	m_farZ = farZ;
}