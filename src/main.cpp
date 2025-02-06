
#include <cstdlib>
#include <process.h>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include "display.h"
#include "constants.h"
#include "classes/entity.h"

#ifdef OS_WINDOWS
#include <windows.h>
#include <dwmapi.h>
#include "glfw/glfw3native.h"
#endif

int main(int argc, char* argv[]) {
	// Seed any calls to rand() in the application with the process id, will be different every time.
	srand(_getpid());

	// Initialize the GLFW3 library.
	if (!glfwInit())
		return 1;

	// The required OpenGL version (4.6).
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	// Avoids old and backwards compatible OpenGL features.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Initialize the single GLFW3 window used by this application, this also initializes an OpenGL context.
	GLFWwindow* mainWindow = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "LithoGen", nullptr, nullptr);

	if (!mainWindow)
		return 1;

#ifdef OS_WINDOWS
	// Give code its own scape so the variables get garbage collected in program lifetime.
	{
		// Gather the windows native window handlers.
		HWND hWindow = glfwGetWin32Window(mainWindow);
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

	// Set the OpenGL context of this window as active for this thread.
	glfwMakeContextCurrent(mainWindow);

	// Load OpenGL for glad and avoid glad doing what GLFW3 has already done by attaching them.
	if (gladLoadGL(glfwGetProcAddress) == 0)
		return 1;

	DisplayData* display = DisplayInit(mainWindow);

	while (!glfwWindowShouldClose(mainWindow)) {
		DisplayDraw(display, mainWindow);

		glfwSwapBuffers(mainWindow); // Push the prepared frame buffer to the screen.
		glfwPollEvents(); // Process the OS's window events, in other words, gathering inputs and window state from the OS.
	}

	// Cleanup
	glfwTerminate();

	return 0;
}
