#version 430 core

in vec2 fs_uvs;

uniform vec4 col;

out vec4 out_col;

void main(void) {
    out_col = vec4(col.rgb, 1.0);
}
