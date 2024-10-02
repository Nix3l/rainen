#version 330 core

in vec2 fs_uvs;

uniform vec4 color;

uniform sampler2D tex;

out vec4 out_color;

void main(void) {
    out_color = texture(tex, fs_uvs) + color;
}
