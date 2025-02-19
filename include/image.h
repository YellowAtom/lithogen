#pragma once

struct Image {
	int width = 0;
	int height = 0;
	int aspectRatioW = 0;
	int aspectRatioH = 0;
	unsigned char* data = nullptr;
	unsigned int texture = 0;
};
