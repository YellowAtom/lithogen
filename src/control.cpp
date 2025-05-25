#include "control.h"
#include "GLFW/glfw3.h"
#include "glad/gl.h"

void FramebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
	const auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));
	// Recalculate viewport size when the window is resized.
	data->render->CalcViewport(width, height);
}

void KeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
	const auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	if (!data->render->entity.HasModel()) {
		return;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		data->render->camera.ResetRotation();
	} else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		data->config->drawWireframe = !data->config->drawWireframe;
		data->render->UpdateWireframe();
	}
}

void CursorPosCallback(GLFWwindow* window, const double x, const double y) {
	auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	if (!data->render->entity.HasModel()) {
		return;
	}

	// TODO: Rotating the cube too much will make this inaccurate, find a way.

	// Store previous mouse location to calculate difference.
	// Static allows a variable to persist through multiple function calls.
	static double previousX = 0;
	static double previousY = 0;

	int width = 0;
	int height = 0;
	glfwGetWindowSize(window, &width, &height);

	data->cursorWithinViewport =
		width - x < data->render->GetViewportWidth() && height - y < data->render->GetViewportHeight();

	// Is left mouse down and is the cursor within the viewport.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GL_TRUE && data->cursorWithinViewport) {
		// Apply the difference between the previous and current mouse location to the camera's rotation.
		data->render->camera.RotateHorizontal((x - previousX) / 20);
		data->render->camera.RotateVertical((y - previousY) / 20);
	}

	previousX = x;
	previousY = y;
}

void ScrollCallback(GLFWwindow* window, const double x, const double y) {
	const auto* data = static_cast<glfwUserData*>(glfwGetWindowUserPointer(window));

	// Move 4mm per scroll.
	if (data->render->entity.HasModel() && data->cursorWithinViewport) {
		data->render->camera.Zoom(y * 4);
	}
}
