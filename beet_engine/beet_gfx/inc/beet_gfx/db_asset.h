#ifndef BEETROOT_DB_ASSET_H
#define BEETROOT_DB_ASSET_H

#include <vulkan/vulkan_core.h>
#include <beet_gfx/gfx_types.h>
#include <beet_shared/beet_types.h>

//===CAMERA=============================================================================================================
#define MAX_DB_CAMERAS 1

uint32_t db_add_camera(const Camera &camera);
Camera *db_get_camera(uint32_t index);
//======================================================================================================================

//===CAMERA_ENTITIES====================================================================================================
#define MAX_DB_CAMERA_ENTITIES 1

uint32_t db_get_camera_entity_count();
uint32_t db_add_camera_entity(const CameraEntity &camera);
CameraEntity *db_get_camera_entity(uint32_t index);
//======================================================================================================================

//===TRANSFORM==========================================================================================================
#define MAX_DB_TRANSFORMS 256

uint32_t db_add_transform(const Transform &transform);
Transform *db_get_transform(uint32_t index);
//======================================================================================================================

//===DESCRIPTOR=========================================================================================================
#define MAX_DB_VK_DESCRIPTOR_SETS 64

uint32_t db_add_descriptor_set(const VkDescriptorSet &descriptorSet);
VkDescriptorSet *db_get_descriptor_set(uint32_t index);
//======================================================================================================================

//===TEXTURE============================================================================================================
#define MAX_DB_GFX_TEXTURES 64

uint32_t db_get_texture_count();
uint32_t db_add_texture(const GfxTexture &gfxTexture);
GfxTexture *db_get_texture(uint32_t index);
//======================================================================================================================

//===MESH===============================================================================================================
#define MAX_DB_GFX_MESHES 256

uint32_t db_get_mesh_count();
uint32_t db_add_mesh(const GfxMesh &gfxMesh);
GfxMesh *db_get_mesh(uint32_t index);
//======================================================================================================================

//===LIT_MATERIAL=======================================================================================================
#define MAX_DB_LIT_MATERIALS 256
uint32_t db_add_lit_material(const LitMaterial &litMaterial);
LitMaterial *db_get_lit_material(uint32_t index);
//======================================================================================================================

//===LIT_SKY============================================================================================================
#define MAX_DB_SKY_MATERIALS 1
uint32_t db_add_sky_material(const SkyMaterial &skyMaterial);
SkyMaterial *db_get_sky_material(uint32_t index);
//======================================================================================================================

//===LIT_ENTITIES=======================================================================================================
#define MAX_DB_LIT_ENTITIES 256

uint32_t db_get_lit_entity_count();
uint32_t db_add_lit_entity(const LitEntity &litEntity);
LitEntity *db_get_lit_entity(uint32_t index);
//======================================================================================================================

//===LIT_ENTITIES=======================================================================================================
#define MAX_DB_SKY_ENTITIES 64

uint32_t db_get_sky_entity_count();
uint32_t db_add_sky_entity(const SkyEntity &skyEntity);
SkyEntity *db_get_sky_entity(uint32_t index);
//======================================================================================================================

#endif //BEETROOT_DB_ASSET_H
