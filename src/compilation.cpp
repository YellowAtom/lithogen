#include "compilation.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

void CompileModel(Model& model, const Config* config, const Image& image) {
	const auto startTime = std::chrono::high_resolution_clock::now();
	const int pixelCount = image.width * image.height;

	// TODO: Move vertices from resize() to reserve() if it is faster. This will require implementing the loop to only
	// use emplace_back.

	// Pre-allocate the space to avoid dynamic memory overhead.
	// - Each row and column of vertices is just the pixel count plus one, multiplied by each other with give the total
	// amount.
	// - Indices prediction is simply 6 per pixel as each pixel is two triangles.
	model.vertices.resize((image.width + 1) * (image.height + 1));
	model.indices.reserve(pixelCount * 6);

	// TODO: Add option to change pixel size.
	constexpr float pixelSize = 0.01F; // Millimeters seems to be unit x2.
	constexpr float depthScale = 0.05F;

	// Stores the current column the pixel belongs to.
	int column = 0;
	// Pixels must be read backwards as the mesh is generated from the bottom left. TODO: Maybe this could be avoided.
	int nextHeight = 0;
	// Keeps track of the vertex count, needed as last row is inserted parallel.
	size_t nextIndex = 0;

	for (int i = 0; i < pixelCount; i++) {
		const int row = i / image.width;
		const bool lastRow = row == image.height - 1;
		const bool firstInRow = i - row * image.width == 0;

		// === Image Processing ===

		if (firstInRow) {
			nextHeight = (image.height - 1 - row) * image.width;
		}

		const int rgbaIndex = nextHeight * 4;

		// Calculate the grayscale out of the RGB values, weighted by the config.
		const float grayScale = config->sliderGsPref[0] * image.data[rgbaIndex] +
								config->sliderGsPref[1] * image.data[rgbaIndex + 1] +
								config->sliderGsPref[2] * image.data[rgbaIndex + 2];

		// TODO: Implement "sliderGsPref[3]" to scale the alpha between inverted and not.

		// Normalise the gray scale value and flip the result for alpha processing.
		float depth = 1 - grayScale / 255;

		// Fully transparent pixels are made thinnest and opaque is unmodified.
		depth *= image.data[rgbaIndex + 3] / 255.0F;

		// Flip the depth again to return values to the expected output.
		depth = 1 - depth;

		nextHeight++;

		// === Vertex Generation ===

		if (firstInRow) {
			column = 0;

			model.vertices[nextIndex] =
				Vertex(glm::vec3(column * pixelSize, row * pixelSize, depth * depthScale), glm::vec3(depth));

			if (lastRow) {
				model.vertices[nextIndex + (image.width + 1)] =
					Vertex(glm::vec3(column * pixelSize, (row + 1) * pixelSize, depth * depthScale), glm::vec3(depth));
			}

			nextIndex++;
		}

		model.vertices[nextIndex] =
			Vertex(glm::vec3(column * pixelSize + pixelSize, row * pixelSize, depth * depthScale), glm::vec3(depth));

		if (lastRow) {
			model.vertices[nextIndex + (image.width + 1)] = Vertex(
				glm::vec3(column * pixelSize + pixelSize, (row + 1) * pixelSize, depth * depthScale), glm::vec3(depth));
		}

		nextIndex++;

		// === Index Generation ===

		// Build indices map for current pixel.
		// - Adding the pixel index will shift the triangles along by 1 each time, creating the grid.
		// - Adding the row will ensure the indices does not attempt to wrap one side of the plane to the other
		// through skipping the triangles that would cause this.

		// Invert the triangles every other column and invert that every other row.
		if (((i ^ row) & 1) == 0) {
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

	// TODO: This works but it sucks.
	// Acquire centre offset to centre the mesh in the view port later and ensure it is still accurate if there is an
	// odd amount.
	if (const size_t totalVertices = model.vertices.size() - 1; totalVertices % 2 == 0) {
		model.centerOffset = model.vertices[totalVertices / 2].position;
	} else {
		model.centerOffset = model.vertices[model.indices[(model.indices.size() - 1) / 2]].position;
	}

	const auto endTimePoint = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double, std::milli> elapsedTime = endTimePoint - startTime;

	std::cout << "Mesh compiled in " << elapsedTime.count() << "ms\n";
	std::flush(std::cout);

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