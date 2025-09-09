#ifndef _EVENTS_H
#define _EVENTS_H

#include "base.h"
#include "memory/memory.h"

enum {
    EVENTS_MAX = 128,
};

typedef enum event_type_t {
    EVENT_INVALID = 0,
    _EVENT_NUM,
} event_type_t;

typedef enum event_state_t {
    EVENT_STATE_INVALID = 0,
    EVENT_STATE_WAITING,
    EVENT_STATE_ACCEPTED,
} event_state_t;

typedef union event_payload_t {
    // all the data
} event_payload_t;

typedef struct event_t {
    event_type_t type;
    event_state_t state;
    event_payload_t payload;
} event_t;

typedef struct event_info_t {
    event_type_t type;
    event_payload_t payload;
} event_info_t;

void event_invoke(event_info_t info);
void event_consume(event_t* event);

typedef void (*event_callback_fn) (event_t* event);

typedef struct event_observer_t {
    event_type_t type;
    // NOTE(nix3l): llist for now
    // doesnt play nice with the cpu cache but easier to implement
    // will fix if needed
    llist_t callbacks;
} event_observer_t;

void event_listen(event_type_t type, event_callback_fn clbk);

typedef struct events_ctx_t {
    arena_t rations;
    vector_t queue;
    vector_t observers;
} events_ctx_t;

extern events_ctx_t events_ctx;

void events_init();
void events_terminate();

void events_process();

#endif
