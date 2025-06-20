#version 430 core

layout (location = 0) in vec3 vs_position;
layout (location = 1) in vec2 vs_uvs;
layout (location = 2) in vec2 vs_normals;

void main(void) {
    gl_Position = vec4(vs_position.xyz, 1.0);
}
