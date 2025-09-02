#ifndef _BASE_MACROS_H
#define _BASE_MACROS_H

// use this to tag functions that should **only** be used in the development stage
#define DEVONLY

#include <stdio.h>
#if !defined(ASSERT_BREAK)
#   define ASSERT_BREAK(_x) do { fprintf(stderr, "assertion `%s` failed [%s:%u]\n", #_x, __FILE__, __LINE__); /**(int*)0=0*/exit(1); } while(0) // crash (ouch)
#endif

// use this to remove compiler warnings for unused parameters
// only use when the parameter can not be removed
#define UNUSED(_x) do { (void) (_x); } while(0)

#define ASSERT(_x) do { if(!(_x)) ASSERT_BREAK((_x)); } while(0)
#define PANIC(...) do { fprintf(stderr, "[%s:%u]: !!! PANIC !!!\n => ", __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); exit(1); } while(0)

#define UNREACHABLE do { fprintf(stderr, "ERR (%s:[%s]:%u) unreachable branch\n", __FILE__, __PRETTY_FUNCTION__, __LINE__); exit(1); } while(0);

#define STRINGIFY(_m) (#_m)

#define ARRAY_SIZE(_arr) (sizeof((_arr))/(sizeof(*(_arr))))

#define KILOBYTES(_x) (1024*(_x))
#define MEGABYTES(_x) (1024*KILOBYTES((_x)))
#define GIGABYTES(_x) (1024*MEGABYTES((_x)))

#define INT_FROM_PTR(_ptr) (unsigned long long)((char*)_ptr-(char*)0)
#define PTR_FROM_INT(_int) (void*)((char*)0 + (_int))

// macros here so we dont have to have separate functions for each data type
// so can be used with floats, doubles, all types of ints, etc
#define MIN(_a, _b) ((_a)<(_b)?(_a):(_b))
#define MAX(_a, _b) ((_a)>(_b)?(_a):(_b))
#define CLAMP(_x, _min, _max) ((_x)<(_min)?(_min):((_x)>(_max)?(_max):(_x)))
#define CLAMP_MAX(_x, _max) MIN(_x, _max)
#define CLAMP_MIN(_x, _min) MAX(_x, _min)

#define SIGN(_x) ((_x)<0?:-1:1)

#define v2f_new(_x, _y) (v2f) { .x = (_x), .y = (_y) }
#define v3f_new(_x, _y, _z) (v3f) { .x = (_x), .y = (_y), .z = (_z) }
#define v4f_new(_x, _y, _z, _w) (v4f) { .x = (_x), .y = (_y), .z = (_z), .w = (_w) }

#define v2f_ZERO v2f_new(0.0f, 0.0f)
#define v3f_ZERO v3f_new(0.0f, 0.0f, 0.0f)
#define v4f_ZERO v4f_new(0.0f, 0.0f, 0.0f, 0.0f)

#define v2f_ONE v2f_new(1.0f, 1.0f)
#define v3f_ONE v3f_new(1.0f, 1.0f, 1.0f)
#define v4f_ONE v4f_new(1.0f, 1.0f, 1.0f, 1.0f)

#define v2i_new(_x, _y) (v2i) { .x = (_x), .y = (_y) }
#define v3i_new(_x, _y, _z) (v3i) { .x = (_x), .y = (_y), .z = (_z) }
#define v4i_new(_x, _y, _z, _w) (v4i) { .x = (_x), .y = (_y), .z = (_z), .w = (_w) }

#define v2i_ZERO v2i_new(0, 0)
#define v3i_ZERO v3i_new(0, 0, 0)
#define v4i_ZERO v4i_new(0, 0, 0, 0)

#define v2i_ONE v2i_new(1, 1)
#define v3i_ONE v3i_new(1, 1, 1)
#define v4i_ONE v4i_new(1, 1, 1, 1)

#define v3f_rgb(_r, _g, _b) v3f_new((_r) / 255.0f, (_g) / 255.0f, (_b) / 255.0f)
#define v4f_rgba(_r, _g, _b, _a) v3f_new((_r) / 255.0f, (_g) / 255.0f, (_b) / 255.0f, (_a) / 255.0f)

#define v2f_add(_x, _y) (glms_vec2_add(_x, _y))
#define v3f_add(_x, _y) (glms_vec3_add(_x, _y))
#define v4f_add(_x, _y) (glms_vec4_add(_x, _y))

#define v2f_sub(_x, _y) (glms_vec2_sub(_x, _y))
#define v3f_sub(_x, _y) (glms_vec3_sub(_x, _y))
#define v4f_sub(_x, _y) (glms_vec4_sub(_x, _y))

#define v2f_mul(_x, _y) (glms_vec2_mul(_x, _y))
#define v3f_mul(_x, _y) (glms_vec3_mul(_x, _y))
#define v4f_mul(_x, _y) (glms_vec4_mul(_x, _y))

#define v2f_div(_x, _y) (glms_vec2_div(_x, _y))
#define v3f_div(_x, _y) (glms_vec3_div(_x, _y))
#define v4f_div(_x, _y) (glms_vec4_div(_x, _y))

#define v2f_scale(_x, _y) (glms_vec2_scale(_x, _y))
#define v3f_scale(_x, _y) (glms_vec3_scale(_x, _y))
#define v4f_scale(_x, _y) (glms_vec4_scale(_x, _y))

#define v2f_dot(_x, _y) (glms_vec2_dot(_x, _y))
#define v3f_dot(_x, _y) (glms_vec3_dot(_x, _y))
#define v4f_dot(_x, _y) (glms_vec4_dot(_x, _y))

#define v2f_cross(_x, _y) (glms_vec2_cross(_x, _y))
#define v3f_cross(_x, _y) (glms_vec3_cross(_x, _y))
#define v4f_cross(_x, _y) (glms_vec4_cross(_x, _y))

#define v2f_norm(_x) (glms_vec2_normalize(_x))
#define v3f_norm(_x) (glms_vec3_normalize(_x))
#define v4f_norm(_x) (glms_vec4_normalize(_x))

#define v2f_expand(_x) (_x).x, (_x).y
#define v3f_expand(_x) (_x).x, (_x).y, (_x).z
#define v4f_expand(_x) (_x).x, (_x).y, (_x).z, (_x).w

#define v2i_add(_x, _y) (glms_ivec2_add(_x, _y))
#define v3i_add(_x, _y) (glms_ivec3_add(_x, _y))
#define v4i_add(_x, _y) (glms_ivec4_add(_x, _y))

#define v2i_sub(_x, _y) (glms_ivec2_sub(_x, _y))
#define v3i_sub(_x, _y) (glms_ivec3_sub(_x, _y))
#define v4i_sub(_x, _y) (glms_ivec4_sub(_x, _y))

#define v2i_mul(_x, _y) (glms_ivec2_mul(_x, _y))
#define v3i_mul(_x, _y) (glms_ivec3_mul(_x, _y))
#define v4i_mul(_x, _y) (glms_ivec4_mul(_x, _y))

#define v2i_div(_x, _y) (glms_ivec2_div(_x, _y))
#define v3i_div(_x, _y) (glms_ivec3_div(_x, _y))
#define v4i_div(_x, _y) (glms_ivec4_div(_x, _y))

#define v2i_scale(_x, _y) (glms_ivec2_scale(_x, _y))
#define v3i_scale(_x, _y) (glms_ivec3_scale(_x, _y))
#define v4i_scale(_x, _y) (glms_ivec4_scale(_x, _y))

#define v2i_dot(_x, _y) (glms_ivec2_dot(_x, _y))
#define v3i_dot(_x, _y) (glms_ivec3_dot(_x, _y))
#define v4i_dot(_x, _y) (glms_ivec4_dot(_x, _y))

#define v2i_norm(_x) (glms_ivec2_normalize(_x))
#define v3i_norm(_x) (glms_ivec3_normalize(_x))
#define v4i_norm(_x) (glms_ivec4_normalize(_x))

#define v2f_expand(_x) (_x).x, (_x).y
#define v3f_expand(_x) (_x).x, (_x).y, (_x).z
#define v4f_expand(_x) (_x).x, (_x).y, (_x).z, (_x).w

// yes these do the same as the float version but i like symmetry
#define v2i_expand(_x) (_x).x, (_x).y
#define v3i_expand(_x) (_x).x, (_x).y, (_x).z
#define v4i_expand(_x) (_x).x, (_x).y, (_x).z, (_x).w

#define v2f_UNITX (v2f_new(1.0f, 0.0f))
#define v2f_UNITY (v2f_new(0.0f, 1.0f))

#define v3f_UNITX (v3f_new(1.0f, 0.0f, 0.0f))
#define v3f_UNITY (v3f_new(0.0f, 1.0f, 0.0f))
#define v3f_UNITZ (v3f_new(0.0f, 0.0f, 1.0f))

#define v4f_UNITX (v4f_new(1.0f, 0.0f, 0.0f, 0.0f))
#define v4f_UNITY (v4f_new(0.0f, 1.0f, 0.0f, 0.0f))
#define v4f_UNITZ (v4f_new(0.0f, 0.0f, 1.0f, 0.0f))
#define v4f_UNITW (v4f_new(0.0f, 0.0f, 0.0f, 1.0f))

#define mat3_IDENTITY (glms_mat3_identity())
#define mat4_IDENTITY (glms_mat4_identity())

#define RADIANS(_x) (glm_rad((_x)))
#define DEGREES(_x) (glm_deg((_x)))

#endif /* ifndef _BASE_MACROS_H */
