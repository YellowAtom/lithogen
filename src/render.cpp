
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "render.h"
#include "constants.h"

Render::Render(GLFWwindow* window, Config* config)
	: m_window(window), m_config(config)
{
	// Ensure our OpenGL configurations will affect the correct context.
	glfwMakeContextCurrent(window);

	// Move the starting position of the entity.
	entity.SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));

	// Call this during init as the resize callback is not called on startup.
	CalcViewport(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

void Render::Draw() const {
	// Only draw if desired and if a model has been compiled.
	if (m_config->drawPreview && entity.HasModel()) {
		glm::mat4 mvp(1.0f);
		camera.ApplyMatrix(mvp, GetAspectRatio());
		entity.Draw(mvp);
	}
}

void Render::UpdateWireframe() const {
	if (m_config->drawWireframe) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE); // Render back side of wireframe faces.
	glPolygonMode(GL_FRONT_AND_BACK, m_config->drawWireframe ? GL_LINE : GL_FILL); // Toggle line rendering.
}

void Render::CalcViewport(int width, int height) {
	// Place the renderer viewport to the right of the sidepanel and below the menu bar.
	// The render calculations need access to this data also.
	m_viewportWidth = width - GUI_SIDEPANEL_WIDTH;
	m_viewportHeight = height - GUI_MENUBAR_HEIGHT;
	m_aspectRatio = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);

	glViewport(GUI_SIDEPANEL_WIDTH, 0, m_viewportWidth, m_viewportHeight);
}

int Render::GetViewportWidth() const {
	return m_viewportWidth;
}

int Render::GetViewportHeight() const {
	return m_viewportHeight;
}

float Render::GetAspectRatio() const {
	return m_aspectRatio;
}
