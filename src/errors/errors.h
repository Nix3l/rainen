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
    ERR_RENDER_BAD_PASS,
    ERR_RENDER_NO_ACTIVE_GROUP,
    ERR_RENDER_BAD_CALL,
    ERR_ENT_BAD_ID,
    ERR_ENT_BAD_SLOT,
    ERR_ENT_BAD_MANAGER,
    ERR_ENT_GARBAGE_COLLECTION_MISMATCH,
} error_code_t;

#define LOG_ERR_CODE(_c) do { LOG_ERR("%s - code [%u]\n", #_c, _c); } while(0)

#endif
