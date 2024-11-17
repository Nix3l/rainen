#include "physics.h"

#include "engine.h"
#include "util/log.h"

// COLLISIONS
aabb_s aabb_create(f32 width, f32 height) {
    f32 hw = width / 2.0f;
    f32 hh = height / 2.0f;
    return (aabb_s) {
        .centre =   V2F_ZERO,
        .min    =   V2F(-hw, -hh),
        .max    =   V2F( hw,  hh),
    };
}

aabb_s aabb_translate(aabb_s box, v2f translation) {
    return (aabb_s) {
        .centre = V2F_ADD(box.centre, translation),
        .min    = V2F_ADD(box.min, translation),
        .max    = V2F_ADD(box.max, translation),
    };
}

aabb_s aabb_minkowski_sum(aabb_s box1, aabb_s box2) {
    return (aabb_s) {
        .centre = box1.centre,
        .min    = V2F_ADD(box1.min, box2.min),
        .max    = V2F_ADD(box1.max, box2.max),
    };
}

aabb_s aabb_minkowski_diff(aabb_s box1, aabb_s box2) {
    return (aabb_s) {
        .centre = V2F_SUB(box1.centre, box2.centre),
        .min    = V2F_SUB(box1.min, box2.min),
        .max    = V2F_SUB(box1.max, box2.max),
    };
}

bool aabb_point_intersection_check(aabb_s box, v2f point) {
    return point.x >= box.min.x && point.x <= box.max.x &&
           point.y >= box.min.y && point.y <= box.max.y;
}

bool aabb_abbb_intersection_check(aabb_s box1, aabb_s box2) {
    // it makes sense in my head ok
    return ((box1.min.x > box2.min.x && box1.min.x < box2.max.x) ||
           (box1.max.x < box2.max.x && box1.max.x > box2.min.x)) &&
           ((box1.min.y > box2.min.y && box1.min.y < box2.max.y) ||
           (box1.max.y < box2.max.y && box1.max.y > box2.min.y));
}

contact_s aabb_aabb_penetration_info(aabb_s box1, aabb_s box2) {
    aabb_s minkowski = aabb_minkowski_diff(box1, box2);
    contact_s contact = {
        .axis = V2F_ZERO,
        .dist = 0.0f,
        // .intersection = false,
        .intersection = true,
    };

    if(!aabb_point_intersection_check(minkowski, V2F_ZERO))
        return contact;
    else
        contact.intersection = true;

    contact.dist = fabsf(minkowski.min.x);
    contact.axis = V2F(-1.0f, 0.0f);

    if(fabsf(minkowski.max.x) < contact.dist) {
        contact.dist = fabsf(minkowski.max.y);
        contact.axis = V2F(1.0f, 0.0f);
    }

    if(fabsf(minkowski.min.y) < contact.dist) {
        contact.dist = fabsf(minkowski.min.y);
        contact.axis = V2F(0.0f, -1.0f);
    }

    if(fabsf(minkowski.max.y) < contact.dist) {
        contact.dist = fabsf(minkowski.max.y);
        contact.axis = V2F(0.0f, 1.0f);
    }

    return contact;
}

// PHYSICS
void init_physics_ctx(physics_ctx_s* ctx, arena_s* physics_objects_arena, u32 physics_objects_capacity, arena_s* collisions_arena) {
    ctx->physics_objects_arena = physics_objects_arena;
    ctx->physics_objects = create_compact_list(physics_objects_arena, sizeof(rigidbody_s), physics_objects_capacity);
    ctx->collisions_arena = collisions_arena;
    ctx->collisions_list = create_linked_list(&engine_state->frame_arena);

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
    rigidbody->velocity = V2F_ZERO;
    rigidbody->force = V2F_ZERO;

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

// NOTE(nix3l): this should probably be switched out for a different one
//              if the framerate goes too low
static void euler_integrate(entity_s* entity, rigidbody_s* rb, f32 dt) {
    // symplectic euler integration
    // conditionally stable. i.e. (if dt is not too big, system is stable)
    rb->velocity = V2F_ADD(rb->velocity, V2F_SCALE(rb->force, dt * rb->inv_mass));
    entity->position = V2F_ADD(entity->position, V2F_SCALE(rb->velocity, dt));
    rb->force = V2F_ZERO;
    // move the aabb to the new entity position
    rb->box = aabb_translate(rb->box, V2F_SUB(entity->position, rb->box.centre));
}

static void integrate_objects(physics_ctx_s* ctx) {
    f32 delta_time = engine_state->delta_time;

    for(u32 i = 0; i < ctx->physics_objects.capacity; i ++) {
        rigidbody_s* rigidbody = get_rigidbody(ctx, i);
        if(!rigidbody) continue;

        entity_s* entity = entity_data(rigidbody->entity_handle);
        euler_integrate(entity, rigidbody, delta_time);
    }
}

// TODO(nix3l): have this be the broad phase
static void DEBUGdetect_collisions(physics_ctx_s* ctx) {
    // TODO(nix3l): this is very horrible. just screams exponential growth
    //              also creates duplicate collisions
    for(u32 i = 0; i < ctx->physics_objects.capacity; i ++) {
        rigidbody_s* rb1 = get_rigidbody(ctx, i);
        if(!rb1) continue;
        for(u32 j = 0; j < ctx->physics_objects.capacity; j ++) {
            if(i == j) continue;
            rigidbody_s* rb2 = get_rigidbody(ctx, j);
            if(!rb2) continue;

            entity_s* ent1 = entity_data(rb1->entity_handle);
            entity_s* ent2 = entity_data(rb2->entity_handle);

            if(aabb_abbb_intersection_check(rb1->box, rb2->box)) {
                collision_s* collision = arena_push(ctx->collisions_arena, sizeof(collision_s));
                collision->rb1 = rb1;
                collision->rb2 = rb2;
                linked_list_push(&ctx->collisions_list, collision);
            }
        }
    }
}

// TODO(nix3l): should include both the narrow phase AND collision resolution
static void resolve_collisions(physics_ctx_s* ctx) {
    if(ctx->collisions_list.count == 0) return;
    linked_list_element_s* iterator = ctx->collisions_list.first;

    for(u32 i = 0; i < ctx->collisions_list.count; i ++) {
        if(!iterator) continue;

        collision_s* collision = iterator->contents;
        if(!collision) continue;

        contact_s contact = aabb_aabb_penetration_info(collision->rb1->box, collision->rb2->box);
        if(contact.intersection) {
            // TODO(nix3l): resolve collision
            // LOG("a: [%.2f, %.2f], d: %.2f, %c\n", contact.axis.x, contact.axis.y, contact.dist, contact.intersection ? 'Y' : 'N');
        } else {
            // LOG("idk\n");
        }

        iterator = iterator->next;
    }
}

static void flush_collisions(physics_ctx_s* ctx) {
    linked_list_clear(&ctx->collisions_list);
}

void process_physics(physics_ctx_s* ctx) {
    // integrate all objects
    integrate_objects(ctx);
    // detect all collisions between objects
    DEBUGdetect_collisions(ctx);
    // resolve all collisions
    resolve_collisions(ctx);
    // reset collision state
    flush_collisions(ctx);
}
