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

    arena_t shader_code_arena = arena_alloc_new(4096, EXPAND_TYPE_IMMUTABLE);
    range_t vertex_src = platform_load_file(&shader_code_arena, "shader/default.vs");
    range_t fragment_src = platform_load_file(&shader_code_arena, "shader/default.fs");

    vshader_t shader = shader_new((shader_info_t) {
        .name = "dummy",
        .pretty_name = "Dummy",
        .attribs = {
            { .name = "vs_position" },
        },
        .uniforms = {
            { .name = "col", .type = UNIFORM_TYPE_v2f, },
            { .name = "time", .type = UNIFORM_TYPE_f32, },
        },
        .vertex_src = vertex_src,
        .fragment_src = fragment_src,
    });

    f32 vertices[] = {
        0.0f, 0.5f,
        -0.5f, 0.0f,
        0.5f, 0.0f
    };

    vmesh_t mesh = mesh_new((mesh_info_t) {
        .format = MESH_FORMAT_X2,
        .index_type = MESH_INDEX_NONE,
        .attributes = {
            mesh_attribute(vertices, sizeof(vertices), 2),
        },
        .vertex_count = 3,
    });

    while(!window_closing(game_window)) {
        input_start_frame(&io_ctx);

        // do ALL the things
        gfx_activate_pipeline((render_pipeline_t) {
            .shader = shader,
        });

        gfx_supply_bindings((render_bindings_t) {
            .mesh = mesh
        });

        struct {
            f32 time;
        } uniforms = {
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
