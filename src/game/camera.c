#include "camera.h"
#include "game.h"
#include "render/render.h"
#include "io/io.h"

void camera_attach(camera_t* cam, draw_pass_t* pass) {
    v3f pos = v3f_new(cam->transform.position.x, cam->transform.position.y, cam->transform.z);
    v3f rot = v3f_new(0.0f, 0.0f, cam->transform.rotation);
    f32 w = cam->w > 0.0f ? cam->w : io_ctx.window.width;
    f32 h = cam->h > 0.0f ? cam->h : io_ctx.window.height;
    w *= cam->pixel_scale;
    h *= cam->pixel_scale;

    pass->state.anchor.position = pos;
    pass->state.anchor.rotation = rot;
    pass->state.projection.w = w;
    pass->state.projection.h = h;
    pass->state.projection.near = cam->near;
    pass->state.projection.far = cam->far;
}
