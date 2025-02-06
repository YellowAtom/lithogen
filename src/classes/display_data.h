#pragma once

#include <glad/gl.h>
#include "glfw/glfw3.h"
#include "entity.h"
#include "menu_config.h"

class DisplayData {
public:
	DisplayData(Entity entity, MenuConfig config, unsigned int shaderProgram) : entity(entity), config(config), shaderProgram(shaderProgram) {}

	void CalcViewport(GLFWwindow* window, int width, int height);

	[[nodiscard]] int GetViewportWidth() const;
	[[nodiscard]] int GetViewportHeight() const;
	[[nodiscard]] float GetAspectRatio() const;

	Entity entity;
	MenuConfig config;
	unsigned int shaderProgram;
private:
	int m_viewportWidth = 0;
	int m_viewportHeight = 0;
	float m_aspectRatio = 0.0f;
};
