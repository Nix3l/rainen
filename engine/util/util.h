#ifndef UTILITIES_H
#define UTILITIES_H

#include "base.h"
#include "memory/memory.h"

#define LOG(...) do { fprintf(stdout, "(%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stdout,  __VA_ARGS__); } while(0)
#define LOG_ERR(...) do { fprintf(stderr, "ERR (%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__); } while(0)
#define LOG_WARN(...) do { fprintf(stderr, "WARN (%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__); } while(0)

// MATH
// returns a 4x4 transformation matrix
mat4s get_transformation_matrix(v2f position, f32 rotation, v2f scale);

// STRINGS
#define STRLIT_LEN(_lit) (sizeof(_lit) - 1)

// NOTE(nix3l): have to implement this because gcc and the c standard are awesome yay
// returns the length of the string, not including the null terminator
// never goes past max_len characters in the string (str + max_len in memory)
u32 str_len(const char* str, u32 max_len);

// STRING BUILDER
typedef struct {
    arena_s arena;

    u32 size; // includes the null terminator
    u32 capacity; // maximum size that can be reached (includes the null terminator)
                  // if 0, then uncapped
    char* str; // null terminated string
} string_builder_s;

// initialises a string builder on the heap
void strb_init(string_builder_s* builder);

// initialises a string builder on the heap with a maximum character limit
void strb_init_max(string_builder_s* builder, u32 capacity);

// appends a string to the end of the output
// str doesnt have to be null terminated and size should not include the null terminator if exists
void strb_catstr(string_builder_s* builder, char* str, u32 size);
// appends a character to the end of the output
void strb_catc(string_builder_s* builder, char c);
// appends an i32 to the end of the output
void strb_cati32(string_builder_s* builder, i32 val);
// appends an u32 to the end of the output
void strb_catu32(string_builder_s* builder, u32 val);
// appends an f32 to the end of the output
void strb_catf32(string_builder_s* builder, f32 val);
// appends a boolean to the end of the output
void strb_catbool(string_builder_s* builder, bool b);
// appends a v2f to the end of the output
void strb_catv2f(string_builder_s* builder, v2f v);
// appends a v3f to the end of the output
void strb_catv3f(string_builder_s* builder, v3f v);
// appends a v4f to the end of the output
void strb_catv4f(string_builder_s* builder, v4f v);
// appends to the output according to the given format
void strb_catf(string_builder_s* builder, const char* fmt, ...);

// sets the character of the output at the specified index
void strb_setc(string_builder_s* builder, u32 index, char c);

// copies the output to the given arena
char* strb_export(string_builder_s* builder, arena_s* arena);

// destroys the string builder and its contents
void strb_terminate(string_builder_s* builder);

#endif
