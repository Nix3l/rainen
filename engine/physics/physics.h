#ifndef PHYSICS_H
#define PHYSICS_H

#include "base.h"
#include "memory/memory.h"

// TODO(nix3l): this whole thing is a huge mess
// => change up the collision detection handling, too complicated at the moment
// => add iterations for more accurate

typedef struct {
    v2f centre; // position
    v2f min;
    v2f max;
} aabb_s;

typedef struct {
    u32 handle;

    aabb_s box;
} static_collider_s;

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
    v2f penetration;
    f32 dist;
    bool intersection;
} contact_s;

typedef struct {
    bool intersection;

    f32 t; // [0-1] percentage along the ray at which the intersection occurred
    v2f point;
    v2f normal;
} ray_hit_s;

typedef struct {
    arena_s* physics_objects_arena; // MUST be zero'd
    compact_list_s physics_objects; 

    arena_s* static_objects_arena; // MUST be zero'd
    compact_list_s static_objects;
} physics_ctx_s;

// COLLISIONS
aabb_s aabb_create_dimensions(f32 width, f32 height);
// set the centre point and adjust min/max points accordingly
aabb_s aabb_centre(aabb_s box, v2f centre);
// move the centre point and min/max points
aabb_s aabb_translate(aabb_s box, v2f translation);
// returns the width and height of a box
v2f aabb_size(aabb_s box);
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
ray_hit_s ray_hit_aabb(v2f origin, v2f magnitude, aabb_s box);

// PHYSICS
void init_physics_ctx(physics_ctx_s* ctx, arena_s* physics_objects_arena, u32 physics_objects_capacity, arena_s* static_colliders_arena, u32 static_colliders_capacity);

rigidbody_s* physics_register_entity(physics_ctx_s* ctx, u32 handle);
void physics_remove_entity(physics_ctx_s* ctx, u32 handle);

static_collider_s* physics_register_static_collider(physics_ctx_s* ctx, aabb_s box);
void physics_remove_static_collider(physics_ctx_s* ctx, u32 handle);

rigidbody_s* get_rigidbody(physics_ctx_s* ctx, u32 rb_handle);

void apply_force(rigidbody_s* rigidbody, v2f force);

void process_physics(physics_ctx_s* ctx);

#endif
