#pragma once

#include <utility>
#include <vector>
#include "vertex.h"

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	Model(std::vector<Vertex> vertices, std::vector<unsigned int> indices) : vertices(std::move(vertices)), indices(std::move(indices)) {}
};
