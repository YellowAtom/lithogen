#pragma once

#include "glfw/glfw3.h"
#include "camera.h"
#include "entity.h"
#include "menu_config.h"

class DisplayData {
public:
	explicit DisplayData(GLFWwindow* window, MenuConfig* config, const Model& model);
	void Draw();

	void CalcViewport(int width, int height);

	[[nodiscard]] int GetViewportWidth() const;
	[[nodiscard]] int GetViewportHeight() const;
	[[nodiscard]] float GetAspectRatio() const;

	Camera camera;
	Entity entity;
private:
	GLFWwindow* m_window;
	MenuConfig* m_config;

	unsigned int m_shaderProgram = 0;

	int m_viewportWidth = 0;
	int m_viewportHeight = 0;
	float m_aspectRatio = 0.0f;
};
