#include "physics.h"

#include "engine.h"
#include "util/log.h"

void init_physics_ctx(physics_ctx_s* ctx, arena_s* physics_objects_arena, u32 physics_objects_capacity) {
    ctx->physics_objects_arena = physics_objects_arena;
    ctx->physics_objects = create_compact_list(physics_objects_arena, sizeof(rigidbody_s), physics_objects_capacity);

    ctx->gravity = V2F(0.0f, -9.81f);
}

rigidbody_s* physics_register_entity(physics_ctx_s* ctx, u32 entity_handle) {
    entity_s* entity = entity_data(entity_handle);
    if(!entity) {
        LOG_ERR("cant add rigidbody to a non-existent entity\n");
        return NULL;
    }

    u32 slot_taken;
    rigidbody_s* rigidbody = compact_list_push(&ctx->physics_objects, &slot_taken);
    rigidbody->rb_handle = slot_taken;
    rigidbody->entity_handle = entity_handle;

    rigidbody->mass = 1.0f;
    rigidbody->inv_mass = 1.0f / rigidbody->mass;
    rigidbody->velocity = V2F_ZERO();
    rigidbody->force = V2F_ZERO();

    entity->rigidbody = rigidbody->rb_handle;
    entity->flags &= ENTITY_HAS_PHYSICS;
    return rigidbody;
}

void physics_remove_entity(physics_ctx_s* ctx, u32 handle) {
    entity_s* entity = entity_data(handle);
    entity->flags &= ~ENTITY_HAS_PHYSICS;
    compact_list_remove(&ctx->physics_objects, handle);
}

rigidbody_s* get_rigidbody(physics_ctx_s* ctx, u32 handle) {
    return compact_list_get(&ctx->physics_objects, handle);
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

    for(u32 i = 0; i < ctx->physics_objects.capacity; i ++) {
        rigidbody_s* rigidbody = get_rigidbody(ctx, i);
        if(!rigidbody) continue;

        entity_s* entity = entity_data(rigidbody->entity_handle);
        step_time(entity, rigidbody, delta_time);
    }
}
