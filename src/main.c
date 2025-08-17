#include "game/entity.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "base.h"

#include "util/util.h"

#include "io/io.h"
#include "gfx/gfx.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "render/render.h"
#include "game/game.h"

int main(void) {
    gfx_init(GFX_BACKEND_GL);
    io_init();
    monitors_detect();
    io_ctx.window = window_new(io_ctx.active_monitor, 1600, 900, "hello there");
    render_init();
    game_init();

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

    for(u32 i = 0; i < 500; i ++) {
        entity_new((entity_info_t) {
            .tags = ENT_TAGS_RENDER,
            .material = {
                .colour = v4f_new(0.1f, 0.8f, 0.5f, 1.0f),
            },
            .transform = {
                .position = v2f_new(-2.0f, 1.0f),
                .rotation = 45.0f,
                .size = v2f_new(500.0f, 700.0f),
                .z = 0,
            },
        });
    }

    entity_t ent = entity_new((entity_info_t) {
        .tags = ENT_TAGS_RENDER,
        .material = {
            .colour = v4f_new(0.8f, 0.3f, 0.5f, 1.0f),
        },
        .transform = {
            .position = v2f_new(0.0f, 0.0f),
            .size = v2f_new(500.0f, 500.0f),
            .z = 0,
        },
    });

    while(!window_closing(game_window)) {
        input_start_frame(&io_ctx);

        if(input_key_pressed(GLFW_KEY_P)) entity_mark_dirty(ent);

        entity_update();

        render_dispatch();
        glfwSwapBuffers(game_window->glfw_window);
        input_end_frame(&io_ctx);
    }

    game_terminate();
    render_terminate();
    gfx_terminate();
    io_terminate();
    return 0;
}
