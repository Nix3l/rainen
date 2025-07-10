#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "base.h"

#include "io/io.h"
#include "gfx/gfx.h"
#include "memory/memory.h"
#include "platform/platform.h"

int main(void) {
    gfx_init(GFX_BACKEND_GL);
    io_init();

    monitors_detect();
    io_ctx.window = window_new(io_ctx.active_monitor, 1600, 900, "hello there");
    glViewport(0, 0, 1600, 900);

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

    arena_t shader_code_arena = arena_alloc_new(4096, EXPAND_TYPE_IMMUTABLE);
    range_t vertex_src = platform_load_file(&shader_code_arena, "shader/default.vs");
    range_t fragment_src = platform_load_file(&shader_code_arena, "shader/default.fs");

    shader_t shader = shader_new((shader_info_t) {
        .name = "dummy",
        .pretty_name = "Dummy",
        .attribs = {
            { .name = "vs_position" },
        },
        .uniforms = {
            { .name = "tex", .type = UNIFORM_TYPE_i32, },
            { .name = "time", .type = UNIFORM_TYPE_f32, },
        },
        .vertex_src = vertex_src,
        .fragment_src = fragment_src,
    });

    f32 vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    f32 uvs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };

    u32 indices[] = {
        0, 1, 2,
        2, 1, 3,
    };

    mesh_t mesh = mesh_new((mesh_info_t) {
        .format = MESH_FORMAT_X2T2,
        .index_type = MESH_INDEX_32b,
        .indices = range_new(indices, sizeof(indices)),
        .attributes = {
            mesh_attribute(vertices, sizeof(vertices), 2),
            mesh_attribute(uvs, sizeof(uvs), 2),
        },
        .count = 6,
    });

    while(!window_closing(game_window)) {
        input_start_frame(&io_ctx);

        // do ALL the things
        gfx_activate_pipeline((render_pipeline_t) {
            .shader = shader,
        });

        gfx_supply_bindings((render_bindings_t) {
            .mesh = mesh,
            .texture_samplers = {
                { .texture = texture, .sampler = sampler },
            },
        });

        struct {
            i32 tex;
            f32 time;
        } uniforms = {
            .tex = 0,
            .time = glfwGetTime(),
        };

        shader_update_uniforms(shader, range_new(&uniforms, sizeof(uniforms)));

        gfx_draw();

        glfwSwapBuffers(game_window->glfw_window);
        input_end_frame(&io_ctx);
    }

    io_terminate();
    return 0;
}
