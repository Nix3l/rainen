#ifndef PHYSICS_H
#define PHYSICS_H

#include "base.h"
#include "memory/memory.h"

// NOTE(nix3l): really good resource https://phys-sim-book.github.io/preface.html

typedef struct {
    u32 rb_handle;

    u32 entity_handle;

    f32 mass;
    f32 inv_mass;

    v2f velocity;
    v2f force;
} rigidbody_s;

typedef struct {
    arena_s* physics_objects_arena; // MUST be zero'd
    compact_list_s physics_objects; 

    v2f gravity;
} physics_ctx_s;

void init_physics_ctx(physics_ctx_s* ctx, arena_s* physics_objects_arena, u32 physics_objects_capacity);

rigidbody_s* physics_register_entity(physics_ctx_s* ctx, u32 handle);
void physics_remove_entity(physics_ctx_s* ctx, u32 handle);

rigidbody_s* get_rigidbody(physics_ctx_s* ctx, u32 rb_handle);

void apply_force(rigidbody_s* rigidbody, v2f force);

void integrate_physics(physics_ctx_s* ctx); 

#endif
