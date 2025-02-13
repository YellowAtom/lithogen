#pragma once

#include "glfw/glfw3.h"
#include "camera.h"
#include "entity.h"
#include "config.h"

class Render {
public:
	explicit Render(GLFWwindow* window, Config* config);
	void Draw() const;

	void UpdateWireframe() const;
	void CalcViewport(int width, int height);

	[[nodiscard]] int GetViewportWidth() const;
	[[nodiscard]] int GetViewportHeight() const;
	[[nodiscard]] float GetAspectRatio() const;

	Camera camera{};
	Entity entity{};
private:
	GLFWwindow* m_window = nullptr;
	Config* m_config = nullptr;

	int m_viewportWidth = 0;
	int m_viewportHeight = 0;
	float m_aspectRatio = 0.0f;
};
