#include "memory.h"

linked_list_s create_linked_list(arena_s* arena) {
    return (linked_list_s) {
        .arena = arena,
        .count = 0,
        .first = NULL,
        .last = NULL
    };
}

void linked_list_push(linked_list_s* list, void* data) {
    linked_list_element_s* slot = arena_push(list->arena, sizeof(linked_list_element_s));
    slot->contents = data;
    slot->next = NULL;

    if(list->count == 0) {
        list->first = slot;
        list->last = slot;
    } else {
        list->last->next = slot;
        list->last = slot;
    }

    list->count ++;
}

void linked_list_clear(linked_list_s* list) {
    list->count = 0;
    list->first = NULL;
    list->last  = NULL;
}
