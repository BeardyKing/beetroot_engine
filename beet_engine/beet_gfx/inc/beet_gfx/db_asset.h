#ifndef BEETROOT_DB_ASSET_H
#define BEETROOT_DB_ASSET_H

#include <beet_shared/beet_types.h>

#define MAX_DB_CAMERA_ENTITIES 1
#define MAX_DB_LIT_ENTITIES 64

#define MAX_DB_CAMERAS 1
#define MAX_DB_TRANSFORMS 64
#define MAX_DB_GFX_TEXTURES 64
#define MAX_DB_GFX_MESHES 64

#define MAX_DB_VK_DESCRIPTOR_SETS 64

#define MAX_DB_LIT_MATERIALS 64

//===CAMERA=================================================================
uint32_t db_add_camera(const Camera &camera);
Camera *db_get_camera(uint32_t index);

uint32_t db_add_camera_entity(const CameraEntity &camera);
CameraEntity *db_get_camera_entity(uint32_t index);
//==========================================================================

//===TRANSFORM==============================================================
uint32_t db_add_transform(const Transform &transform);
Transform *db_get_transform(uint32_t index);
//==========================================================================


#endif //BEETROOT_DB_ASSET_H
