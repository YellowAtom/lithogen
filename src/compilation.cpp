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

	// TODO: Move vertices from resize() to reserve() if it is faster. This will require implementing the loop to only
	// use emplace_back.

	// Pre-allocate the space to avoid dynamic memory overhead.
	// - Each row and column of vertices is just the pixel count plus one, multiplied by each other with give the total
	// amount.
	// - Indices prediction is simply 6 per pixel as each pixel is two triangles.
	model.vertices.resize((image.width + 1) * (image.height + 1));
	model.indices.reserve(pixelCount * 6);

	// TODO: Different vertices need to interp height based on sounding vertices and colour needs to account for this.
	// Step 1: Make sure each line interps values horizontally
	// Step 2: Make sure the same happens vertically, which should be possible through being on the second row loop when
	// fishing the first row line.

	// TODO: Add option to change pixel size.
	constexpr float pixelSize = 1.0F;

	int column = 0;
	size_t nextIndex = 0; // Keeps track of the vertex count, needed as last row is inserted parallel.

	for (int i = 0; i < pixelCount; i++) {
		// === Vertex Generation ===
		const int row = i / image.width;
		const bool lastRow = row == image.height - 1;
		// const float height = pixelHeights[i];

		// If the first in the row.
		if (i - row * image.height == 0) {
			column = 0;

			model.vertices[nextIndex] = Vertex(glm::vec3(column * pixelSize, row * pixelSize, 0), glm::vec3(0, 0, 0));

			if (lastRow) {
				model.vertices[nextIndex + (image.width + 1)] =
					Vertex(glm::vec3(column * pixelSize, (row + 1) * pixelSize, 0), glm::vec3(0, 0, 0));
			}

			nextIndex++;
		}

		model.vertices[nextIndex] = Vertex(glm::vec3(column * pixelSize + 1, row * pixelSize, 0), glm::vec3(0, 0, 0));

		if (lastRow) {
			model.vertices[nextIndex + (image.width + 1)] =
				Vertex(glm::vec3(column * pixelSize + 1, (row + 1) * pixelSize, 0), glm::vec3(0, 0, 0));
		}

		nextIndex++;

		// === Index Generation (Parallel but not connected) ===
		bool invertTriangles = i % 2 == 0;

		// Invert the invert every other row.
		if (row % 2 == 0) {
			invertTriangles = !invertTriangles;
		}

		// Build indices map for current pixel.
		// - Adding the pixel index will shift the triangles along by 1 each time, creating the grid.
		// - Adding the row will ensure the indices does not attempt to wrap one side of the plane to the other
		// through skipping the triangles that would cause this.
		if (invertTriangles) {
			model.indices.push_back(i + row + 0);
			model.indices.push_back(i + row + (0 + (image.width + 1)));
			model.indices.push_back(i + row + 1);

			model.indices.push_back(i + row + 1);
			model.indices.push_back(i + row + (0 + (image.width + 1)));
			model.indices.push_back(i + row + (1 + (image.width + 1)));
		} else {
			model.indices.push_back(i + row + 1);
			model.indices.push_back(i + row + 0);
			model.indices.push_back(i + row + (1 + (image.width + 1)));

			model.indices.push_back(i + row + 0);
			model.indices.push_back(i + row + (0 + (image.width + 1)));
			model.indices.push_back(i + row + (1 + (image.width + 1)));
		}

		column++;
	}

	// Debug Vertex Positions.
	/* for (int i = 0; i < model.vertices.size(); i++) {
		std::cout << i << " = " << model.vertices[i].position.x << ", " << model.vertices[i].position.y << '\n';
	}

	std::cout << "==============\n";

	// Debug Index List.
	for (int i = 0; i < model.indices.size(); i++) {
		std::cout << i / 6 + 1 << " = " << model.indices[i] << '\n';
	} */
}