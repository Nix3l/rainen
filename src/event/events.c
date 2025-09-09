#include "events.h"
#include "memory/memory.h"
#include "rations/rations.h"

events_ctx_t events_ctx = {0};

void events_init() {
    arena_t events_rations = arena_new(rations.events);
    vector_t queue = arena_vector_push(&events_rations, EVENTS_MAX, sizeof(event_t));
    vector_t observers = arena_vector_push(&events_rations, _EVENT_NUM, sizeof(event_observer_t));

    for(u32 i = 0; i < _EVENT_NUM; i ++) {
        event_observer_t observer = {
            .type = i,
            .callbacks = llist_new(),
        };

        vector_push_data(&observers, &observer);
    }

    events_ctx = (events_ctx_t) {
        .rations = events_rations,
        .queue = queue,
        .observers = observers,
    };
}

void events_terminate() {
    arena_clear(&events_ctx.rations);
}

void event_invoke(event_info_t info) {
    event_t event = {0};

    if(info.type == EVENT_INVALID || info.type >= _EVENT_NUM) return;

    event.type = info.type;
    event.state = EVENT_STATE_WAITING;
    event.payload = info.payload;

    vector_push_data(&events_ctx.queue, &event);
}

void event_consume(event_t* event) {
    event->state = EVENT_STATE_ACCEPTED;
}

void events_process() {
    while(events_ctx.queue.size > 0) {
        event_t* event = vector_get(&events_ctx.queue, 0);
        event_observer_t* observer = vector_get(&events_ctx.observers, event->type);
        llist_iter_t iter = {0};
        while(event->state == EVENT_STATE_WAITING && llist_iter(&observer->callbacks, &iter)) {
            event_callback_fn clbk = iter.data;
            clbk(event);
        }

        vector_remove(&events_ctx.queue, 0);
    }
}
