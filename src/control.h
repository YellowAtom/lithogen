#pragma once

#include "config.h"
#include "render.h"

struct glfwUserData {
	bool cursorWithinViewport = false;

	Config* config;
	Render* render;

	glfwUserData(Config* config, Render* render) : config(config), render(render) {}
};

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double x, double y);
void ScrollCallback(GLFWwindow* window, double x, double y);