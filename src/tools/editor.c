#include "editor.h"
#include "base.h"
#include "base_macros.h"
#include "errors/errors.h"
#include "game/camera.h"
#include "game/room.h"
#include "imgui/imgui_manager.h"
#include "io/io.h"
#include "memory/memory.h"
#include "physics/bounds.h"
#include "platform/platform.h"
#include "render/render.h"
#include "util/util.h"
#include "util/math_util.h"
#include "gfx/gfx.h"

// temporary because my cmp keeps auto including this stupid file and its causing errors
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

editor_ctx_t editor_ctx = {0};

// KEYBINDS:
//  => GLOBAL
//      - clear active tool [BACKSPACE]
//      - place tool        [1]
//      - select tool       [2]
//      - delete tool       [3]
//      - move view         [MIDDLE MOUSE]
//      - enable alt mode   [LEFT ALT]
//      - cancel selection  [ESC]
//      - delete selection  [CTRL+D] [DEL]
//      - select all        [CTRL+A]

static void editor_render_to_grid_pass(draw_call_t call);
static void editor_render_to_room_pass(draw_call_t call);
static void editor_render_to_selection_pass(draw_call_t call);

static void editor_tile_delete(v2i pos);
static bool editor_tile_selected(v2i pos);
static v2i editor_tile_at_screen_pos(v2f offset);
static v2f editor_tile_get_screen_pos(v2i tile);
static void editor_tile_show_info(tile_t tile);
static void editor_tile_show_settings(tile_t tile);

static void editor_selection_clear();
static void editor_selection_delete();
static void editor_select_range(v2i min, v2i max);
static void editor_selection_update();
static void editor_selection_show_info();

static void editor_topbar();
static void editor_dockspace();
static void editor_window_view();
static void editor_window_resviewer();
static void editor_window_tools();
static void editor_tooltip_tile_info();

static void resviewer_show_texture_contents(texture_t texture, bool show_image);
static void resviewer_show_sampler_contents(sampler_t sampler);
static void resviewer_show_att_contents(attachments_t att);
static void resviewer_show_shader_pass_contents(shader_pass_t pass);
static void resviewer_show_shader_contents(shader_t shader);

static void editor_tool_place();
static void editor_tool_select();
static void editor_tool_delete();
static void editor_tool_update();

static void editor_show_grid();
static void editor_show_room();

static void editor_camera_update();
static void editor_hovered_tile_update();

static void editor_render();

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

static const char* tool_names[] = {
    STRINGIFY(NONE),
    STRINGIFY(PLACE),
    STRINGIFY(SELECT),
    STRINGIFY(DELETE),
};

// UNIFORMS
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

static void selection_construct_uniforms(void* out, draw_call_t* call) {
    struct __attribute__((packed)) {
        vec2 start;
        vec2 end;
        f32 outline_width;
        vec4 outline;
        vec4 inside;
    } uniforms;

    memcpy(uniforms.start, call->min.raw, sizeof(vec2));
    memcpy(uniforms.end, call->max.raw, sizeof(vec2));

    uniforms.outline_width = call->stroke;

    memcpy(uniforms.outline, call->colour.raw, sizeof(vec4));
    memcpy(uniforms.inside, call->bg.raw, sizeof(vec4));

    memcpy(out, &uniforms, sizeof(uniforms));
}

// STATE
void editor_init() {
    // leak ALL the memory
    arena_t code_arena = arena_alloc_new(4096, EXPAND_TYPE_IMMUTABLE);
    range_t grid_vs = platform_load_file(&code_arena, "shader/editor/grid.vs");
    range_t grid_fs = platform_load_file(&code_arena, "shader/editor/grid.fs");
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

    range_t room_vs = platform_load_file(&code_arena, "shader/editor/room.vs");
    range_t room_fs = platform_load_file(&code_arena, "shader/editor/room.fs");
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

    range_t sel_vs = platform_load_file(&code_arena, "shader/editor/selection.vs");
    range_t sel_fs = platform_load_file(&code_arena, "shader/editor/selection.fs");
    shader_t sel_shader = shader_new((shader_info_t) {
        .pretty_name = "selection shader",
        .attribs = {
            { .name = "vs_position" },
            { .name = "vs_uvs" },
        },
        .uniforms = {
            { .name = "start", .type = UNIFORM_TYPE_v2f, },
            { .name = "end", .type = UNIFORM_TYPE_v2f, },
            { .name = "outline_width", .type = UNIFORM_TYPE_f32, },
            { .name = "outline_col", .type = UNIFORM_TYPE_v4f, },
            { .name = "inside_col", .type = UNIFORM_TYPE_v4f, },
        },
        .vertex_src = sel_vs,
        .fragment_src = sel_fs,
    });

    texture_t col_target = texture_new((texture_info_t) {
        .type = TEXTURE_TYPE_2D,
        .format = TEXTURE_FORMAT_RGBA32F,
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
        .num_groups = 3,
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
                .batch = vector_alloc_new(1, sizeof(draw_call_t)),
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
                            .dst_func = BLEND_FUNC_SRC_ONE_MINUS_ALPHA,
                            .src_func = BLEND_FUNC_SRC_ALPHA,
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
            [2] = {
                .pass = {
                    .label = "select pass",
                    .type = DRAW_PASS_RENDER,
                    .pipeline = {
                        .cull = { .enable = true, },
                        .blend = {
                            .enable = true,
                            .dst_func = BLEND_FUNC_SRC_ONE_MINUS_ALPHA,
                            .src_func = BLEND_FUNC_SRC_ALPHA,
                        },
                        .draw_attachments = att,
                        .colour_targets = {
                            [0] = { .enable = true, },
                        },
                        .shader = sel_shader,
                    },
                },
                .batch = vector_alloc_new(8, sizeof(draw_call_t)),
                .construct_uniforms = selection_construct_uniforms,
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

    editor_ctx = (editor_ctx_t) {
        .open = true,

        .cam = cam,
        .max_zoom = 2.0f,
        .renderer = renderer,
        .render_texture = col_target,

        .editor = {
            .open = true,
        },

        .resviewer = {
            .open = true,
        },

        .select_tool.drag.cancel_key = KEY_ESCAPE,
        .selection = {
            .tiles = {0},
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

// RENDERING
static void editor_render_to_grid_pass(draw_call_t call) {
    render_push_draw_call(&editor_ctx.renderer.groups[0], call);
}

static void editor_render_to_room_pass(draw_call_t call) {
    render_push_draw_call(&editor_ctx.renderer.groups[1], call);
}

static void editor_render_to_selection_pass(draw_call_t call) {
    render_push_draw_call(&editor_ctx.renderer.groups[2], call);
}

// TILES
static void editor_tile_delete(v2i pos) {
    if(pos.x < 0 || pos.x >= ROOM_WIDTH || pos.y < 0 || pos.y >= ROOM_HEIGHT) {
        LOG_ERR_CODE(ERR_EDITOR_SELECTION_OUTSIDE_BOUNDS);
        return;
    }

    room_set_tile(&editor_ctx.room, (tile_t) {
        .x = pos.x,
        .y = pos.y,
    });
}

static bool editor_tile_selected(v2i pos) {
    if(!editor_ctx.selection.selected) return false;
    if(pos.x < editor_ctx.selection.min.x || pos.x > editor_ctx.selection.max.x) return false;
    if(pos.y < editor_ctx.selection.min.y || pos.y > editor_ctx.selection.max.y) return false;

    vector_t* tiles = &editor_ctx.selection.tiles;
    for(u32 i = 0; i < tiles->size; i ++) {
        v2i* tile = vector_get(tiles, i);
        if(!tile) continue;
        if(tile->x == pos.x && tile->y == pos.y) return true;
    }

    return false;
}

static v2i editor_tile_at_screen_pos(v2f offset) {
    f32 scale = editor_ctx.cam.pixel_scale;

    f32 w = io_ctx.window.width;
    f32 h = io_ctx.window.height;
    f32 hw = w / 2.0f;
    f32 hh = h / 2.0f;

    offset.x = remapf(offset.x, 0.0f, w, -hw, hw);
    offset.y = remapf(offset.y, 0.0f, h, -hh, hh);
    offset = v2f_scale(offset, scale);

    v2f pos = editor_ctx.cam.transform.position;
    pos = v2f_add(pos, offset);

    v2i tile = v2i_new(
        floor(pos.x / TILE_WIDTH),
        floor(pos.y / TILE_HEIGHT)
    );

    if(tile.x >= ROOM_WIDTH) tile.x = -1;
    if(tile.y >= ROOM_HEIGHT) tile.x = -1;

    return tile;
}

static v2f editor_tile_get_screen_pos(v2i tile) {
    f32 scale = editor_ctx.cam.pixel_scale;
    v2f pos = v2f_new(tile.x * TILE_WIDTH / scale, tile.y * TILE_HEIGHT / scale);
    v2f offset = v2f_scale(editor_ctx.cam.transform.position, -1.0f / scale);
    offset.x += io_ctx.window.width / 2.0f;
    offset.y += io_ctx.window.height / 2.0f;
    return v2f_add(pos, offset);
}

static void editor_tile_show_info(tile_t tile) {
    const ImGuiColorEditFlags col_flags = ImGuiColorEditFlags_None;

    char label[32];
    snprintf(label, sizeof(label), "tile [%d, %d]", v2f_expand(editor_ctx.hovered_tile));
    igSetNextItemOpen(true, ImGuiCond_Appearing);
    if(igTreeNode_Str(label)) {
        igSetNextItemOpen(true, ImGuiCond_Appearing);
        if(igTreeNode_Str("tags")) {
            if(tile.tags == TILE_TAGS_NONE) igBulletText("none");
            if(tile.tags & TILE_TAGS_RENDER) igBulletText("render");
            if(tile.tags & TILE_TAGS_SOLID) igBulletText("solid");
            igTreePop();
        }

        igColorEdit4("colour", tile.col.raw, col_flags);
        igText("selected [%s]", editor_tile_selected(v2i_new(tile.x, tile.y)) ? "yes" : "no");
        igTreePop();
    }
}

static void editor_tile_show_settings(tile_t tile) {
    const ImGuiColorEditFlags col_flags = ImGuiColorEditFlags_None;

    char label[32];
    snprintf(label, sizeof(label), "tile [%d, %d]", v2f_expand(editor_ctx.hovered_tile));
    igSetNextItemOpen(true, ImGuiCond_Appearing);
    if(igTreeNode_Str(label)) {
        igSetNextItemOpen(true, ImGuiCond_Appearing);
        if(igTreeNode_Str("tags")) {
            if(tile.tags == TILE_TAGS_NONE) igBulletText("none");
            if(tile.tags & TILE_TAGS_RENDER) igBulletText("render");
            if(tile.tags & TILE_TAGS_SOLID) igBulletText("solid");
            igTreePop();
        }

        igColorEdit4("colour", tile.col.raw, col_flags);
        igTreePop();
    }

    room_set_tile(&editor_ctx.room, tile);
}

// SELECTION
static void editor_selection_clear() {
    editor_ctx.selection.selected = false;
    editor_ctx.selection.min = v2i_new(-1, -1);
    editor_ctx.selection.max = v2i_new(-1, -1);
    vector_destroy(&editor_ctx.selection.tiles);
}

static void editor_selection_delete() {
    if(!editor_ctx.selection.selected) return;

    vector_t* tiles = &editor_ctx.selection.tiles;
    for(u32 i = 0; i < tiles->size; i ++) {
        v2i* tile = vector_get(tiles, i);
        if(!tile) continue;
        editor_tile_delete(*tile);
    }

    editor_selection_clear();
}

static void editor_select_range(v2i min, v2i max) {
    bool in_range = true;
    if(min.x < 0 || max.x < 0 || min.x >= ROOM_WIDTH || max.x >= ROOM_WIDTH) in_range = false;
    if(min.y < 0 || max.y < 0 || min.y >= ROOM_HEIGHT || max.y >= ROOM_HEIGHT) in_range = false;
    if(!in_range) {
        LOG_ERR_CODE(ERR_EDITOR_SELECTION_OUTSIDE_BOUNDS);
        return;
    }

    editor_selection_clear();

    u32 num_tiles = 0;
    for(i32 y = min.y; y <= max.y; y ++) {
        for(i32 x = min.x; x <= max.x; x ++) {
            tile_t tile = room_get_tile(&editor_ctx.room, x, y);
            if(tile.tags != TILE_TAGS_NONE) num_tiles ++;
        }
    }

    if(num_tiles == 0) return;

    editor_ctx.selection.tiles = vector_alloc_new(num_tiles, sizeof(v2i));

    v2i range_min = max;
    v2i range_max = min;

    for(i32 y = min.y; y <= max.y; y ++) {
        for(i32 x = min.x; x <= max.x; x ++) {
            tile_t tile = room_get_tile(&editor_ctx.room, x, y);
            if(tile.tags != TILE_TAGS_NONE) {
                vector_push_data(&editor_ctx.selection.tiles, &v2i_new(x, y));
                range_min.x = MIN(range_min.x, x);
                range_min.y = MIN(range_min.y, y);
                range_max.x = MAX(range_max.x, x);
                range_max.y = MAX(range_max.y, y);
            }
        }
    }

    editor_ctx.selection.selected = true;
    editor_ctx.selection.min = range_min;
    editor_ctx.selection.max = range_max;
}

static void editor_selection_update() {
    if(input_key_pressed(KEY_ESCAPE))
        editor_selection_clear();

    if((input_key_down(KEY_LEFT_CONTROL) && input_key_pressed(KEY_D)) || input_key_pressed(KEY_DELETE))
        editor_selection_delete();

    if(input_key_down(KEY_LEFT_CONTROL) && input_key_pressed(KEY_A))
        editor_select_range(v2i_ZERO, v2i_new(ROOM_WIDTH - 1, ROOM_HEIGHT - 1));

    if(!editor_ctx.selection.selected) return;

    bool edge_tile_removed = false;
    vector_t* tiles = &editor_ctx.selection.tiles;
    for(i32 i = tiles->size - 1; i >= 0; i --) {
        v2i* pos = vector_get(tiles, i);
        if(!pos) continue;

        tile_t tile = room_get_tile(&editor_ctx.room, pos->x, pos->y);
        if(tile.tags == TILE_TAGS_NONE) {
            if(pos->x == editor_ctx.selection.min.x) edge_tile_removed = true;
            if(pos->x == editor_ctx.selection.max.x) edge_tile_removed = true;
            if(pos->y == editor_ctx.selection.min.y) edge_tile_removed = true;
            if(pos->y == editor_ctx.selection.max.y) edge_tile_removed = true;

            vector_remove(tiles, i);
        }
    }

    if(edge_tile_removed) {
        v2i range_min = editor_ctx.selection.max;
        v2i range_max = editor_ctx.selection.min;
        for(u32 i = 0; i < tiles->size; i ++) {
            v2i* pos = vector_get(tiles, i);
            if(!pos) continue;
            range_min.x = MIN(range_min.x, pos->x);
            range_min.y = MIN(range_min.y, pos->y);
            range_max.x = MAX(range_max.x, pos->x);
            range_max.y = MAX(range_max.y, pos->y);
        }

        editor_ctx.selection.min = range_min;
        editor_ctx.selection.max = range_max;
    }

    if(editor_ctx.alt_mode) {
        v2f min = editor_tile_get_screen_pos(editor_ctx.selection.min);
        v2f max = editor_tile_get_screen_pos(v2i_add(editor_ctx.selection.max, v2i_ONE));

        editor_render_to_selection_pass((draw_call_t) {
            .min = min,
            .max = max,
            .stroke = 2.0f,
            .colour = v4f_new(0.0f, 1.0f, 0.0f, 0.9f),
            .bg = v4f_new(1.0f, 1.0f, 1.0f, 0.2f),
        });
    }
}

static void editor_selection_show_info() {
    if(!editor_ctx.selection.selected) return;

    if(editor_ctx.selection.tiles.size > 1) {
        igSeparatorText("SELECTION");
        igText("selected region: [%i, %i] to [%i, %i]", v2f_expand(editor_ctx.selection.min), v2f_expand(editor_ctx.selection.max));
        igText("number of selected tiles: [%u]", editor_ctx.selection.tiles.size);
    } else {
        v2i* pos = vector_get(&editor_ctx.selection.tiles, 0);
        if(!pos) return;
        editor_tile_show_settings(room_get_tile(&editor_ctx.room, pos->x, pos->y));
    }
}

// EDITOR WINDOWS
static void editor_topbar() {
    if(igBeginMainMenuBar()) {
        if(igBeginMenu("editor", true)) {
            if(igMenuItem_Bool("quit", "F10", false, true))
                editor_toggle();

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

// VIEW
static void editor_window_view() {
    const ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoCollapse     |
        ImGuiWindowFlags_NoBackground   |
        ImGuiWindowFlags_NoDecoration   |
        ImGuiWindowFlags_NoInputs       |
        ImGuiWindowFlags_NoNavFocus     |
        ImGuiWindowFlags_NoNavInputs    |
        ImGuiWindowFlags_NoMouseInputs  |
        ImGuiWindowFlags_NoTitleBar     |
        ImGuiWindowFlags_NoScrollbar    |
        ImGuiWindowFlags_NoScrollWithMouse;

    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, imv2f_ZERO);
    if(!igBegin("view", NULL, flags)) {
        igEnd();
        igPopStyleVar(1);
        return;
    }

    ImVec2 pos, size, content;
    igGetWindowPos(&pos);
    igGetWindowSize(&size);
    igGetContentRegionAvail(&content);

    pos.y += size.y - content.y;
    pos.x -= size.x - content.x; // ???

    f32 w = io_ctx.window.width;
    f32 h = io_ctx.window.height;

    imgui_texture_image_range(
        editor_ctx.render_texture,
        v2f_new(content.x, content.y),
        v2f_new(pos.x / w, (h - (pos.y + content.y)) / h),
        v2f_new((pos.x + content.x) / w, (h - pos.y) / h)
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

// MAIN
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

    if(editor_ctx.alt_mode) {
        igSeparatorText("ALT MODE");
    }

    editor_selection_show_info();

    igEnd();
}

// RESOURCE VIEWER
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

static void editor_window_tools() {
    if(!igBegin("tools", &editor_ctx.tools.open, ImGuiWindowFlags_None)) {
        igEnd();
        return;
    }

    for(u32 i = 1; i < _EDITOR_TOOLS_NUM; i ++) {
        if(igSelectable_Bool(tool_names[i], editor_ctx.tools.active_tool == i, ImGuiSelectableFlags_SelectOnClick, imv2f_ZERO))
            editor_ctx.tools.active_tool = editor_ctx.tools.active_tool == i ? 0 : i;
    }

    igEnd();
}

static void editor_tooltip_tile_info() {
    if(!editor_ctx.alt_mode) return;
    if(editor_ctx.hovered_tile.x < 0 || editor_ctx.hovered_tile.y < 0) return;

    tile_t hovered = room_get_tile(&editor_ctx.room, editor_ctx.hovered_tile.x, editor_ctx.hovered_tile.y);
    if(hovered.tags == TILE_TAGS_NONE) return;

    if(igBeginTooltip()) {
        editor_tile_show_info(hovered);
        igEndTooltip();
    }
}

// CAMERA
static void editor_camera_update() {
    camera_t* cam = &editor_ctx.cam;

    if(!editor_ctx.view.focused) return;
    if(editor_ctx.select_tool.selecting) return;

    f32 scroll = input_get_scroll();
    cam->pixel_scale -= scroll * 0.05f;
    cam->pixel_scale = MAX(cam->pixel_scale, 0.05f);
    cam->pixel_scale = MIN(cam->pixel_scale, editor_ctx.max_zoom);

    if(input_button_down(BUTTON_MIDDLE)) {
        v2f move = input_mouse_move_absolute();
        cam->transform.position.x -= move.x * cam->pixel_scale;
        cam->transform.position.y -= move.y * cam->pixel_scale;
    }
}

// TOOLS
static void editor_hovered_tile_update() {
    if(!editor_ctx.view.focused) {
        editor_ctx.hovered_tile = v2i_new(-1, -1);
        return;
    }

    editor_ctx.hovered_tile = editor_tile_at_screen_pos(input_mouse_pos());
}

static void editor_tool_place() {
    if(!input_button_down(BUTTON_LEFT)) return;
    if(editor_ctx.hovered_tile.x < 0 || editor_ctx.hovered_tile.y < 0) return;

    room_set_tile(&editor_ctx.room, (tile_t) {
        .x = editor_ctx.hovered_tile.x,
        .y = editor_ctx.hovered_tile.y,

        .tags = TILE_TAGS_RENDER,
        .col = v4f_new(0.2f, 0.84f, 0.55f, 1.0f),
    });
}

static void editor_tool_select() {
    mouse_drag_t* drag = &editor_ctx.select_tool.drag; 
    input_drag(drag);

    if(!editor_ctx.select_tool.selecting && drag->state == DRAG_STATE_DRAGGING) {
        editor_ctx.select_tool.selecting = true;
        editor_selection_clear();
    }

    if(drag->state == DRAG_STATE_ACCEPTED) {
        v2i min = editor_tile_at_screen_pos(drag->min);
        v2i max = editor_tile_at_screen_pos(drag->max);

        if((min.x >= 0 || max.x >= 0) && (min.y >= 0 || max.y >= 0)) {
            if(min.x < 0) min.x = 0;
            if(min.y < 0) min.y = 0;
            if(max.x < 0) max.x = 0;
            if(max.y < 0) max.y = 0;
            editor_select_range(min, max);
        }
    }

    if(drag->state != DRAG_STATE_DRAGGING)
        editor_ctx.select_tool.selecting = false;

    if(editor_ctx.select_tool.selecting) {
        editor_render_to_selection_pass((draw_call_t) {
            .min = drag->min,
            .max = drag->max,
            .stroke = 2.0f,
            .colour = v4f_new(0.0f, 1.0f, 0.0f, 0.9f),
            .bg = v4f_new(1.0f, 1.0f, 1.0f, 0.2f),
        });
    }
}

static void editor_tool_delete() {
    if(!input_button_down(BUTTON_LEFT)) return;
    if(editor_ctx.hovered_tile.x < 0 || editor_ctx.hovered_tile.y < 0) return;
    editor_tile_delete(editor_ctx.hovered_tile);
}

static void editor_tool_update() {
    if(!editor_ctx.select_tool.selecting) {
        for(u32 i = 1; i < _EDITOR_TOOLS_NUM; i ++) {
            if(input_key_pressed(KEY_0 + i)) {
                editor_ctx.tools.active_tool = i;
                break;
            }
        }

        if(input_key_pressed(KEY_BACKSPACE)) editor_ctx.tools.active_tool = EDITOR_TOOL_NONE;
    }

    if(!editor_ctx.view.focused) {
        if(editor_ctx.select_tool.selecting) {
            input_drag_interrupt(&editor_ctx.select_tool.drag);
        }

        return;
    }

    switch(editor_ctx.tools.active_tool) {
        case EDITOR_TOOL_PLACE: editor_tool_place(); break;
        case EDITOR_TOOL_SELECT: editor_tool_select(); break;
        case EDITOR_TOOL_DELETE: editor_tool_delete(); break;
        default: break;
    }

    const v4f hover_colours[_EDITOR_TOOLS_NUM] = {
        [EDITOR_TOOL_NONE]   = v4f_ZERO,
        [EDITOR_TOOL_PLACE]  = v4f_new(1.0, 1.0f, 1.0f, 0.6f),
        [EDITOR_TOOL_SELECT] = v4f_ZERO,
        [EDITOR_TOOL_DELETE] = v4f_new(0.92f, 0.04f, 0.12f, 0.6f),
    };

    v2f pos = tile_get_world_pos((tile_t) { .x = editor_ctx.hovered_tile.x, .y = editor_ctx.hovered_tile.y });
    v4f col = hover_colours[editor_ctx.tools.active_tool];
    editor_render_to_room_pass((draw_call_t) {
        .position = v3f_new(pos.x, pos.y, 1),
        .scale = v3f_new(TILE_WIDTH / 2.0f, TILE_HEIGHT / 2.0f, 1.0f),
        .colour = col,
    });
}

// GRID
static void editor_show_grid() {
    editor_render_to_grid_pass((draw_call_t) {});
}

// ROOM
static void editor_show_room() {
    room_t room = editor_ctx.room;
    v3f scale = v3f_new(TILE_WIDTH / 2.0f, TILE_HEIGHT / 2.0f, 1.0f);
    for(u32 y = 0; y < ROOM_HEIGHT; y ++) {
        for(u32 x = 0; x < ROOM_WIDTH; x ++) {
            tile_t tile = room_get_tile(&room, x, y);
            v2f pos = tile_get_world_pos(tile);

            if(!(tile.tags & TILE_TAGS_RENDER)) continue;
            editor_render_to_room_pass((draw_call_t) {
                .position = v3f_new(pos.x, pos.y, 0),
                .scale = scale,
                .colour = tile.col,
            });

            if(editor_tile_selected(v2i_new(tile.x, tile.y))) {
                editor_render_to_room_pass((draw_call_t) {
                    .position = v3f_new(pos.x, pos.y, 1),
                    .scale = scale,
                    .colour = v4f_new(0.0f, 1.0f, 0.0f, 0.4f),
                });
            }
        }
    }

    editor_render_to_selection_pass((draw_call_t) {
        .min = editor_tile_get_screen_pos(v2i_ZERO),
        .max = editor_tile_get_screen_pos(v2i_new(ROOM_WIDTH, ROOM_HEIGHT)),
        .stroke = 3.0f,
        .colour = v4f_new(1.0f, 1.0f, 1.0f, 0.1f),
    });
}

static void editor_render() {
    for(u32 i = 0; i < editor_ctx.renderer.num_groups; i ++)
        camera_attach(&editor_ctx.cam, &editor_ctx.renderer.groups[i].pass);

    render_dispatch(&editor_ctx.renderer);
}

void editor_update() {
    editor_ctx.alt_mode = input_key_down(KEY_LEFT_ALT);
    igPushStyleVar_Float(ImGuiStyleVar_TabRounding, 0.0f);
    editor_topbar();
    editor_dockspace();
    editor_window_main();
    editor_window_resviewer();
    editor_window_tools();
    editor_window_view();
    editor_tooltip_tile_info();

    igPopStyleVar(1);

    editor_camera_update();
    editor_show_grid();
    editor_show_room();

    // these should be before the functions that draw stuff
    // but since i dont sort by z-layer before rendering,
    // it messes up blending
    // so for now keep it here
    editor_hovered_tile_update();
    editor_tool_update();

    editor_selection_update();

    editor_render();
}
