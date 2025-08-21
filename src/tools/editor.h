#ifndef _EDITOR_H
#define _EDITOR_H

#include "base.h"
#include "gfx/gfx.h"
#include "render/render.h"
#include "game/camera.h"

// TODOs:
//  => allow multiple cameras, so one can be used in the editor and one outside it
//  => 

typedef struct editor_ctx_t {
    bool open;

    camera_t cam;
    renderer_t renderer;

    struct {
        bool open;
    } editor;

    struct {
        bool open;
        u32 selected_texture;
        texture_t texture;
    } resviewer;
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
