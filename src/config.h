#pragma once

// Min max values for the sliders.
// Normalised float value sliders are always between 0 and 1.
#define SLIDER_WIDTH_MIN 1.0F
#define SLIDER_WIDTH_MAX 2000.0F
#define SLIDER_HEIGHT_MIN 1.0F
#define SLIDER_HEIGHT_MAX 2000.0F
#define SLIDER_THICK_MIN 0.001F
#define SLIDER_THICK_MAX 20.0F

// The format for sliders.
#define SLIDER_FLOAT_FORMAT_MM "%.3F mm"
#define SLIDER_FLOAT_FORMAT "%.3F"

struct Config {
	// Menu Bar
	bool drawSource = true;
	bool drawPreview = true;
	bool drawWireframe = false;

	// Side Panel
	float sliderWidth = 100.0F;
	float sliderHeight = 100.0F;
	float sliderThickMin = 0.8F;
	float sliderThickMax = 3.2F;
	float sliderGsPref[4] = {0.3F, 0.59F, 0.11F, 0.0F};

	const char* dropdownMeshTypes[1] = {"Plane"};
	int dropdownMesh = 0;

	// Backend
	bool aboutOpened = false;
};