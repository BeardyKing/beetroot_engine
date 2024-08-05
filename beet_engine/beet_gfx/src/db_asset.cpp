#include <beet_gfx/db_asset.h>
#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>
#include <beet_shared/memory.h>

constexpr uint8_t MAX_ALLOCATION_TABLE_SIZE = UINT8_MAX;
constexpr uint8_t MAX_ALLOCATION_NAME = 64;

struct PoolInfo{
    void* start;
    uint32_t currentIndex;
};

struct AllocInfo{
    size_t itemSize;
    size_t itemCount;
    char poolName[MAX_ALLOCATION_NAME];
};

struct AllocEntry{
    AllocInfo allocInfo;
    PoolInfo* poolInfo;
};

static AllocEntry s_allocationTable[MAX_ALLOCATION_TABLE_SIZE] = {};
static size_t s_allocationTableCount = {0};

void db_cleanup_pools(){
    for (uint8_t i = 0; i < s_allocationTableCount; ++i) {
        free(s_allocationTable[i].poolInfo);
    }
}

void db_dump_pool_alloc_table(){
    size_t totalPoolsSize = {};
    size_t usedPoolsSize = {};
    for (uint8_t i = 0; i < s_allocationTableCount; ++i) {
        AllocEntry& entry = s_allocationTable[i];
        printf("=================================\n");
        printf("Name: %s\n", entry.allocInfo.poolName);
        size_t poolSize = entry.allocInfo.itemSize * entry.allocInfo.itemCount;
        size_t usedSize = entry.allocInfo.itemSize * entry.poolInfo->currentIndex;
        totalPoolsSize += poolSize;
        usedPoolsSize += usedSize;

        printf("Pool size: %zu Bytes\n", poolSize);
        printf("Used space: %zu Bytes\n", usedSize);
        printf("=================================\n");
    }
    printf("=================================\n");
    printf("Total Pool size: %zu Bytes\n", totalPoolsSize);
    printf("Used Pool size: %zu Bytes\n", usedPoolsSize);
    printf("=================================\n");
}

PoolInfo* db_pool_alloc(const AllocInfo& info) {
    const size_t poolAllocSize = info.itemCount * info.itemSize;
    const size_t totalAllocation = poolAllocSize + sizeof(PoolInfo);
    PoolInfo* poolInfo = (PoolInfo*)memset(malloc(totalAllocation), 0, totalAllocation);
    ASSERT_MSG(poolInfo, "Err failed to allocate pool %s", info.poolName);

    poolInfo->start = (char*) poolInfo + sizeof(PoolInfo);
    poolInfo->currentIndex = 0;

    AllocEntry& entry = s_allocationTable[s_allocationTableCount];
    s_allocationTableCount++;
    entry.allocInfo = info;
    entry.poolInfo = poolInfo;

    return poolInfo;
}

//===CAMERA=============================================================================================================
static struct CameraPool{
    Camera* start;
    uint32_t count;
}& s_dbCameras = *(CameraPool*) db_pool_alloc({sizeof(Camera), MAX_DB_CAMERAS, "Pool Camera"});


uint32_t db_add_camera(const Camera &camera) {
    ASSERT(s_dbCameras.count < MAX_DB_CAMERAS);
    uint32_t currentCameraIndex = s_dbCameras.count;
    s_dbCameras.start[currentCameraIndex] = camera;
    s_dbCameras.count++;
    return currentCameraIndex;
}

Camera *db_get_camera(uint32_t index) {
    ASSERT(index < MAX_DB_CAMERAS);
    return &s_dbCameras.start[index];
}
//======================================================================================================================

//===CAMERA_ENTITIES====================================================================================================
static struct CameraEntityPool{
    CameraEntity* start;
    uint32_t count;
}& s_dbCameraEntities = *(CameraEntityPool*) db_pool_alloc({sizeof(CameraEntity), MAX_DB_CAMERA_ENTITIES, "Pool Camera Entity"});

uint32_t db_get_camera_entity_count() {
    return s_dbCameraEntities.count;
}

uint32_t db_add_camera_entity(const CameraEntity &camera) {
    ASSERT(s_dbCameraEntities.count < MAX_DB_CAMERA_ENTITIES);
    uint32_t currentCameraEntityIndex = s_dbCameraEntities.count;
    s_dbCameraEntities.start[currentCameraEntityIndex] = camera;
    s_dbCameraEntities.count++;
    return currentCameraEntityIndex;
}

CameraEntity *db_get_camera_entity(uint32_t index) {
    ASSERT(index < MAX_DB_CAMERA_ENTITIES);
    return &s_dbCameraEntities.start[index];
}
//======================================================================================================================

//===TRANSFORM==========================================================================================================
static struct TransformPool{
    Transform* start;
    uint32_t count;
}& s_dbTransforms = *(TransformPool*) db_pool_alloc({sizeof(Transform), MAX_DB_TRANSFORMS, "Pool Transforms"});

uint32_t db_add_transform(const Transform &transform) {
    ASSERT(s_dbTransforms.count < MAX_DB_TRANSFORMS);
    uint32_t currentTransformIndex = s_dbTransforms.count;
    s_dbTransforms.start[currentTransformIndex] = transform;
    s_dbTransforms.count++;
    return currentTransformIndex;
}

Transform *db_get_transform(uint32_t index) {
    ASSERT(index < MAX_DB_TRANSFORMS);
    return &s_dbTransforms.start[index];
}
//======================================================================================================================

//===DESCRIPTOR=========================================================================================================
static struct VkDescriptorSetPool{
    VkDescriptorSet* start;
    uint32_t count;
}& s_dbDescriptorSet = *(VkDescriptorSetPool*) db_pool_alloc({sizeof(VkDescriptorSet), MAX_DB_VK_DESCRIPTOR_SETS, "Pool Vk Descriptor sets"});

uint32_t db_add_descriptor_set(const VkDescriptorSet &descriptorSet) {
    ASSERT(s_dbDescriptorSet.count < MAX_DB_VK_DESCRIPTOR_SETS);
    uint32_t currentDescriptorSetIndex = s_dbDescriptorSet.count;
    s_dbDescriptorSet.start[currentDescriptorSetIndex] = descriptorSet;
    s_dbDescriptorSet.count++;
    return currentDescriptorSetIndex;
}

VkDescriptorSet *db_get_descriptor_set(uint32_t index) {
    ASSERT(index < MAX_DB_VK_DESCRIPTOR_SETS);
    return &s_dbDescriptorSet.start[index];
}
//======================================================================================================================

//===TEXTURE============================================================================================================
static struct GfxTexturePool{
    GfxTexture* start;
    uint32_t count;
}& s_dbTextures = *(GfxTexturePool*) db_pool_alloc({sizeof(GfxTexture), MAX_DB_GFX_TEXTURES, "Pool Gfx Texture"});

uint32_t db_get_texture_count() {
    return s_dbTextures.count;
}

uint32_t db_add_texture(const GfxTexture &gfxTexture) {
    ASSERT(s_dbTextures.count < MAX_DB_GFX_TEXTURES);
    uint32_t currentGfxTextureIndex = s_dbTextures.count;
    s_dbTextures.start[currentGfxTextureIndex] = gfxTexture;
    s_dbTextures.count++;
    return currentGfxTextureIndex;
}

GfxTexture *db_get_texture(uint32_t index) {
    ASSERT(index < MAX_DB_GFX_TEXTURES);
    return &s_dbTextures.start[index];
}
//======================================================================================================================

//===MESH===============================================================================================================
static struct GfxMeshPool{
    GfxMesh* start;
    uint32_t count;
}& s_dbMeshes = *(GfxMeshPool*) db_pool_alloc({sizeof(GfxMesh), MAX_DB_GFX_MESHES, "Pool Gfx Mesh"});

uint32_t db_get_mesh_count() {
    return s_dbMeshes.count;
}

uint32_t db_add_mesh(const GfxMesh &gfxMesh) {
    ASSERT(s_dbMeshes.count < MAX_DB_GFX_MESHES);
    uint32_t currentGfxTextureIndex = s_dbMeshes.count;
    s_dbMeshes.start[currentGfxTextureIndex] = gfxMesh;
    s_dbMeshes.count++;
    return currentGfxTextureIndex;
}

GfxMesh *db_get_mesh(uint32_t index) {
    ASSERT(index < MAX_DB_GFX_MESHES);
    return &s_dbMeshes.start[index];
}
//======================================================================================================================

//===LIT_MATERIAL=======================================================================================================
static struct LitMaterialPool{
    LitMaterial* start;
    uint32_t count;
}& s_dbLitMaterials = *(LitMaterialPool*) db_pool_alloc({sizeof(LitMaterial), MAX_DB_LIT_MATERIALS, "Pool Lit Material"});

uint32_t db_add_lit_material(const LitMaterial &litMaterial) {
    ASSERT(s_dbLitMaterials.count < MAX_DB_LIT_MATERIALS);
    uint32_t currentLitMaterialsIndex = s_dbLitMaterials.count;
    s_dbLitMaterials.start[currentLitMaterialsIndex] = litMaterial;
    s_dbLitMaterials.count++;
    return currentLitMaterialsIndex;
}

LitMaterial *db_get_lit_material(uint32_t index) {
    ASSERT(index < MAX_DB_LIT_MATERIALS);
    return &s_dbLitMaterials.start[index];
}
//======================================================================================================================

//===SKY_MATERIAL=======================================================================================================
static struct SkyMaterialPool{
    SkyMaterial* start;
    uint32_t count;
}& s_dbSkyMaterials = *(SkyMaterialPool*) db_pool_alloc({sizeof(SkyMaterial), MAX_DB_SKY_MATERIALS, "Pool Sky Material"});


uint32_t db_add_sky_material(const SkyMaterial &skyMaterial) {
    ASSERT(s_dbSkyMaterials.count < MAX_DB_LIT_MATERIALS);
    uint32_t currentLitMaterialsIndex = s_dbSkyMaterials.count;
    s_dbSkyMaterials.start[currentLitMaterialsIndex] = skyMaterial;
    s_dbSkyMaterials.count++;
    return currentLitMaterialsIndex;
}

SkyMaterial *db_get_sky_material(uint32_t index) {
    ASSERT(index < MAX_DB_LIT_MATERIALS);
    return &s_dbSkyMaterials.start[index];
}
//======================================================================================================================

//===LIT_ENTITIES=======================================================================================================
static struct LitEntityPool{
    LitEntity* start;
    uint32_t count;
} s_dbLitEntities = *(LitEntityPool*) db_pool_alloc({sizeof(LitEntity), MAX_DB_LIT_ENTITIES, "Pool Lit Entity"});

uint32_t db_get_lit_entity_count() {
    return s_dbLitEntities.count;
}

uint32_t db_add_lit_entity(const LitEntity &litEntity) {
    ASSERT(s_dbLitEntities.count < MAX_DB_LIT_ENTITIES);
    uint32_t currentLitEntityIndex = s_dbLitEntities.count;
    s_dbLitEntities.start[currentLitEntityIndex] = litEntity;
    s_dbLitEntities.count++;
    return currentLitEntityIndex;
}

LitEntity *db_get_lit_entity(uint32_t index) {
    ASSERT(index < MAX_DB_LIT_ENTITIES);
    return &s_dbLitEntities.start[index];
}
//======================================================================================================================

//===SKY_ENTITIES=======================================================================================================
static struct SkyEntityPool{
    SkyEntity* start;
    uint32_t count;
}& s_dbSkyEntities = *(SkyEntityPool*) db_pool_alloc({sizeof(SkyEntity), MAX_DB_SKY_ENTITIES, "Pool Lit Entity"});

uint32_t db_get_sky_entity_count() {
    return s_dbSkyEntities.count;
}

uint32_t db_add_sky_entity(const SkyEntity &skyEntity) {
    ASSERT(s_dbSkyEntities.count < MAX_DB_LIT_ENTITIES);
    uint32_t currentLitEntityIndex = s_dbSkyEntities.count;
    s_dbSkyEntities.start[currentLitEntityIndex] = skyEntity;
    s_dbSkyEntities.count++;
    return currentLitEntityIndex;
}

SkyEntity *db_get_sky_entity(uint32_t index) {
    ASSERT(index < MAX_DB_LIT_ENTITIES);
    return &s_dbSkyEntities.start[index];
}
//======================================================================================================================


