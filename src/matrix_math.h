#pragma once

#include <glm/glm.hpp>

void CreateProjMatrix(glm::mat4& mvp, float fov, float nearZ, float farZ, float g_aspectRatio);
void CreateViewMatrix(glm::mat4& mvp, const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
void CreateModelMatrix(glm::mat4& mvp, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
