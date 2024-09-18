#ifndef CAMERA_H
#define CAMERA_H

#include "base.h"

typedef struct {
    v3f position;
    v3f rotation;

    f32 fov;
    f32 near_plane;
    f32 far_plane;

    // MOVEMENT
    f32 speed;
    f32 sens;
} camera_s;

void update_camera(camera_s* camera);

mat4s camera_projection(camera_s* camera);
mat4s camera_view(camera_s* camera);
mat4s camera_projection_view(camera_s* camera);

#endif
