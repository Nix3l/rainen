#version 430 core

uniform vec2 start;
uniform vec2 end;

uniform float outline_width;

uniform vec4 outline_col;
uniform vec4 inside_col;

out vec4 out_col;

void main(void) {
    out_col = vec4(0.0);

    vec2 pos = gl_FragCoord.xy;

    bool is_outline = false;
    if(abs(pos.x - start.x) <= outline_width) is_outline = true;
    if(abs(pos.y - start.y) <= outline_width) is_outline = true;
    if(abs(pos.x - end.x) <= outline_width) is_outline = true;
    if(abs(pos.y - end.y) <= outline_width) is_outline = true;

    bool is_inside = true;
    if(pos.x < start.x || pos.x > end.x) is_inside = false;
    if(pos.y < start.y || pos.y > end.y) is_inside = false;

    if(is_outline && is_inside) out_col = outline_col;
    else if(is_inside) out_col = inside_col;
}
