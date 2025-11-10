// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "../declarations/structures.h"

class Entity {
public:
	Entity() = default;
	explicit Entity(const Model& model);
	void Draw(glm::mat4 mvp) const;

	void LoadModel(const Model& model);
	[[nodiscard]] bool HasModel() const;

	void SetPosition(const glm::vec3& position);
	void SetRotation(const glm::vec3& rotation);
	void Rotate(const glm::vec3& rotation);
	void SetScale(const glm::vec3& scale);
	void ResetRotation();
private:
	glm::vec3 m_position = glm::vec3(0.0F, 0.0F, 0.0F);
	glm::vec3 m_rotation = glm::vec3(0.0F, 0.0F, 0.0F);
	glm::vec3 m_scale = glm::vec3(1.0F, 1.0F, 1.0F);

	unsigned int m_vao = 0;
	int m_mvpLoc = 0;
	size_t m_indicesCount = 0;
};