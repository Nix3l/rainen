#version 330 core

layout (location = 0) in vec2 vs_position;
layout (location = 1) in vec2 vs_uvs;

uniform int z_layer;

uniform mat4 projection_view;
uniform mat4 transform;

out vec2 fs_uvs;

void main(void) {
    gl_Position = projection_view * transform * vec4(vs_position, 0.0, 1.0);

    fs_uvs = vs_uvs;
}
