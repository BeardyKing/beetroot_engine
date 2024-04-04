//#include <beet_gfx/db_asset.h>

#include <beet_gfx/db_asset.h>
#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>

//===CAMERA=================================================================
static Camera s_dbCameras[MAX_DB_CAMERAS] = {0};
static uint32_t s_dbCameraCount = {0};

static CameraEntity s_dbCameraEntities[MAX_DB_CAMERA_ENTITIES] = {0};
static uint32_t s_dbCameraEntitiesCount = {0};

uint32_t db_add_camera(const Camera &camera) {
    ASSERT(s_dbCameraCount < MAX_DB_CAMERAS);
    uint32_t currentCameraIndex = s_dbCameraCount;
    s_dbCameras[currentCameraIndex] = camera;
    s_dbCameraCount++;
    return currentCameraIndex;
}

Camera *db_get_camera(uint32_t index) {
    ASSERT(index < MAX_DB_CAMERAS);
    return &s_dbCameras[index];
}

uint32_t db_add_camera_entity(const CameraEntity &camera) {
    ASSERT(s_dbCameraEntitiesCount < MAX_DB_CAMERA_ENTITIES);
    uint32_t currentCameraEntityIndex = s_dbCameraEntitiesCount;
    s_dbCameraEntities[currentCameraEntityIndex] = camera;
    s_dbCameraEntitiesCount++;
    return currentCameraEntityIndex;
}

CameraEntity *db_get_camera_entity(uint32_t index) {
    ASSERT(index < MAX_DB_CAMERA_ENTITIES);
    return &s_dbCameraEntities[index];
}
//==========================================================================

//===TRANSFORM==============================================================
static Transform s_dbTransforms[MAX_DB_TRANSFORMS] = {0};
static uint32_t s_dbTransformsCount = {0};

uint32_t db_add_transform(const Transform &transform) {
    ASSERT(s_dbTransformsCount < MAX_DB_TRANSFORMS);
    uint32_t currentTransformIndex = s_dbTransformsCount;
    s_dbTransforms[currentTransformIndex] = transform;
    s_dbTransformsCount++;
    return currentTransformIndex;
}

Transform *db_get_transform(uint32_t index) {
    ASSERT(index < MAX_DB_TRANSFORMS);
    return &s_dbTransforms[index];
}
//==========================================================================
