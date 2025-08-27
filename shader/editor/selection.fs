#version 430 core

in vec2 fs_uvs;

uniform vec2 start;
uniform vec2 end;

uniform vec4 outline_col;
uniform vec4 inside_col;

out vec4 out_col;

const float threshold = 0.001;

void main(void) {
    out_col = vec4(0.0);

    bool is_outline = false;
    if(abs(fs_uvs.x - start.x) <= threshold) is_outline = true;
    if(abs(fs_uvs.y - start.y) <= threshold) is_outline = true;
    if(abs(fs_uvs.x - end.x) <= threshold) is_outline = true;
    if(abs(fs_uvs.y - end.y) <= threshold) is_outline = true;

    bool is_inside = true;
    if(fs_uvs.x < start.x || fs_uvs.x > end.x) is_inside = false;
    if(fs_uvs.y < start.y || fs_uvs.y > end.y) is_inside = false;

    if(is_outline && is_inside) out_col = outline_col;
    else if(is_inside) out_col = inside_col;
}
