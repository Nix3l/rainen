#version 430 core

layout (location = 0) in vec2 vs_position;
layout (location = 1) in vec2 vs_uvs;

uniform mat4 model_mat;

out vec2 fs_uvs;

void main(void) {
    gl_Position = model_mat * vec4(vs_position, 0.0, 1.0);
    fs_uvs = vs_uvs;
}
