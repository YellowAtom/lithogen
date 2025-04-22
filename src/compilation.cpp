#include "compilation.h"
#include <cstdint>
#include <vector>

void CompileModel(Model& model, const Config* config, const Image& image) {
	const int pixelCount = image.width * image.height;

	// Stores the image's grayscale values as normalised floats.
	std::vector<float> pixelHeights;
	pixelHeights.reserve(pixelCount);

	uint8_t progress = 0; // Calculate if we are looking at R, G, B or A.
	stbi_uc rgba[4]; // A buffer to store the current RGBA values.

	// Loop through the total amount of bytes from within the 4 channel image.
	for (int i = 0; i <= pixelCount * 4; i++) {
		// Once we have called all 4 RGBA values.
		if (progress == 4) {
			progress = 0;

			// Calculate the grayscale out of the RGB values, weighted by the config.
			const float grayScale = config->sliderGsPref[0] * rgba[0] + config->sliderGsPref[1] * rgba[1] +
									config->sliderGsPref[2] * rgba[2];

			// TODO: Implement "sliderGsPref[3]" to scale the alpha between inverted and not.
			// Fully transparent pixels are made thinnest and opaque is unmodified, weighted by config.
			const float alphaScale = rgba[3] / 255.0F;

			// Turn the alpha adjusted grayscale value into a normalized float.
			pixelHeights.push_back(grayScale * alphaScale / 255.0F);
		}

		// TODO: Validate we aren't over reading.
		rgba[progress] = image.data[i];
		progress++;
	}

	// Pre-allocate the space to avoid dynamic memory overhead.
	// - Vertices prediction first calculates the vertex count of pixels touching the edge which will give 2 each, then
	// every other pixel which is one new vertex each.
	// - Indices prediction is simply 6 per pixel as each pixel is two triangles.
	model.vertices.reserve(image.width * 2 + image.height * 2 + pixelHeights.size() - (image.width + image.height - 1));
	model.indices.reserve(pixelHeights.size() * 6);

	// TODO: Different vertices need to interp height based on sounding vertices and colour needs to account for this.
	// Step 1: Make sure each line interps values horizontally
	// Step 2: Make sure the same happens vertically, which should be possible through being on the second row loop when
	// fishing the first row line.

	// TODO: Add option to change pixel size.
	constexpr float pixelSize = 1;

	int column = 0;

	for (int i = 0; i < pixelHeights.size(); i++) {
		const int row = i / image.width;
		const bool firstInRow = i - row * image.width == 0;
		// const float height = pixelHeights[i];

		if (firstInRow) {
			column = 0;
			model.vertices.emplace_back(glm::vec3(column * pixelSize, row * pixelSize, 0), glm::vec3(0, 0, 0));
		}

		model.vertices.emplace_back(glm::vec3(column * pixelSize + 1, row * pixelSize, 0), glm::vec3(0, 0, 0));

		// Only do this for the last row.
		if (row == image.height - 1) {
			if (firstInRow) {
				model.vertices.emplace_back(glm::vec3(column * pixelSize, (row + 1) * pixelSize, 0),
											glm::vec3(0, 0, 0));
			}

			model.vertices.emplace_back(glm::vec3(column * pixelSize + 1, (row + 1) * pixelSize, 0),
										glm::vec3(0, 0, 0));
		}

		// Build indices map for current pixel.
		// - Adding the pixel index will shift the triangles along by 1 each time, creating the grid.
		// - Adding the row will ensure the indices does not attempt to wrap one side of the plane to the other
		// through skipping the triangles that would cause this.

		// TODO: I think the organisation here is incorrect.
		model.indices.push_back(i + row + (0 + image.width));
		model.indices.push_back(i + row + 0);
		model.indices.push_back(i + row + (1 + image.width));

		model.indices.push_back(i + row + (1 + image.width));
		model.indices.push_back(i + row + 0);
		model.indices.push_back(i + row + 1);

		column++;
	}
}