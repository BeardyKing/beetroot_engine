#include <beet_shared/beet_types.h>
#include "beet_gfx/db_asset.h"

void primary_camera_entity_create() {
    Camera camera{};
    Transform transform{};
    transform.position = vec3f{0.0f, 0.0f, -1.0f};

    CameraEntity cameraEntity{};
    cameraEntity.transformIndex = db_add_transform(transform);
    cameraEntity.cameraIndex = db_add_camera(camera);
    db_add_camera_entity(cameraEntity);
}

void entities_create(){
    primary_camera_entity_create();
}