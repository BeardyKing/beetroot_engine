#include <runtime/scripts/script_dungeon_tiles.h>
#include <beet_math/vec2.h>
#include <runtime/entity_builder.h>
#include "beet_gfx/db_asset.h"

static struct {
    const vec2i tileAmount = {5, 8};
    uint32_t tiles[128] = {};
} s_dungeonTileData;

//===API================================================================================================================
void script_dungeon_tile_create() {
    int32_t idx = {};
    for (int row = 0; row < s_dungeonTileData.tileAmount.x; ++row) {
        for (int col = 0; col < s_dungeonTileData.tileAmount.y; ++col) {
            s_dungeonTileData.tiles[idx] = entity_create_lit_cube();
            LitEntity *currEntity = db_get_lit_entity(s_dungeonTileData.tiles[idx]);
#if BEET_DEBUG
            sprintf(currEntity->debug_name, "Tile[%i] - {%i,%i} ", idx, row, col);
#endif //BEET_DEBUG
            Transform *currTransform = db_get_transform(currEntity->transformIndex);
            currTransform->position = {row, 0, col};
            currTransform->scale = {0.9f, 0.25f, 0.9f};
            ++idx;
        }
    }
}

void script_dungeon_tile_update(float deltaTime) {}
void script_dungeon_tile_cleanup() {}
//======================================================================================================================
