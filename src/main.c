#include "base_macros.h"
#include "game/entity.h"
#include "physics/bounds.h"
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
#include "physics/physics.h"
#include "tools/editor.h"

int main(void) {
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

    v2f size2 = v2f_new(600, 100);
    collider_t coll2 = collider_new((collider_info_t) {
        .tags = COLLIDER_TAGS_STATIC,
        .pos = v2f_new(0, -250),
        .bounds = {
            .type = COLLIDER_SHAPE_AABB,
            .box = aabb_new_rect(v2f_ZERO, size2),
        },
        .restitution = 1.0f,
    });

    v2f size1 = v2f_new(20, 20);
    collider_t coll1 = collider_new((collider_info_t) {
        .tags = COLLIDER_TAGS_NO_GRAV,
        .pos = v2f_ZERO,
        .bounds = {
            .type = COLLIDER_SHAPE_AABB,
            .box = aabb_new_rect(v2f_ZERO, size1),
        },
        .mass = 10.0f,
        .restitution = 0.3f,
    });

    v2f size3 = v2f_new(35, 35);
    collider_t coll3 = collider_new((collider_info_t) {
        .pos = v2f_new(100, 200),
        .bounds = {
            .type = COLLIDER_SHAPE_AABB,
            .box = aabb_new_rect(v2f_ZERO, size3),
        },
        .mass = 4.0f,
        .restitution = 1.0f,
    });

    v2f size4 = v2f_new(100, 35);
    collider_t coll4 = collider_new((collider_info_t) {
        .pos = v2f_new(100, 300),
        .bounds = {
            .type = COLLIDER_SHAPE_AABB,
            .box = aabb_new_rect(v2f_ZERO, size4),
        },
        .mass = 1.0f,
        .restitution = 1.0f,
    });

    entity_new((entity_info_t) {
        .tags = ENT_TAGS_RENDER | ENT_TAGS_PHYSICS,
        .material = {
            .colour = v4f_new(0.82f, 1.0f, 0.05f, 1.0f),
        },
        .transform = {
            .size = size1,
        },
        .collider = coll1,
    });

    entity_new((entity_info_t) {
        .tags = ENT_TAGS_RENDER | ENT_TAGS_PHYSICS,
        .material = {
            .colour = v4f_new(1.0f, 1.0f, 1.0f, 1.0f),
        },
        .transform = {
            .size = size2,
        },
        .collider = coll2,
    });

    entity_new((entity_info_t) {
        .tags = ENT_TAGS_RENDER | ENT_TAGS_PHYSICS,
        .material = {
            .colour = v4f_new(0.0f, 1.0f, 1.0f, 1.0f),
        },
        .transform = {
            .size = size3,
        },
        .collider = coll3,
    });

    entity_new((entity_info_t) {
        .tags = ENT_TAGS_RENDER | ENT_TAGS_PHYSICS,
        .material = {
            .colour = v4f_new(0.2f, 0.93f, 0.14f, 1.0f),
        },
        .transform = {
            .size = size4,
        },
        .collider = coll4,
    });

    while(!window_closing()) {
        input_start_frame();
        imgui_start_frame();

        if(input_key_pressed(KEY_F10)) editor_toggle();

        // igShowDemoWindow(NULL);

        if(!editor_is_open()) {
            const f32 speed = 500.0f;
            if(input_key_down(KEY_UP))    collider_apply_force(coll1, v2f_new(0.0f, speed));
            if(input_key_down(KEY_RIGHT)) collider_apply_force(coll1, v2f_new(speed, 0.0f));
            if(input_key_down(KEY_DOWN))  collider_apply_force(coll1, v2f_new(0.0f, -speed));
            if(input_key_down(KEY_LEFT))  collider_apply_force(coll1, v2f_new(-speed, 0.0f));
            physics_update();
            game_update();
            game_render();
        } else {
            editor_update();
        }

        imgui_show();
        window_swap_buffers();
    }

    editor_terminate();
    game_terminate();
    entity_terminate();
    physics_terminate();
    imgui_terminate();
    render_terminate();
    gfx_terminate();
    window_destroy();
    io_terminate();
    return 0;
}
