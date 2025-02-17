#pragma once

// Min max values for the sliders.
// Normalised float value sliders are always between 0 and 1.
#define SLIDER_WIDTH_MIN 1.0f
#define SLIDER_WIDTH_MAX 500.0f

#define SLIDER_HEIGHT_MIN 1.0f
#define SLIDER_HEIGHT_MAX 500.0f

#define SLIDER_THICK_MIN 0.001f
#define SLIDER_THICK_MAX 20.0f

// The format for sliders.
#define SLIDER_FLOAT_FORMAT_MM "%.3f mm"
#define SLIDER_FLOAT_FORMAT "%.3f"

struct Config {
	// Menu Bar
	bool drawSource = true;
	bool drawPreview = true;
	bool drawWireframe = false;

	// Side Panel
	float sliderWidth = 100.0f;
	float sliderHeight = 100.0f;
	float sliderThickMin = 0.001f;
	float sliderThickMax = 20.0f;
	float sliderGsPref[4] = {0.3f, 0.59f, 0.11f, 0.0f};

	// Backend
	bool aboutOpened = false;
};
