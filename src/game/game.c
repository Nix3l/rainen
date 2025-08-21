#include "game.h"
#include "entity.h"

game_ctx_t game_ctx;

static pool_t entity_pool;

static entity_manager_t render_manager;

void game_init() {
    // ENTITIES
    entity_pool = pool_alloc_new(GAME_MAX_ENTITIES, sizeof(entity_slot_t), EXPAND_TYPE_IMMUTABLE);

    // reserve first element for invalid ids
    pool_push(&entity_pool, NULL);

    render_manager.label = "render-manager";
    render_manager.batch = vector_alloc_new(GAME_MAX_ENTITIES, sizeof(entity_t));

    // CAMERA
    camera_t camera = (camera_t) {
        .transform = (transform_t) {
            .position = v2f_ZERO,
            .rotation = 0.0f,
            .z = 100,
        },
        .near = 0.1f,
        .far = 200.0f,
        .pixel_scale = 2.0f,
    };

    game_ctx = (game_ctx_t) {
        .num_dirty_entities = 0,
        .entity_pool = &entity_pool,
        .render_manager = &render_manager,
        .camera = camera,
    };
}

void game_terminate() {
    pool_destroy(game_ctx.entity_pool);
    vector_destroy(&game_ctx.render_manager->batch);
}

void game_update() {
    entity_update();
    camera_attach(&game_ctx.camera, &render_ctx.renderer.pass);
}
