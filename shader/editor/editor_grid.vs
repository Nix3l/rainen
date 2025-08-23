#version 430 core

layout (location = 0) in vec2 vs_position;
layout (location = 1) in vec2 vs_uvs;

void main(void) {
    gl_Position = vec4(vs_position, 0.0, 1.0);
}
