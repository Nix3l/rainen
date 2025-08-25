#ifndef _EDITOR_H
#define _EDITOR_H

#include "base.h"
#include "gfx/gfx.h"
#include "render/render.h"
#include "game/camera.h"
#include "game/room.h"

// TODOs:
//  => figure out the room_t architecture

typedef struct editor_ctx_t {
    bool open;

    camera_t cam;
    f32 max_zoom;
    renderer_t renderer;
    texture_t render_texture;
    bool view_focused;

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

    room_t room;
    renderer_t room_renderer;
} editor_ctx_t;

void editor_init();
void editor_terminate();

void editor_set_open(bool open);
void editor_toggle();
bool editor_is_open();

void editor_update();
void editor_render();

extern editor_ctx_t editor_ctx;

#endif
