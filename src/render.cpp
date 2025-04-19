#include "render.h"
#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include "constants.h"

Render::Render(GLFWwindow* window, Config* config) : m_window(window), m_config(config) {
	// Ensure our OpenGL configurations will affect the correct context.
	glfwMakeContextCurrent(window);

	// Move the starting position of the entity.
	entity.SetPosition(glm::vec3(0.0F, 0.0F, 2.0F));

	// Call this during init as the resize callback is not called on startup.
	CalcViewport(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

void Render::Draw() const {
	// Only draw if desired and if a model has been compiled.
	if (m_config->drawPreview && entity.HasModel()) {
		glm::mat4 mvp(1.0F);
		camera.ApplyMatrix(mvp, GetAspectRatio());
		entity.Draw(mvp);
	}
}

void Render::UpdateWireframe() const {
	// Render back side of wireframe faces.
	if (m_config->drawWireframe) {
		glDisable(GL_CULL_FACE);
	} else {
		glEnable(GL_CULL_FACE);
	}

	// Toggle line rendering.
	glPolygonMode(GL_FRONT_AND_BACK, m_config->drawWireframe ? GL_LINE : GL_FILL);
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