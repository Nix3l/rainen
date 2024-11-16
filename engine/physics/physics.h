#ifndef PHYSICS_H
#define PHYSICS_H

#include "base.h"
#include "memory/memory.h"

// NOTE(nix3l): really good resources
// => https://phys-sim-book.github.io/preface.html
// TODO

typedef struct {
    v2f centre; // position

    v2f min;
    v2f max;
    v2f points[4]; // top left, top right, bottom left, bottom right
    v2f half_extents;
} aabb_s;

typedef struct {
    u32 rb_handle;
    u32 entity_handle;

    aabb_s box;

    f32 mass;
    f32 inv_mass;

    v2f velocity;
    v2f force;
} rigidbody_s;

typedef struct {
    v2f axis;
    f32 dist;
    bool intersection;
} contact_s;

typedef struct {
    rigidbody_s* rb1;
    rigidbody_s* rb2;
} collision_s;

typedef struct {
    arena_s* physics_objects_arena; // MUST be zero'd
    compact_list_s physics_objects; 

    arena_s* collisions_arena;
    linked_list_s collisions_list;

    v2f gravity;
} physics_ctx_s;

// COLLISIONS
aabb_s aabb_create(f32 width, f32 height);
aabb_s aabb_translate(aabb_s box, v2f translation);
bool aabb_abbb_collision_check(aabb_s box1, aabb_s box2);
contact_s aabb_aabb_penetration_info(aabb_s box1, aabb_s box2);

// PHYSICS
void init_physics_ctx(physics_ctx_s* ctx, arena_s* physics_objects_arena, u32 physics_objects_capacity, arena_s* collisions_arena);

rigidbody_s* physics_register_entity(physics_ctx_s* ctx, u32 handle);
void physics_remove_entity(physics_ctx_s* ctx, u32 handle);

rigidbody_s* get_rigidbody(physics_ctx_s* ctx, u32 rb_handle);

void apply_force(rigidbody_s* rigidbody, v2f force);

void process_physics(physics_ctx_s* ctx);

#endif
