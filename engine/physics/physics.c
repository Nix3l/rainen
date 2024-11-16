#include "physics.h"

#include "engine.h"
#include "util/log.h"

// COLLISIONS
// NOTE(nix3l): https://gamedev.stackexchange.com/questions/129446/how-can-i-calculate-the-penetration-depth-between-two-colliding-3d-aabbs
// separating axis theorem
static bool test_axis_penetration(v2f axis, f32 min1, f32 max1, f32 min2, f32 max2, contact_s* contact_info) {
    f32 axis_length_2 = V2F_DOT(axis, axis);
    if(axis_length_2 < EPSILON_f32)
        return false;

    f32 d0 = max2 - min1; // left side
    f32 d1 = max1 - min2; // right side

    // if intervals dont overlap, no intersection
    if(d0 <= 0.0f || d1 <= 0.0f)
        return false;

    // find out which side the overlap is on
    f32 overlap = d0 < d1 ? d0 : -d1;

    v2f separating_axis = V2F_SCALE(axis, overlap / axis_length_2); 
    f32 separating_length_2 = V2F_DOT(separating_axis, separating_axis);

    // use this vector if less than the current minimum translation vector
    if(separating_length_2 < contact_info->dist) {
        contact_info->axis = separating_axis;
        contact_info->dist = separating_length_2;
    }

    return true;
}

aabb_s aabb_create(f32 width, f32 height) {
    f32 hw = width / 2.0f;
    f32 hh = width / 2.0f;
    return (aabb_s) {
        .centre =   V2F_ZERO(),

        .min    =   V2F(-hw, -hh),
        .max    =   V2F( hw,  hh),
        .points = { V2F(-hw,  hh),
                    V2F( hw,  hh),
                    V2F(-hw, -hh),
                    V2F( hw, -hh), },

        .half_extents = V2F(hw, hh),
    };
}

aabb_s aabb_translate(aabb_s box, v2f translation) {
    return (aabb_s) {
        .centre = V2F_ADD(box.centre, translation),
        .min    = V2F_ADD(box.min, translation),
        .max    = V2F_ADD(box.max, translation),
        .points = { V2F_ADD(box.points[0], translation),
                    V2F_ADD(box.points[1], translation),
                    V2F_ADD(box.points[2], translation),
                    V2F_ADD(box.points[3], translation), },
        .half_extents = box.half_extents,
    };
}

contact_s aabb_aabb_penetration_info(aabb_s box1, aabb_s box2) {
    contact_s contact = {
        .axis = V2F_ZERO(),
        .dist = 0.0f,
        .intersection = false,
    };

    // separating axis theorem my beloved
    if(!test_axis_penetration(V2F_UNITX(), box1.min.x, box1.max.x, box2.min.x, box2.max.x, &contact))
        return contact;
    if(!test_axis_penetration(V2F_UNITY(), box1.min.y, box1.max.y, box2.min.y, box2.max.y, &contact))
        return contact;

    contact.intersection = true;

    contact.axis = V2F_NORM(contact.axis);
    contact.dist = sqrtf(contact.dist) * 1.001f;

    return contact;
}

bool aabb_abbb_collision_check(aabb_s box1, aabb_s box2) {
    // it makes sense in my head ok
    return ((box1.min.x > box2.min.x && box1.min.x < box2.max.x) ||
           (box1.max.x < box2.max.x && box1.max.x > box2.min.x)) &&
           ((box1.min.y > box2.min.y && box1.min.y < box2.max.y) ||
           (box1.max.y < box2.max.y && box1.max.y > box2.min.y));
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

// NOTE(nix3l): this should probably be switched out for a different one
//              if the framerate goes too low
static void euler_integrate(entity_s* entity, rigidbody_s* rb, f32 dt) {
    // symplectic euler integration
    // conditionally stable. i.e. (if dt is not too big, system is stable)
    rb->velocity = V2F_ADD(rb->velocity, V2F_SCALE(rb->force, dt * rb->inv_mass));
    entity->position = V2F_ADD(entity->position, V2F_SCALE(rb->velocity, dt));
    rb->force = V2F_ZERO();
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

// TODO(nix3l): broad and narrow phase
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

            if(aabb_abbb_collision_check(aabb_translate(rb1->box, ent1->position), aabb_translate(rb2->box, ent2->position))) {
                LOG("COLLISION!!!\n");
                collision_s* collision = arena_push(ctx->collisions_arena, sizeof(collision_s));
                collision->rb1 = rb1;
                collision->rb2 = rb2;
                linked_list_push(&ctx->collisions_list, collision);
            }
        }
    }
}

static void resolve_collisions(physics_ctx_s* ctx) {
    // TODO(nix3l)
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
