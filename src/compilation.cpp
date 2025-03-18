#include "compilation.h"
#include <cstdint>
#include <vector>
#include <random>

void CompileModel(Model& model, Config* config, const Image& image) {
	// Stores the image's grayscale values as normalised floats.
	std::vector<float> heightValues(image.width * image.height);

	const uint32_t imageValues = image.width * image.height * 4; // The total amount of bytes from within the 4 channel image.
	uint8_t progress = 0; // Calculate if we are looking at R, G, B or A.
	uint8_t rgba[4]; // A buffer to store that current RGBA values.

	for (uint32_t i = 0; i < imageValues; i++) {
		// Once we have called all 4 RGBA values.
		if (progress == 4) {
			progress = 0;

			// 1. Calculate the grayscale out of the RGB values.
			// 2. Fully transparent pixels are made thinnest and opaque is unmodified.
			// 3. Turn the alpha adjusted grayscale value into a normalized float.
			heightValues.push_back(
				(config->sliderGsPref[0] * rgba[0] + config->sliderGsPref[1] * rgba[1] + config->sliderGsPref[2] * rgba[2])
				* (config->sliderGsPref[3] * rgba[3] / 255.0f)
				/ 255.0f
			);
		}

		rgba[progress] = image.data[i];
		progress++;
	}

	// Configure a randomness generator for the demo colours.
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	// Temporary demo mesh, will be replaced with the lithophane generation pipeline.
	model.vertices = {
		Vertex(
			glm::vec3(0.5f, 0.5f, 0.5f),
			glm::vec3(dist(mt), dist(mt), dist(mt))
		),
		Vertex(
			glm::vec3(-0.5f, 0.5f, -0.5f),
			glm::vec3(dist(mt), dist(mt), dist(mt))
		),
		Vertex(
			glm::vec3(-0.5f, 0.5f, 0.5f),
			glm::vec3(dist(mt), dist(mt), dist(mt))
		),
		Vertex(
			glm::vec3(0.5f, -0.5f, -0.5f),
			glm::vec3(dist(mt), dist(mt), dist(mt))
		),
		Vertex(
			glm::vec3(-0.5f, -0.5f, -0.5f),
			glm::vec3(dist(mt), dist(mt), dist(mt))
		),
		Vertex(
			glm::vec3(0.5f, 0.5f, -0.5f),
			glm::vec3(dist(mt), dist(mt), dist(mt))
		),
		Vertex(
			glm::vec3(0.5f, -0.5f, 0.5f),
			glm::vec3(dist(mt), dist(mt), dist(mt))
		),
		Vertex(
			glm::vec3(-0.5f, -0.5f, 0.5f),
			glm::vec3(dist(mt), dist(mt), dist(mt))
		)
	};

	model.indices = {
		0, 1, 2,
		1, 3, 4,
		5, 6, 3,
		7, 3, 6,
		2, 4, 7,
		0, 7, 6,
		0, 5, 1,
		1, 5, 3,
		5, 0, 6,
		7, 4, 3,
		2, 1, 4,
		0, 2, 7
	};
}