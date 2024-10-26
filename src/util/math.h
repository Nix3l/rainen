#ifndef MATH_H
#define MATH_H

#include "base.h"

mat4s get_transformation_matrix(v2f position, f32 rotation, v2f scale);

v3f yaw_pitch_to_direction(f32 yaw, f32 pitch);
v3f yaw_to_right(f32 yaw);
v3f yaw_pitch_to_up(f32 yaw, f32 pitch);

#endif
