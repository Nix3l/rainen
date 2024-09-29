#ifndef CAMERA_H
#define CAMERA_H

#include "base.h"

typedef enum {
    PERSPECTIVE_PROJECTION = 1,
    ORTHOGRAPHIC_PROJECTION = 2,
} projection_e;

typedef struct {
    v3f position;
    v3f rotation;
    
    // PROJECTION
    f32 near_plane, far_plane;
    f32 fov; // PERSPECTIVE
    f32 ortho_width, ortho_height; // ORTHOGRAPHIC

    mat4s projection;
    mat4s view;
    mat4s projection_view;

    // MOVEMENT
    f32 speed;
    f32 sens;
} camera_s;

void update_camera(camera_s* camera);

mat4s camera_perspective_projection(camera_s* camera);
mat4s camera_orthographic_projection(camera_s* camera);
mat4s camera_view(camera_s* camera);
mat4s camera_projection_view(camera_s* camera);

#endif
