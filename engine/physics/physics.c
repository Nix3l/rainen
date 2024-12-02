#include "physics.h"

#include "engine.h"
#include "util/log.h"

// COLLISIONS
aabb_s aabb_create_dimensions(f32 width, f32 height) {
    f32 hw = width / 2.0f;
    f32 hh = height / 2.0f;
    return (aabb_s) {
        .centre = V2F_ZERO,
        .min    = V2F(-hw, -hh),
        .max    = V2F( hw,  hh),
    };
}

aabb_s aabb_centre(aabb_s box, v2f centre) {
    v2f diff = V2F_SUB(centre, box.centre);
    return (aabb_s) {
        .centre = centre,
        .min = V2F_ADD(box.min, diff),
        .max = V2F_ADD(box.max, diff),
    };
}

aabb_s aabb_translate(aabb_s box, v2f translation) {
    return (aabb_s) {
        .centre = V2F_ADD(box.centre, translation),
        .min    = V2F_ADD(box.min, translation),
        .max    = V2F_ADD(box.max, translation),
    };
}

v2f aabb_size(aabb_s box) {
    return V2F_SUB(box.max, box.min);
}

v2f aabb_extents(aabb_s box) {
    return V2F((box.max.x - box.min.x) / 2.0f,
               (box.max.y - box.min.y) / 2.0f);
}

aabb_s aabb_fix(aabb_s box) {
    return (aabb_s) {
        box.centre,
        V2F(MIN(box.min.x, box.max.x), MIN(box.min.y, box.max.y)),
        V2F(MAX(box.min.x, box.max.x), MAX(box.min.y, box.max.y)),
    };
}

aabb_s aabb_minkowski_sum(aabb_s box1, aabb_s box2) {
    return aabb_fix((aabb_s) {
        .centre = box1.centre,
        .min    = V2F_ADD(box1.min, box2.min),
        .max    = V2F_ADD(box1.max, box2.max),
    });
}

aabb_s aabb_minkowski_diff(aabb_s box1, aabb_s box2) {
    return aabb_fix((aabb_s) {
        .centre = V2F_SUB(box1.centre, box2.centre),
        .min    = V2F_ADD(box1.min, box2.min),
        .max    = V2F_ADD(box1.max, box2.max),
    });
}

bool aabb_point_intersection_check(aabb_s box, v2f point) {
    return point.x >= box.min.x && point.x <= box.max.x &&
           point.y >= box.min.y && point.y <= box.max.y;
}

bool aabb_abbb_intersection_check(aabb_s box1, aabb_s box2) {
    // it makes sense in my head ok
    return ((box1.min.x >= box2.min.x && box1.min.x <= box2.max.x) ||
           (box1.max.x <= box2.max.x && box1.max.x >= box2.min.x)) &&
           ((box1.min.y >= box2.min.y && box1.min.y <= box2.max.y) ||
           (box1.max.y <= box2.max.y && box1.max.y >= box2.min.y));
}

contact_s aabb_aabb_penetration_info(aabb_s box1, aabb_s box2) {
    aabb_s minkowski = aabb_minkowski_diff(box1, box2);
    contact_s contact = {
        .dist = MAX_f32,
        .penetration = V2F_ZERO,
        .intersection = false,
    };

    if(!aabb_point_intersection_check(minkowski, V2F_ZERO))
        return contact;
    else
        contact.intersection = true;

    if(fabsf(minkowski.min.x) < contact.dist) {
        contact.dist = fabsf(minkowski.min.x);
        contact.penetration = V2F(minkowski.min.x, 0.0f);
    }

    if(fabsf(minkowski.max.x) < contact.dist) {
        contact.dist = fabsf(minkowski.max.x);
        contact.penetration = V2F(minkowski.max.x, 0.0f);
    }

    if(fabsf(minkowski.min.y) < contact.dist) {
        contact.dist = fabsf(minkowski.min.y);
        contact.penetration = V2F(0.0f, minkowski.min.y);
    }

    if(fabsf(minkowski.max.y) < contact.dist) {
        contact.dist = fabsf(minkowski.max.y);
        contact.penetration = V2F(0.0f, minkowski.max.y);
    }

    return contact;
}

ray_hit_s ray_hit_aabb(v2f origin, v2f magnitude, aabb_s box) {
    ray_hit_s hit = (ray_hit_s) {
        .intersection = false,
        .t = 0.0f,

        .point = V2F_ZERO,
        .normal = V2F_ZERO,
    };

    f32 last_entry = -MAX_f32;
    f32 first_exit =  MAX_f32;

    for(u32 i = 0; i < 2; i ++) {
        if(magnitude.raw[i] != 0.0f) {
            f32 t1 = (box.min.raw[i] - origin.raw[i]) / magnitude.raw[i];
            f32 t2 = (box.max.raw[i] - origin.raw[i]) / magnitude.raw[i];

            last_entry = MAX(last_entry, MIN(t1, t2));
            first_exit = MIN(first_exit, MAX(t1, t2));
        } else if(origin.raw[i] <= box.min.raw[i] || origin.raw[i] >= box.max.raw[i]) {
            return hit;
        }
    }

    if(first_exit > last_entry && first_exit > 0.0f && last_entry < 1.0f) {
        hit.intersection = true;
        hit.t = last_entry;

        hit.point.x = origin.x + magnitude.x * last_entry;
        hit.point.y = origin.y + magnitude.y * last_entry;

        f32 dx = hit.point.x - box.centre.x;
        f32 dy = hit.point.y - box.centre.y;

        v2f extents = aabb_extents(box);
        f32 px = extents.x - fabsf(dx);
        f32 py = extents.y - fabsf(dy);

        if(px < py) hit.normal.x = (dx > 0.0f) - (dx < 0.0f);
        else hit.normal.y = (dy > 0.0f) - (dy < 0.0f);
    }

    return hit;
}

// PHYSICS
void init_physics_ctx(physics_ctx_s* ctx, arena_s* physics_objects_arena, u32 physics_objects_capacity, arena_s* static_colliders_arena, u32 static_colliders_capacity) {
    ctx->physics_objects_arena = physics_objects_arena;
    ctx->physics_objects = create_compact_list(physics_objects_arena, sizeof(rigidbody_s), physics_objects_capacity);

    ctx->static_objects_arena = static_colliders_arena;
    ctx->static_objects = create_compact_list(static_colliders_arena, sizeof(static_collider_s), static_colliders_capacity);
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

static_collider_s* physics_register_static_collider(physics_ctx_s* ctx, aabb_s box) {
    u32 slot_taken;
    static_collider_s* collider = compact_list_push(&ctx->static_objects, &slot_taken);
    collider->handle = slot_taken;
    collider->box = box;
    return collider;
}

void physics_remove_static_collider(physics_ctx_s* ctx, u32 handle) {
    compact_list_remove(&ctx->static_objects, handle);
}

rigidbody_s* get_rigidbody(physics_ctx_s* ctx, u32 handle) {
    return compact_list_get(&ctx->physics_objects, handle);
}

void apply_force(rigidbody_s* rigidbody, v2f force) {
    rigidbody->force = V2F_ADD(rigidbody->force, force);
}

// NOTE(nix3l): this should probably be switched out for a different one
//              if the framerate goes too low
static void euler_integrate(rigidbody_s* rb, f32 dt) {
    // symplectic (semi-implicit) euler integration
    // conditionally stable. i.e. (if dt is not too big, system is stable)
    rb->velocity = V2F_ADD(rb->velocity, V2F_SCALE(rb->force, dt * rb->inv_mass));
    rb->box = aabb_translate(rb->box, V2F_SCALE(rb->velocity, dt));
    rb->force = V2F_ZERO;
}

static void resolve_static_collisions(physics_ctx_s* ctx, rigidbody_s* rb) {
    ray_hit_s closest_intersection = { .intersection = false, .t = MAX_f32, };
    for(u32 j = 0; j < ctx->static_objects.count; j ++) {
        static_collider_s* collider = compact_list_get(&ctx->static_objects, j);
        if(!collider) continue;

        render_debug_rect(collider->box.centre, aabb_size(collider->box), COL_RED);

        contact_s contact = aabb_aabb_penetration_info(rb->box, collider->box);
        /*
        if(contact.intersection)
            rb->box = aabb_translate(rb->box, V2F_SCALE(contact.penetration, -1.0f));
        */

        aabb_s minkowski_sum = aabb_minkowski_sum(collider->box, rb->box);
        ray_hit_s hit = ray_hit_aabb(rb->box.centre, rb->velocity, minkowski_sum);
        render_debug_point(hit.point, 12.0f, COL_WHITE);
        LOG("collision [%d], t [%.2f], p [%.2f, %.2f]\n", hit.intersection ? 1 : 0, hit.t, V2F_EXPAND(hit.point));
        if(hit.intersection && hit.t < closest_intersection.t)
            closest_intersection = hit;
    }

    /*
    if(closest_intersection.intersection)
        rb->box = aabb_centre(rb->box, closest_intersection.point);
    */
}

// TODO(nix3l): velocity resolution
void process_physics(physics_ctx_s* ctx) {
    f32 delta_time = engine_state->delta_time;
    for(u32 i = 0; i < ctx->physics_objects.count; i ++) {
        rigidbody_s* rb = compact_list_get(&ctx->physics_objects, i);
        if(!rb) continue;
        entity_s* ent = entity_data(rb->entity_handle);
        if(!ent) continue;
        // start off at the entities position (in case of me doing stupid stuff)
        rb->box = aabb_centre(rb->box, ent->position);

        apply_force(rb, V2F_SCALE(V2F_UNITY, -9.81f * rb->mass));

        // integrate
        euler_integrate(rb, delta_time);

        // check for collisions with static objects
        resolve_static_collisions(ctx, rb);

        // check for collisions with physics objects
        // TODO

        // update entity to hold the rigidbody's position
        ent->position = rb->box.centre;
    }
}
