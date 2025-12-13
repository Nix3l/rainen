#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "base.h"
#include "memory/memory.h"
#include "bounds.h"

enum {
    PHYS_MAX_OBJS = 256,
};

typedef struct { handle_t id; } collider_t;

typedef enum collider_tags_t {
    COLLIDER_TAGS_NONE    = 0x00,
    COLLIDER_TAGS_STATIC  = 0x01,
    COLLIDER_TAGS_NO_GRAV = 0x02,
} collider_tags_t;

typedef enum collider_type_t {
    COLLIDER_SHAPE_INVALID = 0,
    COLLIDER_SHAPE_AABB,
    // TODO(nix3l):
    // COLLIDER_SHAPE_CIRCLE,
    // COLLIDER_SHAPE_TRIANGLE,
    // COLLIDER_SHAPE_CAPSULE,
} collider_type_t;

typedef struct collider_shape_t {
    collider_type_t type;

    union {
        aabb_t box;
    };
} collider_shape_t;

typedef struct rigidbody_t {
    collider_tags_t tags;
    collider_shape_t bounds;
    v2f pos;
    v2f vel;
    v2f acc;
    v2f force;
    f32 mass;
    f32 inv_m;
    f32 restitution;
    f32 friction;
} rigidbody_t;

typedef struct collider_info_t {
    collider_tags_t tags;
    collider_shape_t bounds;
    v2f pos;
    f32 mass;
    f32 restitution;
    f32 friction;
} collider_info_t;

collider_t collider_new(collider_info_t info);
void collider_destroy(collider_t collider);

rigidbody_t* collider_get_data(collider_t collider);

void collider_apply_force(collider_t collider, v2f force);

typedef struct manifold_point_t {
    v2f pos;
    v2f fromA;
    v2f fromB;
    f32 penetration;
    f32 normal_impulse;
    f32 tangent_impulse;
} manifold_point_t;

typedef struct manifold_t {
    collider_t collA;
    collider_t collB;
    rigidbody_t* rbA;
    rigidbody_t* rbB;

    u32 point_count;
    manifold_point_t points[2];

    v2f normal;
    v2f tangent;
} manifold_t;

// TODO: broad phase

typedef struct narrow_phase_t {
    vector_t pairs;
} narrow_phase_t;

typedef struct physics_ctx_t {
    arena_t rations;
    pool_t obj_pool;
    arena_t frame_arena;

    u32 substeps;

    narrow_phase_t narrow;

    v2f global_gravity;
} physics_ctx_t;

extern physics_ctx_t physics_ctx;

void physics_init();
void physics_terminate();

void physics_update(f32 dt);

#endif
