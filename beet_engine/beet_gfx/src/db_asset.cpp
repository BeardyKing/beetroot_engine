//#include <beet_gfx/db_asset.h>

#include <beet_gfx/db_asset.h>
#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>

//===CAMERA=============================================================================================================
static Camera s_dbCameras[MAX_DB_CAMERAS] = {0};
static uint32_t s_dbCameraCount = {0};

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
//======================================================================================================================

//===CAMERA_ENTITIES====================================================================================================
static CameraEntity s_dbCameraEntities[MAX_DB_CAMERA_ENTITIES] = {0};
static uint32_t s_dbCameraEntitiesCount = {0};

uint32_t db_get_camera_entity_count() {
    return s_dbCameraEntitiesCount;
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
//======================================================================================================================

//===TRANSFORM==========================================================================================================
static Transform s_dbTransforms[MAX_DB_TRANSFORMS] = {};
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
//======================================================================================================================

//===DESCRIPTOR=========================================================================================================
static VkDescriptorSet s_dbDescriptorSet[MAX_DB_VK_DESCRIPTOR_SETS] = {0};
static uint32_t s_dbDescriptorSetCount = {0};

uint32_t db_add_descriptor_set(const VkDescriptorSet &descriptorSet) {
    ASSERT(s_dbDescriptorSetCount < MAX_DB_VK_DESCRIPTOR_SETS);
    uint32_t currentDescriptorSetIndex = s_dbDescriptorSetCount;
    s_dbDescriptorSet[currentDescriptorSetIndex] = descriptorSet;
    s_dbDescriptorSetCount++;
    return currentDescriptorSetIndex;
}

VkDescriptorSet *db_get_descriptor_set(uint32_t index) {
    ASSERT(index < MAX_DB_VK_DESCRIPTOR_SETS);
    return &s_dbDescriptorSet[index];
}
//======================================================================================================================

//===TEXTURE============================================================================================================
static GfxTexture s_dbTextures[MAX_DB_GFX_TEXTURES] = {0};
static uint32_t s_dbTexturesCount{0};

uint32_t db_get_texture_count() {
    return s_dbTexturesCount;
}

uint32_t db_add_texture(const GfxTexture &gfxTexture) {
    ASSERT(s_dbTexturesCount < MAX_DB_GFX_TEXTURES);
    uint32_t currentGfxTextureIndex = s_dbTexturesCount;
    s_dbTextures[currentGfxTextureIndex] = gfxTexture;
    s_dbTexturesCount++;
    return currentGfxTextureIndex;
}

GfxTexture *db_get_texture(uint32_t index) {
    ASSERT(index < MAX_DB_GFX_TEXTURES);
    return &s_dbTextures[index];
}
//======================================================================================================================

//===MESH===============================================================================================================
static GfxMesh s_dbMeshes[MAX_DB_GFX_MESHES] = {0};
static uint32_t s_dbMeshesCount{0};

uint32_t db_get_mesh_count() {
    return s_dbMeshesCount;
}

uint32_t db_add_mesh(const GfxMesh &gfxMesh) {
    ASSERT(s_dbMeshesCount < MAX_DB_GFX_MESHES);
    uint32_t currentGfxTextureIndex = s_dbMeshesCount;
    s_dbMeshes[currentGfxTextureIndex] = gfxMesh;
    s_dbMeshesCount++;
    return currentGfxTextureIndex;
}

GfxMesh *db_get_mesh(uint32_t index) {
    ASSERT(index < MAX_DB_GFX_MESHES);
    return &s_dbMeshes[index];
}
//======================================================================================================================

//===LIT_MATERIAL=======================================================================================================
static LitMaterial s_dbLitMaterials[MAX_DB_LIT_MATERIALS] = {0};
static uint32_t s_dbLitMaterialsCount{0};

uint32_t db_add_lit_material(const LitMaterial &litMaterial) {
    ASSERT(s_dbLitMaterialsCount < MAX_DB_LIT_MATERIALS);
    uint32_t currentLitMaterialsIndex = s_dbLitMaterialsCount;
    s_dbLitMaterials[currentLitMaterialsIndex] = litMaterial;
    s_dbLitMaterialsCount++;
    return currentLitMaterialsIndex;
}

LitMaterial *db_get_lit_material(uint32_t index) {
    ASSERT(index < MAX_DB_LIT_MATERIALS);
    return &s_dbLitMaterials[index];
}
//======================================================================================================================

//===SKY_MATERIAL=======================================================================================================
static SkyMaterial s_dbSkyMaterials[MAX_DB_SKY_MATERIALS] = {0};
static uint32_t s_dbSkyMaterialsCount{0};

uint32_t db_add_sky_material(const SkyMaterial &skyMaterial) {
    ASSERT(s_dbSkyMaterialsCount < MAX_DB_LIT_MATERIALS);
    uint32_t currentLitMaterialsIndex = s_dbSkyMaterialsCount;
    s_dbSkyMaterials[currentLitMaterialsIndex] = skyMaterial;
    s_dbSkyMaterialsCount++;
    return currentLitMaterialsIndex;
}

SkyMaterial *db_get_sky_material(uint32_t index) {
    ASSERT(index < MAX_DB_LIT_MATERIALS);
    return &s_dbSkyMaterials[index];
}
//======================================================================================================================

//===LIT_ENTITIES=======================================================================================================
static LitEntity s_dbLitEntities[MAX_DB_LIT_ENTITIES] = {0};
static uint32_t s_dbLitEntitiesCount{0};
uint32_t db_get_lit_entity_count() {
    return s_dbLitEntitiesCount;
}

uint32_t db_add_lit_entity(const LitEntity &litEntity) {
    ASSERT(s_dbLitEntitiesCount < MAX_DB_LIT_ENTITIES);
    uint32_t currentLitEntityIndex = s_dbLitEntitiesCount;
    s_dbLitEntities[currentLitEntityIndex] = litEntity;
    s_dbLitEntitiesCount++;
    return currentLitEntityIndex;
}

LitEntity *db_get_lit_entity(uint32_t index) {
    ASSERT(index < MAX_DB_LIT_ENTITIES);
    return &s_dbLitEntities[index];
}
//======================================================================================================================

//===SKY_ENTITIES=======================================================================================================
static SkyEntity s_dbSkyEntities[MAX_DB_SKY_ENTITIES] = {0};
static uint32_t s_dbSkyEntitiesCount{0};

uint32_t db_get_sky_entity_count() {
    return s_dbSkyEntitiesCount;
}

uint32_t db_add_sky_entity(const SkyEntity &skyEntity) {
    ASSERT(s_dbSkyEntitiesCount < MAX_DB_LIT_ENTITIES);
    uint32_t currentLitEntityIndex = s_dbSkyEntitiesCount;
    s_dbSkyEntities[currentLitEntityIndex] = skyEntity;
    s_dbSkyEntitiesCount++;
    return currentLitEntityIndex;
}

SkyEntity *db_get_sky_entity(uint32_t index) {
    ASSERT(index < MAX_DB_LIT_ENTITIES);
    return &s_dbSkyEntities[index];
}
//======================================================================================================================


