#version 430 core

in vec2 fs_uvs;

out vec4 out_col;

uniform sampler2D tex;
uniform float time;

void main(void) {
    out_col = vec4(texture(tex, fs_uvs * 0.3).rrr * abs(sin(time)), 1.0);
}
