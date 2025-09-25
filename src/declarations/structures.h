// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <stb_image.h>
#include <vector>

struct Image {
	int width = 0;
	int height = 0;
	int aspectRatioW = 0;
	int aspectRatioH = 0;
	stbi_uc* data = nullptr;
	GLuint texture = 0;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;

	Vertex() = default;
	Vertex(const glm::vec3 position, const glm::vec3 color) : position(position), color(color) {}
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	glm::vec3 centerOffset;
};