#version 430 core

uniform float tw;
uniform float th;
uniform float scale;
uniform float screen_width;
uniform float screen_height;
uniform vec2 cam_offset;
uniform vec4 bg_col;
uniform vec4 grid_col;

out vec4 out_col;

void main(void) {
    vec2 off = cam_offset / scale;
    vec2 grid = vec2(abs(gl_FragCoord.x + off.x - screen_width / 2.0), abs(gl_FragCoord.y + off.y - screen_height / 2.0));
    grid.x = mod(grid.x, tw / scale);
    grid.y = mod(grid.y, th / scale);

    out_col = bg_col;
    if(grid.x <= 1.0 || grid.y <= 1.0) out_col = grid_col;
}
