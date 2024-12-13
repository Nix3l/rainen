#ifndef UTILITIES_H
#define UTILITIES_H

#include "base.h"
#include "memory/memory.h"

#define LOG(...) do { fprintf(stdout, "(%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stdout,  __VA_ARGS__); } while(0)
#define LOG_ERR(...) do { fprintf(stderr, "ERR (%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__); } while(0)
#define LOG_WARN(...) do { fprintf(stderr, "WARN (%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__); } while(0)

// MATH
mat4s get_transformation_matrix(v2f position, f32 rotation, v2f scale);

// STRING BUILDER
typedef struct {
    arena_s arena;

    u32 size; // includes the null terminator
    u32 capacity; // maximum size that can be reached (includes the null terminator)
                  // if 0, then uncapped
    char* str; // null terminated string
} string_builder_s;

// initialises a string builder on the heap
void string_builder_init(string_builder_s* str_builder);

// initialises a string builder on the heap with a maximum character limit
void string_builder_init_max(string_builder_s* str_builder, u32 capacity);

// appends a string to the end of the output
// str doesnt have to be null terminated and size should not include the null terminator if exists
void string_builder_concat(string_builder_s* str_builder, char* str, u32 size);
// sets the character of the output at the specified index
void string_builder_setc(string_builder_s* str_builder, u32 index, char c);

// copies the output to the given arena
char* string_builder_export(string_builder_s* str_builder, arena_s* arena);

// destroys the string builder and its contents
void string_builder_terminate(string_builder_s* str_builder);

#endif
