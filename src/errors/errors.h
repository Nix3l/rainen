#ifndef _ERRORS_H
#define _ERRORS_H

#include "base.h"
#include "util/util.h"

// TODO(nix3l): make this a u32, give each part of the code its own bit, add toggling of warnings/errors

typedef enum error_code_t {
    ERR_UNKNOWN = 0,
    ERR_BAD_POINTER,
    ERR_GFX_BAD_ID,
    ERR_GFX_BAD_SLOT,
    ERR_GFX_INIT_BEFORE_ALLOC,
    ERR_GFX_MESH_INVALID_FORMAT,
} error_code_t;

#define LOG_ERR_CODE(_c) do { LOG_ERR("%u - %s\n", _c, #_c); } while(0)

#endif
