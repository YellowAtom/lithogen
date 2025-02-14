#pragma once

struct Config {
	// Menu Bar
	bool drawSource = true;
	bool drawPreview = true;
	bool drawWireframe = false;

	// Side Panel
	float sliderGsPref[3] = {0.3f, 0.59f, 0.11f};

	// Backend
	bool aboutOpened = false;
};
