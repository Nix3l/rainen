#include "serialisation.h"

#include "util/util.h"
#include "engine.h"

void DEVsave_game_state(char* filename, arena_s* arena) {
    string_builder_s builder;
    strb_init(&builder);

    /* string builder testing dont mind me
    strb_catc(&builder, 'c');
    strb_catstr(&builder, "stringy thingy", sizeof("stringy thingy") - 1);
    strb_cati32(&builder, -13);
    strb_catu32(&builder, 12);
    strb_catf32(&builder, 82.3124f);
    strb_catv2f(&builder, V2F(0.0f, 2.3f));
    strb_catv3f(&builder, V3F(1.0f, 1.3f, 0.4f));
    strb_catv4f(&builder, V4F(8.0f, 2.1f, 8.4232f, 0.0f));
    strb_catf(&builder, " hello this is my string %f", 0.87f);
    */

    LOG("%s\n", builder.str);
    strb_terminate(&builder);
}
