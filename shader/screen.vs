#version 430 core

layout (location = 0) in vec2 vs_position;
layout (location = 1) in vec2 vs_uvs;

out vec2 fs_uvs;
out vec2 fs_pos;

void main(void) {
    gl_Position = vec4(vs_position * 2.0, 0.0, 1.0);
    fs_uvs = vs_uvs;
    fs_pos = vs_position;
}
