#ifndef PHYSICS_H
#define PHYSICS_H

#include "base.h"
#include "memory/memory.h"

// NOTE(nix3l): really good resources
// => 
// TODO

typedef struct {
    v2f centre; // position
    v2f min;
    v2f max;
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

// TODO(nix3l): switch these out to use handles
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
// move the centre point and min/max points of a box
aabb_s aabb_translate(aabb_s box, v2f translation);
// returns the half width and half height of a box
v2f aabb_extents(aabb_s box);
// makes sure the min and max are in the correct order
aabb_s aabb_fix(aabb_s box);
// keeps the centre of the first box and sweeps the second box on top
aabb_s aabb_minkowski_sum(aabb_s box1, aabb_s box2);
aabb_s aabb_minkowski_diff(aabb_s box1, aabb_s box2);
bool aabb_point_intersection_check(aabb_s box, v2f point);
bool aabb_abbb_intersection_check(aabb_s box1, aabb_s box2);
contact_s aabb_aabb_penetration_info(aabb_s box1, aabb_s box2);

// PHYSICS
void init_physics_ctx(physics_ctx_s* ctx, arena_s* physics_objects_arena, u32 physics_objects_capacity, arena_s* collisions_arena);

rigidbody_s* physics_register_entity(physics_ctx_s* ctx, u32 handle);
void physics_remove_entity(physics_ctx_s* ctx, u32 handle);

rigidbody_s* get_rigidbody(physics_ctx_s* ctx, u32 rb_handle);

void apply_force(rigidbody_s* rigidbody, v2f force);

void process_physics(physics_ctx_s* ctx);

#endif
