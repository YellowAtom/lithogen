#version 460 core

layout (location = 0) in vec3 flag_color;
out vec4 out_color;

void main() {
	out_color = vec4(flag_color, 1.0f);
}
