#ifndef ENDIANNESS_H
#define ENDIANNESS_H

typedef enum {
    LITTLE_ENDIAN = 0,
    BIG_ENDIAN = 1,
    UNKNOWN_ENDIAN = 2
} endianness_t;

endianness_t get_endianness();

#endif
