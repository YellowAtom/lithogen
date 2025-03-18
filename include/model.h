#pragma once
#include <vector>
#include "vertex.h"

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};