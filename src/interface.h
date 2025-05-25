#pragma once
#include "GLFW/glfw3.h"
#include "config.h"
#include "image.h"
#include "model.h"
#include "render.h"

void RenderInterface(GLFWwindow* window, Image& image, Config* config, Model& model, Render* render);