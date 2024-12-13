#include "camera.h"

#include "engine.h"
#include "util/util.h"

void update_camera(camera_s* camera) {
    // only move the camera when cursor is hidden
    if(is_key_pressed(GLFW_KEY_ESCAPE))
            window_set_cursor_visibility(&engine->window, !engine->window.cursor_hidden);
    if(!engine->window.cursor_hidden) return;

    /*
    // move the camera
    v3f movement = V3F(
            is_key_down(GLFW_KEY_D) - is_key_down(GLFW_KEY_A),
            is_key_down(GLFW_KEY_SPACE) - is_key_down(GLFW_KEY_LEFT_SHIFT),
            // flipped since we look out on the -z axis
            is_key_down(GLFW_KEY_S) - is_key_down(GLFW_KEY_W)
        );

    movement = glms_vec3_scale(movement, camera->speed * delta_time() / engine->time_scale);

    v3f forward = yaw_pitch_to_direction(RADIANS(camera->rotation.y), RADIANS(camera->rotation.x));
    v3f right = yaw_to_right(RADIANS(camera->rotation.y));

    camera->position = glms_vec3_add(glms_vec3_scale(right, movement.x), camera->position);
    camera->position = glms_vec3_add(glms_vec3_scale(forward, movement.z), camera->position);

    camera->position.y += movement.y;

    // rotate the camera
    v2f mouse_move = get_mouse_move();
    camera->rotation.x += mouse_move.y * camera->sens * delta_time() / engine->time_scale;
    camera->rotation.y += mouse_move.x * camera->sens * delta_time() / engine->time_scale;
    */

    v3f movement = V3F(
            is_key_down(GLFW_KEY_D) - is_key_down(GLFW_KEY_A),
            is_key_down(GLFW_KEY_W) - is_key_down(GLFW_KEY_S),
            0.0f
        );

    camera->position = V3F_ADD(camera->position, V3F_SCALE(movement, camera->speed * delta_time()));
}

mat4s camera_perspective_projection(camera_s* camera) {
    f32 aspect_ratio = (f32) engine->window.width / (f32) engine->window.height;
    return glms_perspective(camera->fov, aspect_ratio, camera->near_plane, camera->far_plane);
}

mat4s camera_orthographic_projection(camera_s* camera) {
    return glms_ortho(
            -camera->ortho_width * camera->zoom,  camera->ortho_width * camera->zoom,
            -camera->ortho_height * camera->zoom, camera->ortho_height * camera->zoom,
             camera->near_plane,   camera->far_plane
        );
}

mat4s camera_view(camera_s* camera) {
    mat4s view = MAT4_IDENTITY;

    view = glms_rotate_x(view, RADIANS(camera->rotation.x));
    view = glms_rotate_y(view, RADIANS(camera->rotation.y));
    view = glms_rotate_z(view, RADIANS(camera->rotation.z));

    view = glms_translate_x(view, -camera->position.x);
    view = glms_translate_y(view, -camera->position.y);
    view = glms_translate_z(view, -camera->position.z);

    return view;
}

mat4s camera_projection_view(camera_s* camera) {
    return glms_mat4_mul(camera_perspective_projection(camera), camera_view(camera));
}
