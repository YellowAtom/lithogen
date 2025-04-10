#pragma once

#include <GLFW/glfw3.h>
#include "camera.h"
#include "config.h"
#include "entity.h"

class Render {
public:
	explicit Render(GLFWwindow* window, Config* config);
	void Draw() const;

	[[nodiscard]] Camera GetCamera() const;
	[[nodiscard]] Entity GetEntity() const;

	void UpdateWireframe() const;
	void CalcViewport(int width, int height);

	[[nodiscard]] int GetViewportWidth() const;
	[[nodiscard]] int GetViewportHeight() const;
	[[nodiscard]] float GetAspectRatio() const;
private:
	Camera m_camera;
	Entity m_entity;

	GLFWwindow* m_window = nullptr;
	Config* m_config = nullptr;

	int m_viewportWidth = 0;
	int m_viewportHeight = 0;
	float m_aspectRatio = 0.0F;
};