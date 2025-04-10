#pragma once

#include <glad/gl.h>
#include <stb_image.h>

struct Image {
	int width = 0;
	int height = 0;
	int aspectRatioW = 0;
	int aspectRatioH = 0;
	stbi_uc* data = nullptr;
	GLuint texture = 0;
};