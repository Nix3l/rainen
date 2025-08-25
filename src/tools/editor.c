#include "editor.h"
#include "base.h"
#include "base_macros.h"
#include "game/camera.h"
#include "game/room.h"
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

// lol
static const char* format_names[] = {
    STRINGIFY(TEXTURE_FORMAT_UNDEFINED),
    STRINGIFY(R8),
    STRINGIFY(R8I),
    STRINGIFY(R8UI),
    STRINGIFY(R16),
    STRINGIFY(R16F),
    STRINGIFY(R16I),
    STRINGIFY(R16UI),
    STRINGIFY(R32F),
    STRINGIFY(R32I),
    STRINGIFY(R32UI),
    STRINGIFY(RG8),
    STRINGIFY(RG8I),
    STRINGIFY(RG8UI),
    STRINGIFY(RG16),
    STRINGIFY(RG16F),
    STRINGIFY(RG16I),
    STRINGIFY(RG16UI),
    STRINGIFY(RG32F),
    STRINGIFY(RG32I),
    STRINGIFY(RG32UI),
    STRINGIFY(RGB8),
    STRINGIFY(RGB8I),
    STRINGIFY(RGB8UI),
    STRINGIFY(RGB16F),
    STRINGIFY(RGB16I),
    STRINGIFY(RGB16UI),
    STRINGIFY(RGB32F),
    STRINGIFY(RGB32I),
    STRINGIFY(RGB32UI),
    STRINGIFY(RGBA8),
    STRINGIFY(RGBA8I),
    STRINGIFY(RGBA8UI),
    STRINGIFY(RGBA16F),
    STRINGIFY(RGBA16I),
    STRINGIFY(RGBA16UI),
    STRINGIFY(RGBA32F),
    STRINGIFY(RGBA32I),
    STRINGIFY(RGBA32UI),
    STRINGIFY(DEPTH),
    STRINGIFY(DEPTH_STENCIL),
};

static const char* filter_names[] = {
    STRINGIFY(TEXTURE_FILTER_UNDEFINED),
    STRINGIFY(NEAREST),
    STRINGIFY(LINEAR),
};

static const char* wrap_names[] = {
    STRINGIFY(TEXTURE_WRAP_UNDEFINED),
    STRINGIFY(REPEAT),
    STRINGIFY(MIRRORED_REPEAT),
    STRINGIFY(CLAMP_TO_EDGE),
};

static const char* shader_pass_names[] = {
    STRINGIFY(SHADER_PASS_INVALID),
    STRINGIFY(VERTEX),
    STRINGIFY(FRAGMENT),
    STRINGIFY(COMPUTE),
};

static const char* uniform_type_names[] = {
    STRINGIFY(UNIFORM_TYPE_INVALID),
    STRINGIFY(i32),
    STRINGIFY(u32),
    STRINGIFY(f32),
    STRINGIFY(v2f),
    STRINGIFY(v3f),
    STRINGIFY(v4f),
    STRINGIFY(v2i),
    STRINGIFY(v3i),
    STRINGIFY(v4i),
    STRINGIFY(mat4),
};

static void grid_construct_uniforms(void* out, draw_call_t* call) {
    struct  __attribute__((packed)) {
        f32 tw;
        f32 th;
        f32 scale;
        f32 screen_width;
        f32 screen_height;
        vec2 cam_offset;
        vec4 bg_col;
        vec4 grid_col;
    } uniforms;

    uniforms.tw = TILE_WIDTH;
    uniforms.th = TILE_HEIGHT;

    uniforms.screen_width = io_ctx.window.width;
    uniforms.screen_height = io_ctx.window.height;

    uniforms.scale = editor_ctx.cam.pixel_scale;

    v2f offset = editor_ctx.cam.transform.position;
    memcpy(uniforms.cam_offset, offset.raw, sizeof(vec2));

    v4f grid_col = v4f_new(0.11f, 0.18f, 0.15f, 1.0f);
    v4f bg_col   = v4f_new(0.05f, 0.05f, 0.05f, 1.0f);

    memcpy(uniforms.grid_col, grid_col.raw, sizeof(vec4));
    memcpy(uniforms.bg_col, bg_col.raw, sizeof(vec4));

    memcpy(out, &uniforms, sizeof(uniforms));
    UNUSED(call);
}

static void room_construct_uniforms(void* out, draw_call_t* call) {
    draw_pass_cache_t cache = render_ctx.active_group.pass.cache;

    struct  __attribute__((packed)){
        mat4 projViewModel;
        vec4 col;
    } uniforms;

    mat4s modelViewProj = glms_mat4_mul(cache.projView, model_matrix_new(call->position, call->rotation, call->scale));
    glm_mat4_copy(modelViewProj.raw, uniforms.projViewModel);

    memcpy(uniforms.col, call->colour.raw, sizeof(vec4));

    memcpy(out, &uniforms, sizeof(uniforms));
}

// STATE
void editor_init() {
    // leak ALL the memory
    arena_t grid_code_arena = arena_alloc_new(4096, EXPAND_TYPE_IMMUTABLE);
    range_t grid_vs = platform_load_file(&grid_code_arena, "shader/editor/editor_grid.vs");
    range_t grid_fs = platform_load_file(&grid_code_arena, "shader/editor/editor_grid.fs");
    shader_t grid_shader = shader_new((shader_info_t) {
        .pretty_name = "grid shader",
        .attribs = {
            { .name = "vs_position" },
            { .name = "vs_uvs" },
        },
        .uniforms = {
            { .name = "tw", .type = UNIFORM_TYPE_f32, },
            { .name = "th", .type = UNIFORM_TYPE_f32, },
            { .name = "scale", .type = UNIFORM_TYPE_f32, },
            { .name = "screen_width", .type = UNIFORM_TYPE_f32, },
            { .name = "screen_height", .type = UNIFORM_TYPE_f32, },
            { .name = "cam_offset", .type = UNIFORM_TYPE_v2f, },
            { .name = "bg_col", .type = UNIFORM_TYPE_v4f, },
            { .name = "grid_col", .type = UNIFORM_TYPE_v4f, },
        },
        .vertex_src = grid_vs,
        .fragment_src = grid_fs,
    });

    arena_t room_code_arena = arena_alloc_new(4096, EXPAND_TYPE_IMMUTABLE);
    range_t room_vs = platform_load_file(&room_code_arena, "shader/editor/editor_room.vs");
    range_t room_fs = platform_load_file(&room_code_arena, "shader/editor/editor_room.fs");
    shader_t room_shader = shader_new((shader_info_t) {
        .pretty_name = "room shader",
        .attribs = {
            { .name = "vs_position" },
            { .name = "vs_uvs" },
        },
        .uniforms = {
            { .name = "projViewModel", .type = UNIFORM_TYPE_mat4, },
            { .name = "col", .type = UNIFORM_TYPE_v4f, },
        },
        .vertex_src = room_vs,
        .fragment_src = room_fs,
    });

    texture_t col_target = texture_new((texture_info_t) {
        .type = TEXTURE_TYPE_2D,
        .format = TEXTURE_FORMAT_RGBA8,
        .width = io_ctx.window.width,
        .height = io_ctx.window.height,
    });

    texture_t depth_target = texture_new((texture_info_t) {
        .type = TEXTURE_TYPE_2D,
        .format = TEXTURE_FORMAT_DEPTH,
        .width = io_ctx.window.width,
        .height = io_ctx.window.height,
    });

    attachments_t att = attachments_new((attachments_info_t) {
        .colours = {
            [0] = col_target,
        },
        .depth_stencil = depth_target,
    });

    renderer_t renderer = {
        .label = "editor renderer",
        .num_groups = 2,
        .groups = {
            [0] = {
                .pass = {
                    .label = "grid pass",
                    .type = DRAW_PASS_RENDER,
                    .pipeline = {
                        .clear = {
                            .colour = true,
                            .clear_col = v4f_new(0.0f, 0.0f, 0.0f, 1.0f),
                        },
                        .cull = { .enable = true, },
                        .draw_attachments = att,
                        .colour_targets = {
                            [0] = { .enable = true, },
                        },
                        .shader = grid_shader,
                    },
                },
                .batch = vector_alloc_new(4, sizeof(draw_call_t)),
                .construct_uniforms = grid_construct_uniforms,
            },
            [1] = {
                .pass = {
                    .label = "tile pass",
                    .type = DRAW_PASS_RENDER,
                    .pipeline = {
                        .clear = { .depth = true, },
                        .cull = { .enable = true, },
                        .depth = { .enable = true, },
                        .blend = {
                            .enable = true,
                            .src_func = BLEND_FUNC_SRC_ALPHA,
                            .dst_func = BLEND_FUNC_DST_ALPHA,
                        },
                        .draw_attachments = att,
                        .colour_targets = {
                            [0] = { .enable = true, },
                        },
                        .shader = room_shader,
                    },
                    .state = {
                        .anchor = { .enable = true, },
                        .projection = { .type = PROJECTION_ORTHO, },
                    },
                },
                .batch = vector_alloc_new(RENDER_MAX_CALLS, sizeof(draw_call_t)),
                .construct_uniforms = room_construct_uniforms,
            },
        }
    };

    camera_t cam = (camera_t) {
        .transform = { .z = 100, },
        .near = 0.01f,
        .far = 200.0f,
        .pixel_scale = 1.0f,
    };

    room_t room = room_new();

    room_set_tile(&room, (tile_t) {
        .tags = TILE_TAGS_RENDER,
        .x = 4,
        .y = 4,
        .col = v4f_ONE,
    });

    editor_ctx = (editor_ctx_t) {
        .open = true,

        .cam = cam,
        .max_zoom = 1.6f,
        .renderer = renderer,
        .render_texture = col_target,

        .editor = {
            .open = true,
        },

        .resviewer = {
            .open = true,
        },

        .room = room,
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

// EDITOR WINDOWS
static void editor_topbar() {
    if(igBeginMainMenuBar()) {
        if(igBeginMenu("editor", true)) {
            if(igMenuItem_Bool("quit", "F10", false, true))
                editor_ctx.open = false;

            igEndMenu();
        }

        if(igBeginMenu("windows", true)) {
            igMenuItem_BoolPtr("editor", NULL, &editor_ctx.editor.open, true);
            igMenuItem_BoolPtr("resource viewer", NULL, &editor_ctx.resviewer.open, true);
            igEndMenu();
        }

        igEndMainMenuBar();
    }
}

static void editor_dockspace() {
    ImGuiViewport* viewport = igGetMainViewport();
    igDockSpaceOverViewport(0, viewport, ImGuiDockNodeFlags_None, NULL);
}

static void editor_window_view() {
    const ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoNavInputs |
        ImGuiWindowFlags_NoMouseInputs |
        ImGuiWindowFlags_NoTitleBar;

    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, imv2f_ZERO);
    if(!igBegin("view", NULL, flags)) {
        igEnd();
        igPopStyleVar(1);
        return;
    }

    ImGuiViewport* viewport = igGetMainViewport();
    f32 top_margin = viewport->Size.y - viewport->WorkSize.y;

    ImVec2 pos, size;
    igGetWindowPos(&pos);
    igGetWindowSize(&size);

    ImGuiViewport* wview = igGetMainViewport();
    editor_ctx.view.pos = v2f_new(wview->WorkPos.x, wview->WorkPos.y);
    editor_ctx.view.size = v2f_new(wview->WorkSize.x, wview->WorkSize.y);

    imgui_texture_image_range(
        editor_ctx.render_texture,
        v2f_new(size.x, size.y - top_margin),
        v2f_new(pos.x / viewport->Size.x, pos.y / viewport->Size.y),
        v2f_new((pos.x + size.x) / viewport->Size.x, (pos.y + size.y) / viewport->Size.y)
    );

    ImVec2 cursor;
    igGetMousePos(&cursor);

    // used to account for when resizing the other windows
    const f32 safe_region = 8.0f;

    editor_ctx.view.focused = true;
    if(cursor.x < (pos.x + safe_region) || cursor.x > (pos.x + size.x - safe_region)) editor_ctx.view.focused = false;
    if(cursor.y < (pos.y + safe_region) || cursor.y > (pos.y + size.y - safe_region)) editor_ctx.view.focused = false;

    igEnd();
    igPopStyleVar(1);
}

static void editor_window_main() {
    if(!editor_ctx.editor.open) return;
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
        igDragFloat("zoom", &editor_ctx.cam.pixel_scale, 0.1f, 0.1f, editor_ctx.max_zoom, "%.1f", ImGuiSliderFlags_None);
    }

    igEnd();
}

static void resviewer_show_texture_contents(texture_t texture, bool show_image) {
    ImVec2 region;
    igGetContentRegionAvail(&region);

    texture_data_t* texture_data = texture_get_data(texture);
    if(!texture_data) {
        igText("couldnt find texture data for texture [%u]. wtf?", texture.id);
        return;
    }

    if(show_image) {
        f32 aspect_ratio = (f32) texture_data->height / (f32) texture_data->width;
        f32 w = texture_data->width > region.x ? region.x : texture_data->width;
        f32 h = w * aspect_ratio;
        imgui_texture_image(texture, v2f_new(w, h));
    }

    igText("width [%u] height [%u]", texture_data->width, texture_data->height);

    igText("format [%s]", format_names[texture_data->format]);
    igText("filter mode [%s]", filter_names[texture_data->filter]);
    igText("wrap mode [%s]", wrap_names[texture_data->wrap]);
    igText("mipmaps [%u]", texture_data->mipmaps);
}

static void resviewer_show_sampler_contents(sampler_t sampler) {
    sampler_data_t* sampler_data = sampler_get_data(sampler);
    if(!sampler_data) {
        igText("couldnt find sampler data for sampler [%u]. wtf?", sampler.id);
        return;
    }

    igText("min filter [%s]", filter_names[sampler_data->min_filter]);
    igText("mag filter [%s]", filter_names[sampler_data->mag_filter]);
    igText("u wrap [%s]", wrap_names[sampler_data->u_wrap]);
    igText("v wrap [%s]", wrap_names[sampler_data->v_wrap]);
}

static void resviewer_show_att_contents(attachments_t att) {
    attachments_data_t* att_data = attachments_get_data(att);
    if(!att_data) {
        igText("couldnt find att data for att [%u]. wtf?", att.id);
        return;
    }

    char label[32];
    snprintf(label, sizeof(label), "colour attachments [%u]", att_data->num_colours);
    if(igTreeNode_Str(label)) {
        for(u32 i = 0; i < GFX_MAX_COLOUR_ATTACHMENTS; i ++) {
            texture_t col = att_data->colours[i];
            if(col.id == GFX_INVALID_ID) continue;
            mem_clear(label, sizeof(label));
            snprintf(label, sizeof(label), "colour att %u", i);
            if(igTreeNode_Str(label)) {
                igText("texture [%u] (index %u)", col.id, handle_index(col.id));
                resviewer_show_texture_contents(col, editor_ctx.resviewer.att_preview_contents);
                igTreePop();
            }
        }

        igTreePop();
    }

    if(att_data->depth_stencil.id != GFX_INVALID_ID) {
        if(igTreeNode_Str("depth-stencil attachment")) {
            resviewer_show_texture_contents(att_data->depth_stencil, editor_ctx.resviewer.att_preview_contents);
            igTreePop();
        }
    }
}

static void resviewer_show_shader_pass_contents(shader_pass_t pass) {
    igText("type [%s]", shader_pass_names[pass.type]);
    igBeginChild_Str("##pass_content", imv2f(0.0f, 400.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
    if(pass.src.ptr) igTextWrapped("%s", pass.src.ptr);
    else igText("couldnt fetch contents of pass");
    igEndChild();
}

static void resviewer_show_shader_contents(shader_t shader) {
    shader_data_t* shader_data = shader_get_data(shader);
    if(!shader_data) {
        igText("couldnt find shader data for shader [%u]. wtf?", shader.id);
        return;
    }

    igText("label [%s]", shader_data->pretty_name);

    char label[32];
    snprintf(label, sizeof(label), "uniforms [%u]", shader_data->uniform_block.num);
    if(igTreeNode_Str(label)) {
        igText("packed size [%u bytes]", shader_data->uniform_block.bytes);
        for(u32 i = 0; i < shader_data->uniform_block.num; i ++) {
            uniform_t uniform = shader_data->uniform_block.uniforms[i];
            igText("%s [%s]", uniform.name, uniform_type_names[uniform.type]);
        }

        igTreePop();
    }

    if(igTreeNode_Str("vertex shader")) {
        resviewer_show_shader_pass_contents(shader_data->vertex_pass);
        igTreePop();
    }

    if(igTreeNode_Str("fragment shader")) {
        resviewer_show_shader_pass_contents(shader_data->fragment_pass);
        igTreePop();
    }
}

static void editor_window_resviewer() {
    if(!editor_ctx.resviewer.open) return;
    if(!igBegin("resviewer", &editor_ctx.resviewer.open, ImGuiWindowFlags_None)) {
        igEnd();
        return;
    }

    // these should be made into functions but like
    // who cares. it works.
    if(igBeginTabBar("##resviewer_bar", ImGuiTabBarFlags_None)) {
        if(igBeginTabItem("textures", NULL, ImGuiTabItemFlags_None)) {
            igBeginChild_Str("##texturelist", imv2f(120.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
            pool_iter_t iter = { .absolute_index = 1, };
            while(pool_iter(&gfx_ctx.texture_pool->res_pool, &iter)) {
                char label[32];
                snprintf(label, sizeof(label), "texture %u", iter.absolute_index);
                if(igSelectable_Bool(label, iter.absolute_index == editor_ctx.resviewer.selected_texture, ImGuiSelectableFlags_SelectOnClick, imv2f_ZERO)) {
                    if(editor_ctx.resviewer.selected_texture == iter.absolute_index) {
                        editor_ctx.resviewer.selected_texture = 0;
                        editor_ctx.resviewer.texture = (texture_t) {0};
                    } else {
                        editor_ctx.resviewer.selected_texture = iter.absolute_index;
                        editor_ctx.resviewer.texture = (texture_t) { iter.handle };
                    }
                }
            }

            igEndChild();
            igSameLine(120.0f, 12.0f);

            igBeginChild_Str("##textureviewer", imv2f_ZERO, ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

            if(editor_ctx.resviewer.selected_texture > 0) {
                resviewer_show_texture_contents(editor_ctx.resviewer.texture, true);
            } else {
                igText("no texture selected");
            }

            igEndChild();
            igEndTabItem();
        }

        if(igBeginTabItem("samplers", NULL, ImGuiTreeNodeFlags_None)) {
            igBeginChild_Str("##samplerlist", imv2f(120.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
            pool_iter_t iter = { .absolute_index = 1, };
            while(pool_iter(&gfx_ctx.sampler_pool->res_pool, &iter)) {
                char label[32];
                snprintf(label, sizeof(label), "sampler %u", iter.absolute_index);
                if(igSelectable_Bool(label, iter.absolute_index == editor_ctx.resviewer.selected_sampler, ImGuiSelectableFlags_SelectOnClick, imv2f_ZERO)) {
                    if(editor_ctx.resviewer.selected_sampler == iter.absolute_index) {
                        editor_ctx.resviewer.selected_sampler = 0;
                        editor_ctx.resviewer.sampler = (sampler_t) {0};
                    } else {
                        editor_ctx.resviewer.selected_sampler = iter.absolute_index;
                        editor_ctx.resviewer.sampler = (sampler_t) { iter.handle };
                    }
                }
            }

            igEndChild();
            igSameLine(120.0f, 12.0f);

            igBeginChild_Str("##samplerviewer", imv2f_ZERO, ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

            if(editor_ctx.resviewer.selected_sampler > 0) {
                resviewer_show_sampler_contents(editor_ctx.resviewer.sampler);
            } else {
                igText("no sampler selected");
            }

            igEndChild();
            igEndTabItem();
        }

        if(igBeginTabItem("attachments", NULL, ImGuiTreeNodeFlags_None)) {
            igBeginChild_Str("##attlist", imv2f(120.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
            igCheckbox("preview", &editor_ctx.resviewer.att_preview_contents);
            pool_iter_t iter = { .absolute_index = 1, };
            while(pool_iter(&gfx_ctx.attachments_pool->res_pool, &iter)) {
                char label[32];
                snprintf(label, 32, "att obj %u", iter.absolute_index);
                if(igSelectable_Bool(label, iter.absolute_index == editor_ctx.resviewer.selected_att, ImGuiSelectableFlags_SelectOnClick, imv2f_ZERO)) {
                    if(editor_ctx.resviewer.selected_att == iter.absolute_index) {
                        editor_ctx.resviewer.selected_att = 0;
                        editor_ctx.resviewer.att = (attachments_t) {0};
                    } else {
                        editor_ctx.resviewer.selected_att = iter.absolute_index;
                        editor_ctx.resviewer.att = (attachments_t) { iter.handle };
                    }
                }
            }

            igEndChild();
            igSameLine(120.0f, 12.0f);

            igBeginChild_Str("##attviewer", imv2f_ZERO, ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

            if(editor_ctx.resviewer.selected_att > 0) {
                resviewer_show_att_contents(editor_ctx.resviewer.att);
            } else {
                igText("no att obj selected");
            }

            igEndChild();
            igEndTabItem();
        }

        if(igBeginTabItem("shaders", NULL, ImGuiTreeNodeFlags_None)) {
            igBeginChild_Str("##shaderlist", imv2f(120.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
            pool_iter_t iter = { .absolute_index = 1, };
            while(pool_iter(&gfx_ctx.shader_pool->res_pool, &iter)) {
                char label[32];
                snprintf(label, 32, "shader %u", iter.absolute_index);
                if(igSelectable_Bool(label, iter.absolute_index == editor_ctx.resviewer.selected_shader, ImGuiSelectableFlags_SelectOnClick, imv2f_ZERO)) {
                    if(editor_ctx.resviewer.selected_shader == iter.absolute_index) {
                        editor_ctx.resviewer.selected_shader = 0;
                        editor_ctx.resviewer.shader = (shader_t) {0};
                    } else {
                        editor_ctx.resviewer.selected_shader = iter.absolute_index;
                        editor_ctx.resviewer.shader = (shader_t) { iter.handle };
                    }
                }
            }

            igEndChild();
            igSameLine(120.0f, 12.0f);

            igBeginChild_Str("##shaderviewer", imv2f_ZERO, ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

            if(editor_ctx.resviewer.selected_shader > 0) {
                resviewer_show_shader_contents(editor_ctx.resviewer.shader);
            } else {
                igText("no shader program selected");
            }

            igEndChild();
            igEndTabItem();
        }

        igEndTabBar();
    }

    igEnd();
}

// CAMERA
static void editor_camera_update() {
    camera_t* cam = &editor_ctx.cam;

    if(!editor_ctx.view.focused) return;

    f32 scroll = input_get_scroll();
    cam->pixel_scale -= scroll * 0.066f;
    cam->pixel_scale = MAX(cam->pixel_scale, 0.05f);
    cam->pixel_scale = MIN(cam->pixel_scale, editor_ctx.max_zoom);

    if(input_button_down(BUTTON_MIDDLE)) {
        v2f move = input_mouse_move_absolute();
        cam->transform.position.x -= move.x * cam->pixel_scale;
        cam->transform.position.y += move.y * cam->pixel_scale;
    }
}

// TOOLS
static void editor_hovered_tile_update() {
    if(!editor_ctx.view.focused) {
        editor_ctx.hovered_tile = v2i_new(-1, -1);
        return;
    }

    v2f view_pos = editor_ctx.view.pos;
    v2f view_size = editor_ctx.view.size;
    f32 scale = editor_ctx.cam.pixel_scale;

    f32 hw = io_ctx.window.width / 2.0f;
    f32 hh = io_ctx.window.height / 2.0f;

    v2f offset = input_mouse_pos();
    offset.x = remapf(offset.x, view_pos.x, view_pos.x + view_size.x, -hw,  hw);
    offset.y = remapf(offset.y, view_pos.y, view_pos.y + view_size.y,  hh, -hh); // flipped out range to make up +ve
    offset = v2f_add(offset, view_pos);
    offset = v2f_scale(offset, scale);

    v2f pos = editor_ctx.cam.transform.position;
    pos = v2f_add(pos, offset);

    v2i tile = v2i_new(
        floor(pos.x / TILE_WIDTH),
        floor(pos.y / TILE_HEIGHT)
    );

    if(editor_ctx.hovered_tile.x >= ROOM_WIDTH) tile.x = -1;
    if(editor_ctx.hovered_tile.y >= ROOM_HEIGHT) tile.x = -1;

    editor_ctx.hovered_tile = tile;
}

static void editor_tool_place() {
    if(!editor_ctx.view.focused) return;
    if(editor_ctx.hovered_tile.x < 0 || editor_ctx.hovered_tile.y < 0) return;
    if(!input_button_down(BUTTON_LEFT)) return;

    room_set_tile(&editor_ctx.room, (tile_t) {
        .x = editor_ctx.hovered_tile.x,
        .y = editor_ctx.hovered_tile.y,

        .tags = TILE_TAGS_RENDER,
        .col = v4f_new(0.2f, 0.84f, 0.55f, 1.0f),
    });
}

// GRID
static void editor_show_grid() {
    render_push_draw_call(&editor_ctx.renderer.groups[0], (draw_call_t) {});
}

// ROOM
static void editor_show_room() {
    room_t room = editor_ctx.room;
    v3f scale = v3f_new(TILE_WIDTH / 2.0f, TILE_HEIGHT / 2.0f, 1.0f);
    for(u32 y = 0; y < ROOM_HEIGHT; y ++) {
        for(u32 x = 0; x < ROOM_WIDTH; x ++) {
            tile_t tile = room.tiles[y][x];
            v2f pos = tile_get_world_pos(tile);

            if(tile.tags & TILE_TAGS_RENDER) {
                render_push_draw_call(&editor_ctx.renderer.groups[1], (draw_call_t) {
                    .position = v3f_new(pos.x, pos.y, 0),
                    .scale = scale,
                    .colour = tile.col,
                });
            }
        }
    }

    if(editor_ctx.hovered_tile.x < 0 || editor_ctx.hovered_tile.y < 0) return;
    v2f pos = tile_get_world_pos((tile_t) { .x = editor_ctx.hovered_tile.x, .y = editor_ctx.hovered_tile.y });
    render_push_draw_call(&editor_ctx.renderer.groups[1], (draw_call_t) {
        .position = v3f_new(pos.x, pos.y, 1),
        .scale = scale,
        .colour = v4f_new(1.0, 1.0f, 1.0f, 0.1f),
    });
}

void editor_update() {
    igPushStyleVar_Float(ImGuiStyleVar_TabRounding, 0.0f);
    editor_topbar();
    editor_dockspace();
    editor_window_view();
    editor_window_main();
    editor_window_resviewer();
    igPopStyleVar(1);

    editor_camera_update();
    editor_hovered_tile_update();
    editor_tool_place();

    editor_show_grid();
    editor_show_room();

    editor_render();
}

void editor_render() {
    for(u32 i = 0; i < editor_ctx.renderer.num_groups; i ++)
        camera_attach(&editor_ctx.cam, &editor_ctx.renderer.groups[i].pass);

    render_dispatch(&editor_ctx.renderer);
}
