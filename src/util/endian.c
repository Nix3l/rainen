#include "endian.h"
#include "log.h"
#include "base.h"

endianness_t get_endianness() {
    endianness_t endianness = UNKNOWN_ENDIAN;
    
    union {
        u32 in;
        u8 out[4];
    } test = { 0x01020304 };

    if(test.out[0] == 0x01)
        endianness = BIG_ENDIAN;
    else if(test.out[0] == 0x04)
        endianness = LITTLE_ENDIAN;

    return endianness;
}
