
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include "display.h"
#include "menu.h"
#include "constants.h"

GLFWwindow* g_mainWindow;

#ifdef OS_WINDOWS
#include <windows.h>
#include <dwmapi.h>
#include "glfw/glfw3native.h"

void ConfigureWindows(GLFWwindow* window) {
	// Gather the windows native window handlers.
	HWND hWindow = glfwGetWin32Window(window);
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	// Force the window title bar to dark mode through the dwmapi.
	// TODO: Figure out how to implement responsive dark mode while using GLFW3.
	constexpr BOOL value = true;
	DwmSetWindowAttribute(hWindow, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

	// Set the .ico from the resource file as the window icon.
	HICON hIcon = LoadIcon(hInstance, "LithoGenIcon");
	SendMessage(hWindow, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
	SendMessage(hWindow, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
}
#endif

int main(int argc, char* argv[]) {
	// Initialize the GLFW3 library.
	if (!glfwInit())
		return 1;

	// The required OpenGL version (4.6).
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	// Avoids old and backwards compatible OpenGL features.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Initialize the single GLFW3 window used by this application, this also initializes an OpenGL context.
	g_mainWindow = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "LithoGen", nullptr, nullptr);

	if (!g_mainWindow)
		return 1;

#ifdef OS_WINDOWS
	ConfigureWindows(g_mainWindow);
#endif

	// Set the OpenGL context of this window as active for this thread.
	glfwMakeContextCurrent(g_mainWindow);

	// Load OpenGL for glad and avoid glad doing what GLFW3 has already done by attaching them.
	if (gladLoadGL(glfwGetProcAddress) == 0)
		return 1;

	DisplayInit();
	MenuInit();

	while (!glfwWindowShouldClose(g_mainWindow)) {
		DisplayDraw();
		MenuDraw();

		glfwSwapBuffers(g_mainWindow); // Push the prepared frame buffer to the screen.
		glfwPollEvents(); // Process the OS's window events, in other words, gathering inputs and window state from the OS.
	}

	MenuShutdown();

	// GLFW3 cleanup.
	glfwTerminate();

	return 0;
}
