#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <stdint.h>
#include <cglm/struct.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef size_t   usize;
typedef float    f32;
typedef double   f64;
typedef vec2s    v2f;
typedef vec3s    v3f;
typedef vec4s    v4f;
typedef ivec2s   v2i;
typedef ivec3s   v3i;
typedef ivec4s   v4i;

#define MAX_i8  ((i8)  0x7f)
#define MAX_i16 ((i16) 0x7fff)
#define MAX_i32 ((i32) 0x7fffff)
#define MAX_i64 ((i64) 0x7fffffffffffffffllu)

#define MAX_u8  ((u8)  0xff)
#define MAX_u16 ((u16) 0xffff)
#define MAX_u32 ((u32) 0xffffff)
#define MAX_u64 ((u64) 0xffffffffffffffffllu)

#define MAX_f32 (FLT_MAX)
#define MAX_f64 (DBL_MAX)

#define PI   (3.14159265358)
#define PI_2 (1.570796327)
#define PI_4 (0.7853981634)

#endif
