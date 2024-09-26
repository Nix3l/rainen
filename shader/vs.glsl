#version 330 core

layout (location = 0) in vec2 vs_position;
layout (location = 1) in vec2 vs_uvs;

uniform int z_layer;

uniform mat4 projection_view;

void main(void) {
    gl_Position = projection_view * vec4(vs_position, 0.0, 1.0);
}
