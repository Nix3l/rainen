#include "serialisation.h"

#include "util/util.h"
#include "engine.h"
#include "platform/platform.h"

// version of save file
#define SVER 0

static void DEVwrite_entity(string_builder_s* builder, entity_s* ent) {
    strb_catstr(builder, "ent {\n", STRLIT_LEN("ent {\n"));

    strb_catf(builder, "\tflags [%u]\n", ent->flags);
    strb_catf(builder, "\tposition [%f, %f]\n", V2F_EXPAND(ent->position));

    if(ent->flags & ENTITY_DRAWABLE) {
        strb_catf(builder, "\tsprite.scale [%f, %f]\n", V2F_EXPAND(ent->sprite.scale));
        strb_catf(builder, "\tsprite.offset [%f, %f]\n", V2F_EXPAND(ent->sprite.offset));
        strb_catf(builder, "\tsprite.rotation [%f]\n", ent->sprite.rotation);
        strb_catf(builder, "\tsprite.layer [%d]\n", ent->sprite.layer);
        strb_catf(builder, "\tsprite.color [%f, %f, %f, %f]\n", V4F_EXPAND(ent->sprite.color));
    }

    strb_catstr(builder, "}\n", STRLIT_LEN("}\n"));
}

void DEVsave_game_state(char* filename, arena_s* arena) {
    string_builder_s builder;
    strb_init(&builder);

    strb_catf(&builder, "v%d.%d\n", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR);
    strb_catf(&builder, "sv%d\n", SVER);
    strb_catf(&builder, "entity_count [%u]\n", engine->entity_handler.entities.count);

    for(u32 i = 0; i <= engine->entity_handler.entities.last_used_index; i ++) {
        entity_s* ent = entity_data(i);
        if(!ent) continue;
        DEVwrite_entity(&builder, ent);
    }

    // no need to write the null terminator
    DEVplatform_write_to_file(filename, builder.str, builder.size - 1, false);
    strb_terminate(&builder);
}

void DEVload_game_state(char* filename, arena_s* arena) {
    // TODO(nix3l)
}
