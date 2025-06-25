// SPDX-License-Identifier: GPL-3.0
#pragma once

#include "GLFW/glfw3.h"
#include "config.h"
#include "render.h"
#include "structures.h"

void RenderInterface(GLFWwindow* window, Image& image, Config* config, Model& model, Render* render);