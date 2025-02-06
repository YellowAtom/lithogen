
#include "matrix_math.h"

void CreateProjMatrix(glm::mat4& mvp, const float fov, const float nearZ, const float farZ, const float aspectRatio) {
	const float tanHalfFOV = tanf(glm::radians(fov / 2));
	const float d = 1.0f / tanHalfFOV;

	float zRange = nearZ - farZ;
	float a = (-farZ - nearZ) / zRange;
	float b = 2.0f * farZ * nearZ / zRange;

	// The projection matrix. calculated form aspect ratio, fov and clip planes.
	mvp *= glm::mat4(
		d / aspectRatio, 0.0f, 0.0f, 0.0f,
		0.0f, d, 0.0f, 0.0f,
		0.0f, 0.0f, a, 1.0f,
		0.0f, 0.0f, b, 0.0f
	);
}

void CreateViewMatrix(glm::mat4& mvp, const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
	glm::mat4 cameraTranslation(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-position.x, -position.y, -position.z, 1.0f
	);

	glm::vec3 n = normalize(target);
	glm::vec3 u = normalize(cross(up, n));
	glm::vec3 v = cross(n, u);

	glm::mat4 cameraRotation(
		u.x, v.x, n.x, 0.0f,
		u.y, v.y, n.y, 0.0f,
		u.z, v.z, n.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	mvp *= cameraTranslation * cameraRotation;
}

void CreateModelMatrix(glm::mat4& mvp, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
	// Apply scale to the matrix.
	glm::mat4 s(
		scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Apply translation / position to the matrix.
	glm::mat4 t(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		position.x, position.y, position.z, 1.0f
	);

	// Apply rotation to the matrix.
	float rotateX = glm::radians(rotation.x);
	float rotateY = glm::radians(rotation.y);
	float rotateZ = glm::radians(rotation.z);

	glm::mat4 rx(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosf(rotateX), sinf(rotateX), 0.0f,
		0.0f, -sinf(rotateX), cosf(rotateX), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::mat4 ry(
		cosf(rotateY), 0.0f, sinf(rotateY), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sinf(rotateY), 0.0f, cosf(rotateY), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::mat4 rz(
		cosf(rotateZ), 0.0f, sinf(rotateZ), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sinf(rotateZ), 0.0f, cosf(rotateZ), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Combine the matrices into the world matrix in the correct order.
	mvp *= t * (rz * ry * rx) * s;
}
