#ifndef ASSET_H
#define ASSET_H

#include "base.h"
#include "memory/memory.h"

// TODO(nix3l): pack asset file loading
// TODO(nix3l): giant concatenated string with every single filepath
// TODO(nix3l): pack asset file saving 

typedef enum {
    ASSET_EMPTY = 0, // MUST be zero
    ASSET_UNLOADED,
    ASSET_LOADED,
} asset_state_t;

typedef enum {
    ASSET_NONE            = 0x00,
    ASSET_DIRTY           = 0x01,
    ASSET_CLEAN_ON_FORGET = 0x02,
} asset_flags_t;

typedef enum {
    ASSET_DISK,
    ASSET_RUNTIME,
} asset_type_t;

typedef enum {
    ASSET_UNKNOWN_DATA,
    ASSET_MESH,
    ASSET_TEXTURE,
} asset_data_t;

// NOTE(nix3l): TREAT ALL OF THESE AS OPAQUE STRUCTS!!!
typedef struct {
    u32 handle; // refers to index in the assets array
    u32 type; // disk or runtime
    u32 data; // type of data held in asset
    u32 state; // empty, loaded, unloaded
    u32 ref_count;
    u32 flags;
} asset_slot_s;

typedef struct {
    asset_slot_s slot;

    void* data;
} asset_s;

// NOTE(nix3l): the idea behind dirty assets is that
//              this wrapper only exists for one frame ever
typedef struct {
    asset_s* contents;

    struct dirty_asset_s* next;
} dirty_asset_s;

typedef struct {
    arena_s* assets_arena; // MUST be zero'd
    arena_s* dirty_assets_arena; // data never removed from here, assumed to be
                                 // cleared every frame by whoever manages it

    u32 disk_asset_capacity;
    u32 runtime_asset_capacity;

    u32 disk_asset_count;
    asset_s* disk_assets;

    u32 runtime_asset_count;
    u32 first_empty_runtime_asset;
    asset_s* runtime_assets;

    u32 dirty_asset_count;
    dirty_asset_s* first_dirty_asset;
    dirty_asset_s* last_dirty_asset;
} asset_manager_s;

extern asset_manager_s* asset_manager;

void init_asset_manager(asset_manager_s* assets, arena_s* assets_arena, arena_s* dirty_assets_arena, u32 disk_assets, u32 runtime_assets);

asset_s* push_runtime_asset();
void destroy_runtime_asset(asset_s* asset);

// TODO(nix3l) -----------
void disk_asset_load(asset_s* asset);
void disk_asset_unload(asset_s* asset);
// -----------------------
void runtime_asset_load(asset_s* asset, void* data, asset_data_t data_type);
void runtime_asset_unload(asset_s* asset, bool destroy_data);

void ref_asset(asset_s* asset);
void unref_asset(asset_s* asset);

asset_s* get_asset(asset_slot_s slot);
asset_s* get_disk_asset(u32 handle);
asset_s* get_runtime_asset(u32 handle);

void flag_asset(asset_s* asset, asset_flags_t flags);
void unflag_asset(asset_s* asset, asset_flags_t flags);

void flag_dirty_asset(asset_s* asset);
void cleanup_assets();

#endif
