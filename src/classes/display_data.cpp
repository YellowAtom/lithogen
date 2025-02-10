
#include <glad/gl.h>
#include "display_data.h"
#include "../constants.h"

void DisplayData::CalcViewport(GLFWwindow* window, int width, int height) {
	// Place the renderer viewport to the right of the sidepanel and below the menu bar.
	// The render calculations need access to this data also.
	m_viewportWidth = width - GUI_SIDEPANEL_WIDTH;
	m_viewportHeight = height - GUI_MENUBAR_HEIGHT;
	m_aspectRatio = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);

	glViewport(GUI_SIDEPANEL_WIDTH, 0, m_viewportWidth, m_viewportHeight);
}

int DisplayData::GetViewportWidth() const {
	return m_viewportWidth;
}

int DisplayData::GetViewportHeight() const {
	return m_viewportHeight;
}

float DisplayData::GetAspectRatio() const {
	return m_aspectRatio;
}
