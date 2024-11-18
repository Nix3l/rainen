#version 330 core

layout (location = 0) in vec3 vs_position;

uniform mat4 projection_view;
uniform mat4 transformation;

void main(void) {
    gl_Position = projection_view * transformation * vec4(vs_position, 1.0);
}
