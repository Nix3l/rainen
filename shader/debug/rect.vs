#version 430 core

layout (location = 0) in vec2 vs_position;

uniform int z_layer;
uniform mat4 model_mat;

void main() {
    gl_Position = model_mat * vec4(vs_position, z_layer, 1.0);
}
