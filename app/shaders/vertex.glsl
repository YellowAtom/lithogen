#version 460 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 frag_color;

void main() {
	frag_color = in_color;
    gl_Position = vec4(in_position, 1.0f);
}
