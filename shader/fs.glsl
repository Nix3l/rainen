#version 330 core

in vec2 fs_uvs;

uniform vec4 color;

uniform sampler2D tex;

out vec4 out_color;

void main(void) {
    out_color = vec4(texture(tex, vec2(fs_uvs.x, 1.0 - fs_uvs.y)).rrr, 1.0) + color;
    // out_color = vec4(fs_uvs, 0.0, 1.0);
}
