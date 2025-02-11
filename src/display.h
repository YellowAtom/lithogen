#pragma once

#include <glfw/glfw3.h>
#include "display_data.h"
#include "menu_config.h"

DisplayData* DisplayInit(GLFWwindow* window);
void DisplayDraw(GLFWwindow* window, DisplayData* display, MenuConfig* config);
