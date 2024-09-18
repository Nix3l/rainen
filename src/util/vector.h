#ifndef VECTOR_H
#define VECTOR_H

#include "base_types.h"

typedef struct {
    // size of reserved data (void* slots available in memory)
    usize capacity;
    // size of occupied data (void* slots in use)
    usize size;

    void** data;
} vector_s;

vector_s vector_create();

void* vector_get(vector_s* vec, usize index);

void vector_set(vector_s* vec, usize index, void* data);
void vector_insert(vector_s* vec, usize index, void* data);

void vector_add(vector_s* vec, void* data);
void vector_remove(vector_s* vec, usize index);

// removes all elements in a vector (without freeing)
void vector_clear(vector_s* vec);
// doesnt free the void* within the vector but frees the vector itself
void vector_free(vector_s* vec);
// frees all the elements in a vector (use with caution)
// does not free the vector itself
void vector_free_elements(vector_s* vec);

#endif
