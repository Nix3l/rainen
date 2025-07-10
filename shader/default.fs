#version 430 core

out vec4 out_col;

uniform float time;

void main(void) {
    out_col = vec4(abs(sin(time)), abs(cos(time)), abs(tan(time)), 1.0);
}
