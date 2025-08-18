#ifndef _CAMERA_H
#define _CAMERA_H

#include "base.h"
#include "entity.h"

typedef struct {
    transform_t transform;

    f32 pixel_scale;
    f32 near, far;
} camera_t;

void camera_attach();
void camera_update();

#endif
