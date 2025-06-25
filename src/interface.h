// SPDX-License-Identifier: GPL-3.0
#pragma once

#include "GLFW/glfw3.h"
#include "declarations/config.h"
#include "declarations/structures.h"
#include "renderer/render.h"

void RenderInterface(GLFWwindow* window, Image& image, Config* config, Model& model, Render* render);