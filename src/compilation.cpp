#include "compilation.h"
#include <chrono>
#include <iostream>
#include <microstl.h>
#include <thread>
#include <vector>

float GetDepth(const int index, const Config* config, const Image& image) {
	const int rgbaIndex = index * 4;

	// Calculate the grayscale out of the RGB values, weighted by the config.
	const float grayScale = config->sliderGsPref[0] * image.data[rgbaIndex] +
	                        config->sliderGsPref[1] * image.data[rgbaIndex + 1] +
	                        config->sliderGsPref[2] * image.data[rgbaIndex + 2];

	// TODO: Implement alpha slider (config->sliderGsPref[3]) to scale the alpha between inverted and not.

	// Normalise the gray scale value and flip the result for alpha processing.
	float depth = 1 - grayScale / 255;

	// Fully transparent pixels are made thinnest and opaque is unmodified.
	depth *= image.data[rgbaIndex + 3] / 255.0F;

	// Make the output negative to ensure the mesh builds in the correct direction.
	return -depth;
}

void CompileModel(Model& model, const Config* config, const Image& image) {
	std::cout << "Compiling mesh...\n";
	std::flush(std::cout);

	const auto startTime = std::chrono::high_resolution_clock::now();
	const int pixelCount = image.width * image.height;

	// Reset the model to blank before we begin editing. Maybe we can avoid allocated the model initially if we do it
	// here, or avoid the double allocation some other way.
	model = Model{};

	// Pre-allocate the space to avoid dynamic memory overhead.
	// - Each row and column of vertices is just the pixel count plus 1, multiplied by each other with give the total
	// amount. This is then doubled to fit in the back panel.
	// - Indices prediction is simply 6 per pixel as each pixel is two triangles. This is doubled for the back panel
	// with an extra 6 for each edge pixel to connect them.

	const size_t frontVertexCount = (image.width + 1) * (image.height + 1);
	const size_t frontIndexCount = pixelCount * 6;

	model.vertices.resize(frontVertexCount * 2);
	model.indices.reserve(frontIndexCount * 2 + ((image.width + image.height) * 2) * 6);

	// This will calculate the size of each pixel to create the target size. As aspect ratio is enforced, we only
	// need to calculate the size of one side of the pixel as they will be equal.
	const float pixelSize = config->sliderWidth / image.width;

	const float depthMax = config->sliderThickMax;
	const float depthMin = config->sliderThickMin;

	// Stores the current column the pixel belongs to.
	int column = 0;

	// Keeps track of the vertex count, needed as last row is inserted parallel.
	size_t nextIndex = 0;

	for (int pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {
		const int row = pixelIndex / image.width;

		// Gather the current state of the pixel.
		const bool firstRow = row == 0;
		const bool lastRow = row == image.height - 1;
		const bool firstInRow = pixelIndex - row * image.width == 0;
		const bool lastInRow = pixelIndex - row * image.width == image.width - 1;

		// === Vertex Generation ===

		const float depth = GetDepth(pixelIndex, config, image);

		if (firstInRow) {
			column = 0;

			// Calculate the average for left side edge vertices, leaving the top left and bottom right points
			// unmodified.
			const float firstDepth =
				!firstRow ? (depth + GetDepth(pixelIndex - image.width, config, image)) / 2 : depth;

			// The first vertex of every row.
			const glm::vec3 firstVert(-(column * pixelSize), -row * pixelSize, firstDepth * depthMax);

			model.vertices[nextIndex] = Vertex(firstVert, glm::vec3(1 - -firstDepth));
			model.vertices[frontVertexCount + nextIndex] =
				Vertex(glm::vec3(firstVert.x, firstVert.y, depthMin), glm::vec3(0));

			// Last row includes creating the final row in parallel.
			if (lastRow) {
				const glm::vec3 secondVert(-(column * pixelSize), (-row - 1) * pixelSize, depth * depthMax);

				model.vertices[nextIndex + (image.width + 1)] = Vertex(secondVert, glm::vec3(1 - -depth));
				model.vertices[frontVertexCount + nextIndex + (image.width + 1)] =
					Vertex(glm::vec3(secondVert.x, secondVert.y, depthMin), glm::vec3(0));
			}

			nextIndex++;
		}

		float secondDepth = 0;

		if (lastInRow && firstRow) {
			secondDepth = depth; // For the top right pixel.
		} else if (firstRow) {
			secondDepth = (depth + GetDepth(pixelIndex + 1, config, image)) / 2; // For normal top pixels.
		} else if (lastInRow) {
			secondDepth = (depth + GetDepth(pixelIndex - image.width, config, image)) / 2; // For normal right pixels.
		} else {
			// For normal pixels.
			secondDepth =
				(depth + GetDepth(pixelIndex + 1, config, image) + GetDepth(pixelIndex - image.width, config, image) +
			     GetDepth(pixelIndex - image.width + 1, config, image)) /
				4;
		}

		// The most common type of vertex.
		const glm::vec3 thirdVert(-(column * pixelSize + pixelSize), -row * pixelSize, secondDepth * depthMax);

		model.vertices[nextIndex] = Vertex(thirdVert, glm::vec3(1 - -secondDepth));
		model.vertices[frontVertexCount + nextIndex] =
			Vertex(glm::vec3(thirdVert.x, thirdVert.y, depthMin), glm::vec3(0));

		// Last row includes creating the final row in parallel.
		if (lastRow) {
			// Calculate the average for the last row pixels, leaving the bottom right pixel alone.
			const float thirdDepth = !lastInRow ? (depth + GetDepth(pixelIndex + 1, config, image)) / 2 : depth;

			const glm::vec3 fourthVert(-(column * pixelSize + pixelSize), (-row - 1) * pixelSize,
			                           thirdDepth * depthMax);

			model.vertices[nextIndex + (image.width + 1)] = Vertex(fourthVert, glm::vec3(1 - -thirdDepth));
			model.vertices[frontVertexCount + nextIndex + (image.width + 1)] =
				Vertex(glm::vec3(fourthVert.x, fourthVert.y, depthMin), glm::vec3(0));
		}

		nextIndex++;

		// === Index Generation ===

		// Build indices map for current pixel.
		// - Adding the pixel index will shift the triangles along by 1 each time, creating the grid.
		// - Adding the row will ensure the indices does not attempt to wrap one side of the plane to the other
		// through skipping the triangles that would cause this.

		// Invert the triangles every other column and invert that every other row.
		if (((pixelIndex ^ row) & 1) == 0) {
			// Front Panel
			model.indices.push_back(pixelIndex + row + 0);
			model.indices.push_back(pixelIndex + row + (1 + (image.width + 1)));
			model.indices.push_back(pixelIndex + row + (0 + (image.width + 1)));

			model.indices.push_back(pixelIndex + row + 1);
			model.indices.push_back(pixelIndex + row + (1 + (image.width + 1)));
			model.indices.push_back(pixelIndex + row + 0);

			// Back Panel
			model.indices.push_back(frontVertexCount + pixelIndex + row + 0);
			model.indices.push_back(frontVertexCount + pixelIndex + row + (0 + (image.width + 1)));
			model.indices.push_back(frontVertexCount + pixelIndex + row + 1);

			model.indices.push_back(frontVertexCount + pixelIndex + row + 1);
			model.indices.push_back(frontVertexCount + pixelIndex + row + (0 + (image.width + 1)));
			model.indices.push_back(frontVertexCount + pixelIndex + row + (1 + (image.width + 1)));
		} else {
			// Front Panel
			model.indices.push_back(pixelIndex + row + (1 + (image.width + 1)));
			model.indices.push_back(pixelIndex + row + (0 + (image.width + 1)));
			model.indices.push_back(pixelIndex + row + 1);

			model.indices.push_back(pixelIndex + row + (0 + (image.width + 1)));
			model.indices.push_back(pixelIndex + row + 0);
			model.indices.push_back(pixelIndex + row + 1);

			// Back Panel
			model.indices.push_back(frontVertexCount + pixelIndex + row + 1);
			model.indices.push_back(frontVertexCount + pixelIndex + row + 0);
			model.indices.push_back(frontVertexCount + pixelIndex + row + (1 + (image.width + 1)));

			model.indices.push_back(frontVertexCount + pixelIndex + row + 0);
			model.indices.push_back(frontVertexCount + pixelIndex + row + (0 + (image.width + 1)));
			model.indices.push_back(frontVertexCount + pixelIndex + row + (1 + (image.width + 1)));
		}

		column++;
	}

	// TODO: This works but it sucks.
	// Acquire centre offset to centre the mesh in the view port later and ensure it is still accurate if there is an
	// odd amount.
	if (const size_t totalVertices = frontVertexCount - 1; totalVertices % 2 == 0) {
		const glm::vec3 pos = model.vertices[totalVertices / 2].position;
		model.centerOffset = glm::vec3(pos.x, pos.y, 0.0F);
	} else {
		const glm::vec3 pos = model.vertices[model.indices[(frontIndexCount - 1) / 2]].position;
		model.centerOffset = glm::vec3(pos.x, pos.y, 0.0F);
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

struct CustomMeshProvider : microstl::Writer::Provider {
	const Model& model;
	bool ascii = false; // Write out the STL in ASCII text format as opposed to binary.
	bool clearNormals = true; // Ignore the n value below.

	CustomMeshProvider(const Model& m) : model(m) {}

	bool asciiMode() override {
		return ascii;
	}
	bool nullifyNormals() override {
		return clearNormals;
	}

	size_t getFacetCount() override {
		return model.indices.size() / 3;
	}

	void getFacet(size_t index, float v1[3], float v2[3], float v3[3], float n[3]) override {
		index *= 3;

		v1[0] = model.vertices[model.indices[index]].position.x;
		v1[1] = model.vertices[model.indices[index]].position.y;
		v1[2] = model.vertices[model.indices[index]].position.z;
		v2[0] = model.vertices[model.indices[index + 1]].position.x;
		v2[1] = model.vertices[model.indices[index + 1]].position.y;
		v2[2] = model.vertices[model.indices[index + 1]].position.z;
		v3[0] = model.vertices[model.indices[index + 2]].position.x;
		v3[1] = model.vertices[model.indices[index + 2]].position.y;
		v3[2] = model.vertices[model.indices[index + 2]].position.z;
	}
};

void WriteModel(const char* filePath, const Model& model) {
	using namespace microstl;

	CustomMeshProvider provider(model);

	if (const Result result = Writer::writeStlFile(filePath, provider); result != Result::Success) {
		std::cerr << "Failed to write stl file!\n";
	}

	std::cout << "Written mesh to disk as \"" << filePath << "\".\n";
	std::flush(std::cout);
}