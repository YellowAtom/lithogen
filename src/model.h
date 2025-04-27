#pragma once

#include <cstdint>
#include <glm/vec3.hpp>
#include <vector>
#include "vertex.h"

struct Model {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	glm::vec3 centerOffset;
};