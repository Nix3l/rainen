#version 430 core

layout (location = 0) in vec2 vs_position;
layout (location = 1) in vec2 vs_uvs;

uniform int z_layer;
uniform mat4 model_mat;

out vec2 fs_uvs;

void main() {
    gl_Position = model_mat * vec4(vs_position, z_layer, 1.0);
    fs_uvs = vs_uvs;
}
