#pragma once
#include <vector>
#include "vertex.h"

struct Model {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};