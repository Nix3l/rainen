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
#include "imgui/imgui_manager.h"
#include "tools/editor.h"

int main(void) {
    gfx_init(GFX_BACKEND_GL);
    io_init();
    monitors_detect();
    window_new(1600, 900, "WINDOW");
    imgui_init();
    render_init();
    game_init();
    editor_init();

    gfx_viewport(0, 0, 1600, 900);

    i32 x, y;
    void* image_data = stbi_load("res/kingterry.jpg", &x, &y, NULL, 3);

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

    while(!window_closing()) {
        input_start_frame();
        imgui_start_frame();

        if(input_key_pressed(KEY_F12)) editor_toggle();

        igShowDemoWindow(NULL);

        if(!editor_is_open()) game_update();
        else editor_update();

        render_dispatch();
        imgui_show();
        window_swap_buffers();
    }

    editor_terminate();
    game_terminate();
    imgui_terminate();
    render_terminate();
    gfx_terminate();
    window_destroy();
    io_terminate();
    return 0;
}
