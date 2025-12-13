#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "base.h"

#include "util/util.h"

#include "rations/rations.h"
#include "stats/stats.h"
#include "io/io.h"
#include "gfx/gfx.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "render/render.h"
#include "game/game.h"
#include "imgui/imgui_manager.h"
#include "physics/physics.h"
#include "tools/editor.h"
#include "tools/debug_render.h"
#include "event/events.h"

// TODO(nix3l):
//  => update gfx.c (fix cache, add way to do wireframe, more docs in header)
//  => update render.c
//  => redo cameras

int main(void) {
    rations_divide();
    events_init();
    stats_init();
    gfx_init(GFX_BACKEND_GL);
    io_init();
    monitors_detect();
    window_new(1600, 900, "WINDOW");
    imgui_init();
    render_init();
    physics_init();
    entity_init();
    game_init();

    editor_init();
    debug_render_init();

    stbi_set_flip_vertically_on_load(true);
    i32 x, y;
    void* image_data = stbi_load("res/kingterry.jpg", &x, &y, NULL, 3);

    texture_t texture = texture_new((texture_info_t) {
        .width = x,
        .height = y,
        .format = TEXTURE_FORMAT_RGB8,
        .data = range_new(image_data, 0),
        // .filter = TEXTURE_FILTER_LINEAR,
    });

    sampler_t sampler = sampler_new((sampler_info_t) {
        .wrap = TEXTURE_WRAP_REPEAT,
        .filter = TEXTURE_FILTER_NEAREST,
    });

    while(!window_closing()) {
        stats_start_frame();
        input_start_frame();
        imgui_start_frame();

        if(input_key_pressed(KEY_F10)) editor_toggle();

        // igShowDemoWindow(NULL);

        if(!editor_is_open()) {
            physics_update(stats_dt());
            game_update();
            game_render();

            debug_render((debug_render_call_t) {
                .shape = DEBUG_SHAPE_CIRCLE,
                .data.circle = {
                    .centre = v2f_new(80.0f, 120.0f),
                    .radius = 16.0f,
                },
                .color = DEBUG_COLOR_WHITE,
            });

            debug_render_dispatch();
        } else {
            editor_update();
        }

        imgui_show();
        window_swap_buffers();

        render_end_frame();
    }

    debug_render_terminate();
    editor_terminate();

    game_terminate();
    entity_terminate();
    physics_terminate();
    imgui_terminate();
    render_terminate();
    gfx_terminate();
    window_destroy();
    io_terminate();
    events_terminate();

    return 0;
}
