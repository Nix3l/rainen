#version 430 core

in vec2 fs_uvs;

uniform vec4 col;

out vec4 out_col;

void main() {
    vec2 uvs = fs_uvs * vec2(2.0) - vec2(1.0);
    if(length(uvs) > 1) discard;
    out_col = col;
}
