#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "base.h"

#include "io/io.h"
#include "gfx/gfx.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "render/render.h"

int main(void) {
    gfx_init(GFX_BACKEND_GL);

    io_init();
    monitors_detect();
    io_ctx.window = window_new(io_ctx.active_monitor, 1600, 900, "hello there");

    render_init();

    gfx_viewport(0, 0, 1600, 900);

    i32 x, y;
    void* image_data = stbi_load("res/test.png", &x, &y, NULL, 3);

    texture_t texture = texture_new((texture_info_t) {
        .width = x,
        .height = y,
        .format = TEXTURE_FORMAT_RGB8,
        .data = range_new(image_data, 0),
    });

    sampler_t sampler = sampler_new((sampler_info_t) {
        .wrap = TEXTURE_WRAP_REPEAT,
        .filter = TEXTURE_FILTER_NEAREST,
    });

    f32 z = 0.0f;
    while(!window_closing(game_window)) {
        input_start_frame(&io_ctx);

        z += 0.1f;

        render_push_draw_call((draw_call_t) {
            .position = v3f_new(-0.4f, 0.0f, 0.0f),
            .rotation = v3f_new(0.0f, 0.0f, z),
            .scale = v3f_new(0.1f, 0.1f, 1.0f),
            .colour = v4f_new(0.0f, 0.8f, 0.3f, 1.0f),
        });

        render_push_draw_call((draw_call_t) {
            .position = v3f_new(0.4f, 0.0f, 0.0f),
            .rotation = v3f_new(0.0f, 0.0f, z * 2.0f),
            .scale = v3f_new(0.5f, 0.1f, 1.0f),
            .colour = v4f_new(1.0f, 0.8f, 0.3f, 1.0f),
        });

        render_dispatch();

        glfwSwapBuffers(game_window->glfw_window);
        input_end_frame(&io_ctx);
    }

    render_terminate();
    gfx_terminate();
    io_terminate();
    return 0;
}
