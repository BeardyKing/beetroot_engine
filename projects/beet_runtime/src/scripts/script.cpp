#include <runtime/scripts/script.h>
#include <runtime/scripts/script_camera.h>
#include <runtime/scripts/script_dungeon_tiles.h>
#include <runtime/scripts/script_lights.h>

//===API================================================================================================================

void script_create() {
    script_dungeon_tile_create();
    script_create_lights();
}

void script_update(float deltaTime){
    script_update_camera();
    script_dungeon_tile_update(deltaTime);
    script_update_lights(deltaTime);
}

void script_shutdown(){
    script_dungeon_tile_cleanup();
    script_shutdown_lights();
}

//======================================================================================================================
