#include "physics.h"

#include "engine.h"
#include "util/log.h"

void init_physics_ctx(physics_ctx_s* ctx, arena_s* physics_objects_arena, u32 physics_objects_capacity) {
    ctx->physics_objects_arena = physics_objects_arena;
    ctx->physics_objects_capacity = physics_objects_capacity;

    ctx->physics_objects_count = 0;
    ctx->first_free_physics_object = 0;
    ctx->physics_objects = arena_push(physics_objects_arena, physics_objects_capacity * sizeof(rigidbody_s));

    ctx->gravity = V2F(0.0f, -9.81f);
}

rigidbody_s* physics_register_entity(physics_ctx_s* ctx, u32 handle) {
    entity_s* entity = entity_data(handle);
    if(entity->state == ENTITY_EMPTY) {
        LOG_ERR("cant add rigidbody to an empty entity\n");
        return NULL;
    }

    rigidbody_s* rigidbody = &ctx->physics_objects[ctx->first_free_physics_object];
    rigidbody->rb_handle = ctx->first_free_physics_object;
    rigidbody->state = PHYSICS_RB_ACTIVE;

    rigidbody->entity_handle = handle;

    rigidbody->mass = 1.0f;
    rigidbody->inv_mass = 1.0f / rigidbody->mass;
    rigidbody->velocity = V2F_ZERO();
    rigidbody->force = V2F_ZERO();

    for(u32 i = ctx->first_free_physics_object + 1; i < ctx->physics_objects_capacity; i ++) {
        if(ctx->physics_objects[i].state == PHYSICS_RB_EMPTY) {
            ctx->first_free_physics_object = i;
            break;
        }
    }

    ctx->physics_objects_count ++;
    entity->rigidbody = rigidbody->rb_handle;
    entity->flags &= ENTITY_HAS_PHYSICS;
    return rigidbody;
}

void physics_remove_entity(physics_ctx_s* ctx, u32 handle) {
    entity_s* entity = entity_data(handle);
    entity->flags &= ~ENTITY_HAS_PHYSICS;

    rigidbody_s* rigidbody = &ctx->physics_objects[entity->rigidbody];
    rigidbody->state = PHYSICS_RB_EMPTY;

    if(rigidbody->rb_handle < ctx->first_free_physics_object)
        ctx->first_free_physics_object = rigidbody->rb_handle;

    ctx->physics_objects_count --;
}

rigidbody_s* get_rigidbody(physics_ctx_s* ctx, u32 handle) {
    return &ctx->physics_objects[handle];
}

void apply_force(rigidbody_s* rigidbody, v2f force) {
    rigidbody->force = V2F_ADD(rigidbody->force, force);
}

static void step_time(entity_s* entity, rigidbody_s* rb, f32 dt) {
    // symplectic euler integration
    // conditionally stable. i.e. if dt is not too big, system is stable
    rb->velocity = V2F_ADD(rb->velocity, V2F_SCALE(rb->force, dt * rb->inv_mass));
    entity->position = V2F_ADD(entity->position, V2F_SCALE(rb->velocity, dt));
    rb->force = V2F_ZERO();
}

void integrate_physics(physics_ctx_s* ctx) {
    f32 delta_time = engine_state->delta_time;

    for(u32 i = 0; i < ctx->physics_objects_capacity; i ++) {
        rigidbody_s* rigidbody = get_rigidbody(ctx, i);
        if(rigidbody->state == PHYSICS_RB_EMPTY) continue;

        entity_s* entity = entity_data(rigidbody->entity_handle);
        step_time(entity, rigidbody, delta_time);
    }
}
