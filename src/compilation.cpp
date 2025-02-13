
#include "compilation.h"
#include <random>

void CompileModel(Model& model, Config* config, unsigned char* image) {
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
