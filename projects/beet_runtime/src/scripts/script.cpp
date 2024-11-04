#include <runtime/scripts/script.h>
#include <runtime/scripts/script_camera.h>
#include <runtime/scripts/script_dungeon_tiles.h>

//===API================================================================================================================

void script_create() {
    script_dungeon_tile_create();
}

void script_update(float deltaTime){
    script_update_camera();
    script_dungeon_tile_update(deltaTime);
}

void script_shutdown(){
    script_dungeon_tile_cleanup();
}

//======================================================================================================================
