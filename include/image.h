#pragma once

struct Image {
	int width = 0;
	int height = 0;
	unsigned char* data = nullptr;
	unsigned int texture = 0;
};
