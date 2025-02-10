#pragma once

#include <glfw/glfw3.h>
#include "classes/display_data.h"
#include "classes/menu_config.h"

DisplayData* DisplayInit(GLFWwindow* window);
void DisplayDraw(GLFWwindow* window, DisplayData* display, MenuConfig* config);
