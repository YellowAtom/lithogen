
#include <process.h>
#include <random>

#include <glad/gl.h>
#include <glfw/glfw3.h>

#include "constants.h"
#include "display_data.h"
#include "menu_config.h"

#ifdef OS_WINDOWS
#include <windows.h>
#include "glfw/glfw3native.h"
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
	GLFWwindow* mainWindow = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "LithoGen", nullptr, nullptr);

	if (!mainWindow)
		return 1;

#ifdef OS_WINDOWS
	// Give code its own scape so the variables get garbage collected in program lifetime.
	{
		// Gather the windows native window handlers.
		HWND hWindow = glfwGetWin32Window(mainWindow);
		HINSTANCE hInstance = GetModuleHandle(nullptr);

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

	// Seed any calls to rand() in the application with the process id, will be different every time.
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	// TODO: Make sure this gets cleaned up after its used.
	// Temporary demo mesh, will be replaced with the lithophane generation pipeline.
	Model cubeModel(
		{
			Vertex(
				glm::vec3(0.5f, 0.5f, 0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(-0.5f, 0.5f, -0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(-0.5f, 0.5f, 0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(0.5f, -0.5f, -0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(-0.5f, -0.5f, -0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(0.5f, 0.5f, -0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(0.5f, -0.5f, 0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			),
			Vertex(
				glm::vec3(-0.5f, -0.5f, 0.5f),
				glm::vec3(dist(mt), dist(mt), dist(mt))
			)
		},
		{
			0, 1, 2,
			1, 3, 4,
			5, 6, 3,
			7, 3, 6,
			2, 4, 7,
			0, 7, 6,
			0, 5, 1,
			1, 5, 3,
			5, 0, 6,
			7, 4, 3,
			2, 1, 4,
			0, 2, 7
		}
	);

	auto* config = new MenuConfig();
	DisplayData display(mainWindow, config, cubeModel);

	while (!glfwWindowShouldClose(mainWindow)) {
		display.Draw();

		glfwSwapBuffers(mainWindow); // Push the prepared frame buffer to the screen.
		glfwPollEvents(); // Process the OS's window events, in other words, gathering inputs and window state from the OS.
	}

	// Cleanup
	glfwTerminate();

	return 0;
}
