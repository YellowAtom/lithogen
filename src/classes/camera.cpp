
#include "camera.h"
#include "../matrix_math.h"

void Camera::ApplyMatrix(glm::mat4& mvp, const float aspectRatio) const {
	CreateProjMatrix(mvp,
		m_fov,
		m_nearZ,
		m_farZ,
		aspectRatio
	);

	CreateViewMatrix(mvp,
		m_position,
		m_target,
		m_up
	);
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
