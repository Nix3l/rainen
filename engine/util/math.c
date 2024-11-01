#include "./math.h"

mat4s get_transformation_matrix(v2f position, f32 rotation, v2f scale) {
    mat4s transformation = glms_translate(MAT4_IDENTITY, V3F(position.x, position.y, 0.0f));
    transformation = glms_rotate(transformation, rotation, V3F(0.0f, 0.0f, 1.0f));
    transformation = glms_scale(transformation, V3F(scale.x, scale.y, 1.0f));
    return transformation;
}

v3f yaw_pitch_to_direction(f32 yaw, f32 pitch) {
    return V3F(
            -cosf(pitch) * sinf(yaw),
             sinf(pitch),
             cosf(yaw) * cosf(pitch)
        );
}

v3f yaw_to_right(f32 yaw) {
    return V3F(
            cosf(yaw),
            0,
            sinf(yaw)
        );
}

v3f yaw_pitch_to_up(f32 yaw, f32 pitch) {
    return glms_cross(yaw_pitch_to_direction(yaw, pitch), yaw_to_right(yaw));
}
