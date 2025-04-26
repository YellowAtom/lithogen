#pragma once

#include <glm/vec3.hpp>

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;

	Vertex() = default;
	Vertex(const glm::vec3 position, const glm::vec3 color) : position(position), color(color) {}
};