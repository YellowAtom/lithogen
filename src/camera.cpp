#include "camera.h"

void Camera::ApplyMatrix(glm::mat4& mvp, const float aspectRatio) const {
	const float tanHalfFOV = tanf(glm::radians(m_fov / 2));
	const float d = 1.0f / tanHalfFOV;

	const float zRange = m_nearZ - m_farZ;
	const float a = (-m_farZ - m_nearZ) / zRange;
	const float b = 2.0f * m_farZ * m_nearZ / zRange;

	// The projection matrix. calculated form aspect ratio, fov and clip planes.
	mvp *= glm::mat4(
		d / aspectRatio, 0.0f, 0.0f, 0.0f,
		0.0f, d, 0.0f, 0.0f,
		0.0f, 0.0f, a, 1.0f,
		0.0f, 0.0f, b, 0.0f
	);

	const glm::mat4 cameraTranslation(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-m_position.x, -m_position.y, -m_position.z, 1.0f
	);

	const glm::vec3 n = normalize(m_target);
	const glm::vec3 u = normalize(cross(m_up, n));
	const glm::vec3 v = cross(n, u);

	const glm::mat4 cameraRotation(
		u.x, v.x, n.x, 0.0f,
		u.y, v.y, n.y, 0.0f,
		u.z, v.z, n.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	mvp *= cameraTranslation * cameraRotation;
}

void Camera::SetPosition(const glm::vec3& position) {
	m_position = position;
}

void Camera::Move(const glm::vec3& movement) {
	m_position += movement;
}

void Camera::SetTargetPos(const glm::vec3& target) {
	m_target = target;
}

void Camera::SetUpDirection(const glm::vec3& up) {
	m_up = up;
}

void Camera::SetFov(float fov) {
	m_fov = fov;
}

void Camera::SetNearZ(float nearZ) {
	m_nearZ = nearZ;
}

void Camera::SetFarZ(float farZ) {
	m_farZ = farZ;
}