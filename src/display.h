#pragma once

#include <glfw/glfw3.h>
#include "classes/display_data.h"

DisplayData* DisplayInit(GLFWwindow* window);
void DisplayDraw(DisplayData* display, GLFWwindow* window);
