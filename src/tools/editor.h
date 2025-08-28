#ifndef _EDITOR_H
#define _EDITOR_H

#include "base.h"
#include "gfx/gfx.h"
#include "io/io.h"
#include "render/render.h"
#include "game/camera.h"
#include "game/room.h"

// TODOs:
//  => more robust tile data editing
//  => room_t architecture

typedef enum editor_tool_t {
    EDITOR_TOOL_NONE = 0,
    EDITOR_TOOL_PLACE,
    EDITOR_TOOL_SELECT,
    EDITOR_TOOL_DELETE,
    _EDITOR_TOOLS_NUM,
} editor_tool_t;

typedef struct editor_ctx_t {
    bool open;
    bool alt_mode;

    camera_t cam;
    f32 max_zoom;

    renderer_t renderer;
    texture_t render_texture;

    struct {
        bool focused;
    } view;

    struct {
        bool open;
    } editor;

    struct {
        bool open;

        u32 selected_texture;
        texture_t texture;

        u32 selected_sampler;
        sampler_t sampler;

        bool att_preview_contents;
        u32 selected_att;
        attachments_t att;

        u32 selected_shader;
        shader_t shader;
    } resviewer;

    v2i hovered_tile;
    v4f hovered_col;

    struct {
        bool open;
        editor_tool_t active_tool;
    } tools;

    struct {
        bool picking;
        tile_tags_t tags;
        tile_data_t data;
    } place_tool;

    struct {
        bool selecting;
        mouse_drag_t drag;
    } select_tool;

    struct {
        bool selected;

        // inclusive
        v2i min;
        v2i max;

        // holds tile grid v2i positions [x, y]
        vector_t tiles;
    } selection;

    room_t room;
} editor_ctx_t;

void editor_init();
void editor_terminate();

void editor_set_open(bool open);
void editor_toggle();
bool editor_is_open();

void editor_update();

extern editor_ctx_t editor_ctx;

#endif
