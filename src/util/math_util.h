#ifndef _UTIL_MATH_H
#define _UTIL_MATH_H

#include "base.h"

mat4s model_matrix_new(v3f pos, v3f rot, v3f scale);
mat4s view_matrix_new(v3f pos, v3f rot);
mat4s proj_perspective_matrix_new(f32 fov, f32 aspect_ratio, f32 near, f32 far);
mat4s proj_ortho_matrix_new(f32 w, f32 h, f32 near, f32 far);

f32 remapf(f32 v, f32 l0, f32 h0, f32 ln, f32 hn);

#endif
