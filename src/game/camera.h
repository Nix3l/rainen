#ifndef _CAMERA_H
#define _CAMERA_H

#include "base.h"
#include "entity.h"
#include "render/render.h"

// TODO(nix3l): redo this whole thing

typedef struct camera_t {
    transform_t transform;
    f32 pixel_scale;
    f32 near, far;
} camera_t;

void camera_attach(camera_t* cam, draw_pass_t* pass);

#endif
