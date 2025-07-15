#version 430 core

in vec2 fs_uvs;
in vec2 fs_pos;

out vec4 out_col;

uniform sampler2D tex;

void main(void) {
    // out_col = texture(tex, fs_uvs * 0.3);
    out_col = vec4(fs_pos.rg, 0.0, 1.0);
}
