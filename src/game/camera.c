#include "camera.h"
#include "game.h"
#include "render/render.h"
#include "io/io.h"

static void draw_pass_attach(draw_pass_t* pass, v3f pos, v3f rot, f32 w, f32 h, f32 near, f32 far) {
    pass->state.anchor.position = pos;
    pass->state.anchor.rotation = rot;
    pass->state.projection.w = w;
    pass->state.projection.h = h;
    pass->state.projection.near = near;
    pass->state.projection.far = far;
}

void camera_attach() {
    camera_t cam = game_ctx.camera;

    v3f pos = v3f_new(cam.transform.position.x, cam.transform.position.y, cam.transform.z);
    v3f rot = v3f_new(0.0f, 0.0f, cam.transform.rotation);
    f32 w = (f32) io_ctx.window.width * cam.pixel_scale;
    f32 h = (f32) io_ctx.window.height * cam.pixel_scale;

    draw_pass_attach(&render_ctx.renderer.pass, pos, rot, w, h, cam.near, cam.far);
}

void camera_update() {
    camera_t* cam = &game_ctx.camera;

    cam->pixel_scale -= input_get_scroll() * 0.1f;
    if(cam->pixel_scale < 0.1f) cam->pixel_scale = 0.1f;

    if(input_button_down(BUTTON_LEFT)) {
        v2f move = v2f_scale(input_mouse_move_absolute(), cam->pixel_scale);
        move.x = -move.x;
        cam->transform.position = v2f_add(cam->transform.position, move);
    }
}
