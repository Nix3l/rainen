#version 330 core

layout (location = 0) in vec3 vs_position;
layout (location = 1) in vec2 vs_uvs;

uniform mat4 projection_view;

out vec2 fs_uvs;

void main(void) {
    gl_Position = projection_view * vec4(vs_position, 1.0);

    fs_uvs = vs_uvs;
}
