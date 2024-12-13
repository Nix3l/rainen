#include "asset.h"

#include "util/util.h"

#include "mesh/mesh.h"
#include "texture/texture.h"

static void asset_cleanup_data(asset_s* asset) {
    if(asset->slot.data == ASSET_MESH)
        destroy_mesh(asset->data);
    if(asset->slot.data == ASSET_TEXTURE)
        destroy_texture(asset->data);

    asset->slot.data = ASSET_UNKNOWN_DATA;
    asset->data = NULL;
}

void init_asset_manager(asset_manager_s* assets, arena_s* arena, arena_s* dirty_assets_arena, u32 disk_assets, u32 runtime_assets) {
    assets->assets_arena = arena;
    assets->dirty_assets_arena = dirty_assets_arena;

    assets->disk_asset_capacity = disk_assets;
    assets->runtime_asset_capacity = runtime_assets;

    assets->disk_asset_count = 0;
    assets->runtime_asset_count = 0;

    assets->disk_assets = arena_push(arena, disk_assets * sizeof(asset_s));
    assets->runtime_assets = arena_push(arena, runtime_assets * sizeof(asset_s));

    assets->dirty_asset_count = 0;
    assets->first_dirty_asset = NULL;
    assets->last_dirty_asset = NULL;
}

asset_s* push_runtime_asset() {
    if(asset_manager->runtime_asset_count + 1 > asset_manager->runtime_asset_capacity) {
        LOG_ERR("maximum amount of runtime assets reached\n");
        return NULL;
    }

    asset_manager->runtime_asset_count ++;
    u32 id = asset_manager->first_empty_runtime_asset;
    asset_s* asset = &asset_manager->runtime_assets[id];

    asset->slot.handle = id;
    asset->slot.type = ASSET_RUNTIME;
    asset->slot.data = ASSET_UNKNOWN_DATA;
    asset->slot.state = ASSET_UNLOADED;
    asset->slot.ref_count = 1;
    asset->slot.flags = ASSET_NONE;

    asset->data = NULL;

    for(u32 i = asset_manager->first_empty_runtime_asset; i < asset_manager->runtime_asset_capacity; i ++) {
        asset_s* curr_asset = &asset_manager->runtime_assets[i];
        if(curr_asset->slot.state == ASSET_EMPTY) {
            asset_manager->first_empty_runtime_asset = i;
            break;
        }
    }

    return asset;
}

void destroy_runtime_asset(asset_s* asset) {
    asset->slot.state = ASSET_EMPTY;
    
    if(asset->slot.handle < asset_manager->first_empty_runtime_asset)
        asset_manager->first_empty_runtime_asset = asset->slot.handle;

    asset_cleanup_data(asset);

    asset_manager->runtime_asset_count --;
}

void runtime_asset_load(asset_s* asset, void* data, asset_data_t data_type) {
    asset->slot.state = ASSET_LOADED;
    asset->slot.data = data_type;
    asset->data = data;
}

void runtime_asset_unload(asset_s* asset, bool destroy_data) {
    asset->slot.state = ASSET_UNLOADED;

    if(destroy_data) asset_cleanup_data(asset);
}

void ref_asset(asset_s* asset) {
    asset->slot.ref_count ++;
    if(asset->slot.flags & ASSET_DIRTY) unflag_asset(asset, ASSET_DIRTY);
}

void unref_asset(asset_s* asset) {
    asset->slot.ref_count --;

    if(!(asset->slot.flags & ASSET_CLEAN_ON_FORGET) && asset->slot.ref_count == 0)
        flag_dirty_asset(asset);
}

asset_s* get_asset(asset_slot_s slot) {
    asset_s* asset = NULL;
    if(slot.type == ASSET_DISK)
        asset = &asset_manager->disk_assets[slot.handle];
    else if(slot.type == ASSET_RUNTIME)
        asset = &asset_manager->runtime_assets[slot.handle];
    else
        PANIC("invalid asset type\n");

    ref_asset(asset);
    return asset;
}

asset_s* get_disk_asset(u32 handle) {
    asset_s* asset = &asset_manager->disk_assets[handle];
    ref_asset(asset);
    return asset;
}

asset_s* get_runtime_asset(u32 handle) {
    asset_s* asset = &asset_manager->runtime_assets[handle];
    ref_asset(asset);
    return asset;
}

void flag_asset(asset_s* asset, asset_flags_t flags) {
    asset->slot.flags |= flags;
}

void unflag_asset(asset_s* asset, asset_flags_t flags) {
    asset->slot.flags &= ~flags;
}

void flag_dirty_asset(asset_s* asset) {
    if(asset->slot.flags & ASSET_DIRTY)
        return;

    flag_asset(asset, ASSET_DIRTY);

    asset_manager->dirty_asset_count ++;
    dirty_asset_s* dirty_asset = arena_push(asset_manager->dirty_assets_arena, sizeof(dirty_asset_s));
    dirty_asset->contents = asset;
    dirty_asset->next = NULL;

    if(!asset_manager->first_dirty_asset) {
        asset_manager->first_dirty_asset = dirty_asset;
        asset_manager->last_dirty_asset = dirty_asset;
    } else {
        asset_manager->last_dirty_asset->next = (struct dirty_asset_s*) dirty_asset;
        asset_manager->last_dirty_asset = dirty_asset;
    }
}

void cleanup_assets() {
    if(asset_manager->dirty_asset_count == 0 || !asset_manager->first_dirty_asset)
        return;

    dirty_asset_s* dirty_asset = asset_manager->first_dirty_asset;
    for(u32 i = 0; i < asset_manager->dirty_asset_count; i ++) {
        // in case it was flagged as dirty at some point and then referenced again
        if(dirty_asset->contents->slot.flags & ASSET_DIRTY) {
            asset_cleanup_data(dirty_asset->contents);
            unflag_asset(dirty_asset->contents, ASSET_DIRTY);
        }

        dirty_asset = (dirty_asset_s*) dirty_asset->next;
        if(!dirty_asset) break; // reached end of linked list
    }

    asset_manager->dirty_asset_count = 0;
    asset_manager->first_dirty_asset = NULL;
    asset_manager->last_dirty_asset = NULL;
}
