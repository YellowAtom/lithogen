#version 460 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

uniform mat4 mvp = mat4(1.0f);

out vec3 frag_color;

void main() {
	frag_color = in_color;
    gl_Position = mvp * vec4(in_position, 1.0f);
}