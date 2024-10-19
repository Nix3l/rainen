#version 330 core

in vec2 fs_uvs;

uniform vec4 color;

uniform sampler2D tex;

out vec4 out_color;

void main(void) {
    out_color = vec4(texture(tex, fs_uvs).r) * color;
    // out_color = vec4(1.0);
}
