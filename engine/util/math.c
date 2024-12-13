#include "util.h"

mat4s get_transformation_matrix(v2f position, f32 rotation, v2f scale) {
    mat4s transformation = glms_translate(MAT4_IDENTITY, V3F(position.x, position.y, 0.0f));
    transformation = glms_rotate(transformation, rotation, V3F(0.0f, 0.0f, 1.0f));
    transformation = glms_scale(transformation, V3F(scale.x, scale.y, 1.0f));
    return transformation;
}
