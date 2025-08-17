#include "math_util.h"

mat4s model_matrix_new(v3f pos, v3f rot, v3f scale) {
    mat4s model = mat4_IDENTITY;

    model = glms_translate(model, pos);
    model = glms_rotate_x(model, RADIANS(rot.x));
    model = glms_rotate_y(model, RADIANS(rot.y));
    model = glms_rotate_z(model, RADIANS(rot.z));
    model = glms_scale(model, scale);

    return model;
}

mat4s view_matrix_new(v3f pos, v3f rot) {
    mat4s view = mat4_IDENTITY;

    view = glms_rotate_x(view, RADIANS(rot.x));
    view = glms_rotate_y(view, RADIANS(rot.y));
    view = glms_rotate_z(view, RADIANS(rot.z));
    view = glms_translate_x(view, -pos.x);
    view = glms_translate_y(view, -pos.y);
    view = glms_translate_z(view, -pos.z);

    return view;
}

mat4s proj_perspective_matrix_new(f32 fov, f32 aspect_ratio, f32 near, f32 far) {
    return glms_perspective_rh_no(fov, aspect_ratio, near, far);
}

mat4s proj_ortho_matrix_new(f32 w, f32 h, f32 near, f32 far) {
    return glms_ortho_rh_no(-w/2.0f, w/2.0f, -h/2.0f, h/2.0f, near, far);
}
