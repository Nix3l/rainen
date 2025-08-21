#version 430 core

in vec2 fs_uvs;

uniform vec4 col;
uniform sampler2D tex;
uniform int use_tex;

out vec4 out_col;

void main(void) {
    if(use_tex > 0) out_col = texture(tex, fs_uvs);
    else out_col = vec4(col.rgb, 1.0);
}
