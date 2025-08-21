#include "editor.h"
#include "base_macros.h"
#include "base_types.h"
#include "game/camera.h"
#include "imgui/imgui_manager.h"
#include "io/io.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "render/render.h"
#include "util/util.h"
#include "util/math_util.h"
#include "gfx/gfx.h"

// temporary because my cmp keeps auto including this stupid file and its causing errors
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

editor_ctx_t editor_ctx = {0};

static void construct_uniforms(void* out, draw_call_t* call) {
    draw_pass_cache_t cache = render_ctx.active_pass.cache;

    struct {
        mat4 projViewModel;
        vec4 col;
        i32 tex;
        i32 use_tex;
    } uniforms;

    mat4s modelViewProj = glms_mat4_mul(cache.projView, model_matrix_new(call->position, call->rotation, call->scale));
    glm_mat4_copy(modelViewProj.raw, uniforms.projViewModel);

    memcpy(uniforms.col, call->colour.raw, sizeof(vec4));
    uniforms.tex = 0;
    uniforms.use_tex = call->sampler.texture.id != GFX_INVALID_ID ? 1 : 0;

    memcpy(out, &uniforms, sizeof(uniforms));
}

// STATE
void editor_init() {
    // leak ALL the memory
    arena_t shader_code_arena = arena_alloc_new(4096, EXPAND_TYPE_IMMUTABLE);
    range_t vertex_src = platform_load_file(&shader_code_arena, "shader/editor.vs");
    range_t fragment_src = platform_load_file(&shader_code_arena, "shader/editor.fs");
    shader_t shader = shader_new((shader_info_t) {
        .name = "ed",
        .pretty_name = "editor shader",
        .attribs = {
            { .name = "vs_position" },
            { .name = "vs_uvs" },
        },
        .uniforms = {
            { .name = "projViewModel", .type = UNIFORM_TYPE_mat4, },
            { .name = "col", .type = UNIFORM_TYPE_v4f, },
            { .name = "tex", .type = UNIFORM_TYPE_i32, },
            { .name = "use_tex", .type = UNIFORM_TYPE_i32, },
        },
        .vertex_src = vertex_src,
        .fragment_src = fragment_src,
    });

    renderer_t renderer = {
        .label = "editor renderer",
        .pass = {
            .label = "pass1",
            .type = DRAW_PASS_RENDER,
            .pipeline = {
                .clear = {
                    .depth = true,
                    .colour = true,
                    .clear_col = v4f_new(0.0f, 0.0f, 0.0f, 1.0f),
                },
                .cull = { .enable = true, },
                .depth = { .enable = true, },
                .shader = shader,
            },
            .state = {
                .anchor = { .enable = true, .position = v3f_new(0.0f, 0.0f, 100.0f), },
                .projection = {
                    .type = PROJECTION_ORTHO,
                    .fov = RADIANS(80.0f),
                    .aspect_ratio = 16.0f/9.0f,
                    .w = 1600.0f,
                    .h = 900.0f,
                    .near = 0.001f,
                    .far = 1000.0f,
                },
            },
        },
        .batch = vector_alloc_new(RENDER_MAX_CALLS, sizeof(draw_call_t)),
        .construct_uniforms = construct_uniforms,
    };

    camera_t cam = (camera_t) {
        .transform = {
            .z = 100,
        },
        .near = 0.01f,
        .far = 200.0f,
        .pixel_scale = 1.0f,
    };

    editor_ctx = (editor_ctx_t) {
        .open = true,

        .cam = cam,
        .renderer = renderer,

        .editor = {
            .open = true,
        },

        .resviewer = {
            .open = true,
            .selected_texture = 0,
            .texture = { 0 },
        },
    };
}

void editor_terminate() {
    // do stuff
}

void editor_set_open(bool open) {
    editor_ctx.open = open;
}

void editor_toggle() {
    editor_ctx.open = !editor_ctx.open;
}

bool editor_is_open() {
    return editor_ctx.open;
}

// CAMERA
static void editor_camera_update() {
    camera_t* cam = &editor_ctx.cam;

    f32 scroll = input_get_scroll();
    cam->pixel_scale -= scroll * 0.066f;

    v2f move = input_mouse_move_absolute();
    if(input_button_down(BUTTON_LEFT)) {
        cam->transform.position.x -= move.x;
        cam->transform.position.y += move.y;
    }
}

// EDITOR WINDOWS
static void editor_window_main() {
    if(!igBegin("editor", &editor_ctx.editor.open, ImGuiWindowFlags_None)) {
        igEnd();
        return;
    }

    if(igCollapsingHeader_BoolPtr("camera", NULL, ImGuiTreeNodeFlags_None)) {
        if(igSmallButton("RESET CAMERA")) {
            editor_ctx.cam.transform.position = v2f_ZERO;
            editor_ctx.cam.transform.rotation = 0.0f;
            editor_ctx.cam.pixel_scale = 1.0f;
        }

        igDragFloat2("position", editor_ctx.cam.transform.position.raw, 0.1f, -MAX_f32, MAX_f32, "%.2f", ImGuiSliderFlags_None);
        igDragFloat("zoom", &editor_ctx.cam.pixel_scale, 0.1f, 0.1f, 200.0f, "%.1f", ImGuiSliderFlags_None);
    }

    igEnd();
}

static void editor_window_resviewer() {
    if(!igBegin("resviewer", &editor_ctx.resviewer.open, ImGuiWindowFlags_None)) {
        igEnd();
        return;
    }

    igSetNextItemOpen(true, ImGuiCond_Once);
    if(igCollapsingHeader_BoolPtr("textures", NULL, ImGuiTreeNodeFlags_None)) {
        igBeginChild_Str("texturelist", imv2f(120.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
        pool_iter_t iter = { .absolute_index = 1, };
        while(pool_iter(&gfx_ctx.texture_pool->res_pool, &iter)) {
            char label[32];
            snprintf(label, 32, "texture %u", iter.absolute_index);
            if(igSelectable_Bool(label, iter.absolute_index == editor_ctx.resviewer.selected_texture, ImGuiSelectableFlags_SelectOnClick, imv2f_ZERO)) {
                if(editor_ctx.resviewer.selected_texture == iter.absolute_index) {
                    editor_ctx.resviewer.selected_texture = 0;
                    editor_ctx.resviewer.texture = (texture_t) { 0 };
                } else {
                    editor_ctx.resviewer.selected_texture = iter.absolute_index;
                    editor_ctx.resviewer.texture = (texture_t) { iter.handle };
                }
            }
        }
        igEndChild();

        igSameLine(120.0f, 12.0f);

        igBeginChild_Str("textureviewer", imv2f_ZERO, ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

        if(editor_ctx.resviewer.selected_texture > 0) {
            ImVec2 region;
            igGetContentRegionAvail(&region);

            texture_data_t* texture_data = texture_get_data(editor_ctx.resviewer.texture);
            f32 aspect_ratio = (f32) texture_data->height / (f32) texture_data->width;
            f32 w = texture_data->width > region.x ? region.x : texture_data->width;
            f32 h = w * aspect_ratio;
            imgui_texture_image(editor_ctx.resviewer.texture, v2f_new(w, h));
            igText("width [%u] height [%u]", texture_data->width, texture_data->height);

            // lol
            static const char* format_names[] = {
                STRINGIFY(TEXTURE_FORMAT_UNDEFINED),
                STRINGIFY(TEXTURE_FORMAT_R8),
                STRINGIFY(TEXTURE_FORMAT_R8I),
                STRINGIFY(TEXTURE_FORMAT_R8UI),
                STRINGIFY(TEXTURE_FORMAT_R16),
                STRINGIFY(TEXTURE_FORMAT_R16F),
                STRINGIFY(TEXTURE_FORMAT_R16I),
                STRINGIFY(TEXTURE_FORMAT_R16UI),
                STRINGIFY(TEXTURE_FORMAT_R32F),
                STRINGIFY(TEXTURE_FORMAT_R32I),
                STRINGIFY(TEXTURE_FORMAT_R32UI),
                STRINGIFY(TEXTURE_FORMAT_RG8),
                STRINGIFY(TEXTURE_FORMAT_RG8I),
                STRINGIFY(TEXTURE_FORMAT_RG8UI),
                STRINGIFY(TEXTURE_FORMAT_RG16),
                STRINGIFY(TEXTURE_FORMAT_RG16F),
                STRINGIFY(TEXTURE_FORMAT_RG16I),
                STRINGIFY(TEXTURE_FORMAT_RG16UI),
                STRINGIFY(TEXTURE_FORMAT_RG32F),
                STRINGIFY(TEXTURE_FORMAT_RG32I),
                STRINGIFY(TEXTURE_FORMAT_RG32UI),
                STRINGIFY(TEXTURE_FORMAT_RGB8),
                STRINGIFY(TEXTURE_FORMAT_RGB8I),
                STRINGIFY(TEXTURE_FORMAT_RGB8UI),
                STRINGIFY(TEXTURE_FORMAT_RGB16F),
                STRINGIFY(TEXTURE_FORMAT_RGB16I),
                STRINGIFY(TEXTURE_FORMAT_RGB16UI),
                STRINGIFY(TEXTURE_FORMAT_RGB32F),
                STRINGIFY(TEXTURE_FORMAT_RGB32I),
                STRINGIFY(TEXTURE_FORMAT_RGB32UI),
                STRINGIFY(TEXTURE_FORMAT_RGBA8),
                STRINGIFY(TEXTURE_FORMAT_RGBA8I),
                STRINGIFY(TEXTURE_FORMAT_RGBA8UI),
                STRINGIFY(TEXTURE_FORMAT_RGBA16F),
                STRINGIFY(TEXTURE_FORMAT_RGBA16I),
                STRINGIFY(TEXTURE_FORMAT_RGBA16UI),
                STRINGIFY(TEXTURE_FORMAT_RGBA32F),
                STRINGIFY(TEXTURE_FORMAT_RGBA32I),
                STRINGIFY(TEXTURE_FORMAT_RGBA32UI),
                STRINGIFY(TEXTURE_FORMAT_DEPTH),
                STRINGIFY(TEXTURE_FORMAT_DEPTH_STENCIL),
            };

            igText("format [%s]", format_names[texture_data->format]);
            igText("mipmaps [%u]", texture_data->mipmaps);
        } else {
            igText("no texture selected");
        }

        igEndChild();
    }

    igEnd();
}

void editor_update() {
    render_push_draw_call(&editor_ctx.renderer, (draw_call_t) {
        .scale = v3f_new(200.0f, 200.0f, 1.0f),
        .colour = v4f_new(0.73f, 0.1f, 0.35f, 1.0f),
    });

    editor_window_main();
    editor_window_resviewer();

    editor_camera_update();
}

void editor_render() {
    camera_attach(&editor_ctx.cam, &editor_ctx.renderer.pass);
    render_dispatch(&editor_ctx.renderer);
}
