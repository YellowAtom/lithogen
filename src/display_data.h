#pragma once

#include "glfw/glfw3.h"
#include "camera.h"
#include "entity.h"

class DisplayData {
public:
	DisplayData(Camera camera, Entity entity, unsigned int shaderProgram) : camera(camera), entity(entity), shaderProgram(shaderProgram) {}

	void CalcViewport(GLFWwindow* window, int width, int height);

	[[nodiscard]] int GetViewportWidth() const;
	[[nodiscard]] int GetViewportHeight() const;
	[[nodiscard]] float GetAspectRatio() const;

	Camera camera;
	Entity entity;
	unsigned int shaderProgram;
private:
	int m_viewportWidth = 0;
	int m_viewportHeight = 0;
	float m_aspectRatio = 0.0f;
};
