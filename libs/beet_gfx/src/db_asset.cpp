#include <cstring>
#include <beet_gfx/db_asset.h>
#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>
#include <beet_shared/memory.h>

static PoolAllocEntry s_allocationTable[MAX_ALLOCATION_TABLE_SIZE] = {};
static size_t s_allocationTableCount = {0};

PoolAllocEntry *db_get_allocation_table(uint32_t &outTableCount) {
    outTableCount = s_allocationTableCount;
    return &s_allocationTable[0];
}

void db_dump_pool_alloc_table() {
    size_t totalPoolsSize = {};
    size_t usedPoolsSize = {};
    for (uint8_t i = 0; i < s_allocationTableCount; ++i) {
        PoolAllocEntry &entry = s_allocationTable[i];
        printf("=================================\n");
        printf("Name: %s\n", entry.allocInfo.poolName);
        size_t poolSize = entry.allocInfo.itemSize * entry.allocInfo.itemCount;
        size_t usedSize = entry.allocInfo.itemSize * entry.poolInfo->count;
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

template<typename T>
struct Pool {
    T *data = nullptr;
    uint32_t count = 0;
    uint32_t maxCount = 0;
};
static_assert(sizeof(Pool<void>) == sizeof(PoolInfo));

template<typename T>
static bool db_pool_alloc(Pool<T> &pool, PoolAllocInfo info) {
    info.itemSize = sizeof(T);

    //alloc
    const size_t poolAllocSize = info.itemCount * info.itemSize;
    const size_t totalAllocation = poolAllocSize + sizeof(Pool<T>);
    pool.data = (T *) memset(malloc(totalAllocation), 0, totalAllocation);
    if (pool.data) {
        pool.count = 0;
        pool.maxCount = info.itemCount;

        //alloc tracking
        PoolAllocEntry *entry = &s_allocationTable[s_allocationTableCount];
        s_allocationTableCount++;
        entry->allocInfo = info;
        entry->poolInfo = (PoolInfo *) &pool;
        return true;
    }
    pool = {};
    return false;
}

template<typename T>
static T *db_get_pool(const Pool<T> &pool, uint32_t idx) {
    ASSERT(idx < pool.maxCount);
    return &pool.data[idx];
}

template<typename T>
static uint32_t db_add_pool(Pool<T> &pool, const T &inType) {
    ASSERT(pool.count < pool.maxCount);
    const uint32_t outIdx = pool.count;
    pool.data[outIdx] = inType;
    pool.count++;
    return outIdx;
}

template<typename T>
static uint32_t db_get_pool_count(const Pool<T> &pool) {
    return pool.count;
}

PoolInfo *db_pool_alloc(const PoolAllocInfo &info) {
    const size_t poolAllocSize = info.itemCount * info.itemSize;
    const size_t totalAllocation = poolAllocSize + sizeof(PoolInfo);
    PoolInfo *poolInfo = (PoolInfo *) memset(malloc(totalAllocation), 0, totalAllocation);
    ASSERT_MSG(poolInfo, "Err failed to allocate pool %s", info.poolName);

    poolInfo->data = (char *) poolInfo + sizeof(PoolInfo);
    poolInfo->count = 0;

    PoolAllocEntry *entry = &s_allocationTable[s_allocationTableCount];
    s_allocationTableCount++;
    entry->allocInfo = info;
    entry->poolInfo = poolInfo;

    return entry->poolInfo;
}

//===CAMERA=============================================================================================================
#define MAX_DB_CAMERAS 1
static Pool<Camera> s_dbCameras = {};

uint32_t db_add_camera(const Camera &camera) { return db_add_pool(s_dbCameras, camera); }
Camera *db_get_camera(uint32_t index) { return db_get_pool(s_dbCameras, index); }
uint32_t db_get_camera_count() { return db_get_pool_count(s_dbCameras); }
//======================================================================================================================

//===CAMERA_ENTITIES====================================================================================================
#define MAX_DB_CAMERA_ENTITIES 1
static Pool<CameraEntity> s_dbCameraEntities = {};

uint32_t db_add_camera_entity(const CameraEntity &camera) { return db_add_pool(s_dbCameraEntities, camera); }
CameraEntity *db_get_camera_entity(uint32_t index) { return db_get_pool(s_dbCameraEntities, index); }
uint32_t db_get_camera_entity_count() { return db_get_pool_count(s_dbCameraEntities); }
//======================================================================================================================

//===TRANSFORM==========================================================================================================
#define MAX_DB_TRANSFORMS 256
static Pool<Transform> s_dbTransforms = {};

uint32_t db_add_transform(const Transform &transform) { return db_add_pool(s_dbTransforms, transform); }
Transform *db_get_transform(uint32_t index) { return db_get_pool(s_dbTransforms, index); }
uint32_t db_get_transform_count() { return db_get_pool_count(s_dbTransforms); }
//======================================================================================================================

//===DESCRIPTOR=========================================================================================================
#define MAX_DB_VK_DESCRIPTOR_SETS 1024
static Pool<VkDescriptorSet> s_dbDescriptorSet = {};

uint32_t db_add_descriptor_set(const VkDescriptorSet &descriptorSet) { return db_add_pool(s_dbDescriptorSet, descriptorSet); }
VkDescriptorSet *db_get_descriptor_set(uint32_t index) { return db_get_pool(s_dbDescriptorSet, index); }
uint32_t db_get_descriptor_set_count() { return db_get_pool_count(s_dbDescriptorSet); }
//======================================================================================================================

//===TEXTURE============================================================================================================
#define MAX_DB_GFX_TEXTURES 1024
static Pool<GfxTexture> s_dbTextures = {};

uint32_t db_add_texture(const GfxTexture &gfxTexture) { return db_add_pool(s_dbTextures, gfxTexture); }
GfxTexture *db_get_texture(uint32_t index) { return db_get_pool(s_dbTextures, index); }
uint32_t db_get_texture_count() { return db_get_pool_count(s_dbTextures); }
//======================================================================================================================

//===MESH===============================================================================================================
#define MAX_DB_GFX_MESHES 256
static Pool<GfxMesh> s_dbMeshes = {};

uint32_t db_add_mesh(const GfxMesh &gfxMesh) { return db_add_pool(s_dbMeshes, gfxMesh); }
GfxMesh *db_get_mesh(uint32_t index) { return db_get_pool(s_dbMeshes, index); }
uint32_t db_get_mesh_count() { return db_get_pool_count(s_dbMeshes); }
//======================================================================================================================

//===LIT_MATERIAL=======================================================================================================
static Pool<LitMaterial> s_dbLitMaterials = {};

uint32_t db_add_lit_material(const LitMaterial &litMaterial) { return db_add_pool(s_dbLitMaterials, litMaterial); }
LitMaterial *db_get_lit_material(uint32_t index) { return db_get_pool(s_dbLitMaterials, index); }
uint32_t db_get_lit_material() { return db_get_pool_count(s_dbLitMaterials); }
//======================================================================================================================

//===SKY_MATERIAL=======================================================================================================
#define MAX_DB_SKY_MATERIALS 1
static Pool<SkyMaterial> s_dbSkyMaterials = {};

uint32_t db_add_sky_material(const SkyMaterial &skyMaterial) { return db_add_pool(s_dbSkyMaterials, skyMaterial); }
SkyMaterial *db_get_sky_material(uint32_t index) { return db_get_pool(s_dbSkyMaterials, index); }
uint32_t db_get_sky_count() { return db_get_pool_count(s_dbSkyMaterials); }
//======================================================================================================================

//===SKY_ENTITIES=======================================================================================================
#define MAX_DB_SKY_ENTITIES 1
static Pool<SkyEntity> s_dbSkyEntities = {};

uint32_t db_add_sky_entity(const SkyEntity &skyEntity) { return db_add_pool(s_dbSkyEntities, skyEntity); }
SkyEntity *db_get_sky_entity(uint32_t index) { return db_get_pool(s_dbSkyEntities, index); }
uint32_t db_get_sky_entity_count() { return db_get_pool_count(s_dbSkyEntities); }
//======================================================================================================================


//===LIT_ENTITIES=======================================================================================================
#define MAX_DB_LIT_ENTITIES 256
static Pool<LitEntity> s_dbLitEntities = {};

uint32_t db_add_lit_entity(const LitEntity &litEntity) { return db_add_pool(s_dbLitEntities, litEntity); }
LitEntity *db_get_lit_entity(uint32_t index) { return db_get_pool(s_dbLitEntities, index); }
uint32_t db_get_lit_entity_count() { return db_get_pool_count(s_dbLitEntities); }
//======================================================================================================================

//===LIGHT_ENTITIES=====================================================================================================
#define MAX_DB_LIGHT_ENTITIES 64
static Pool<LightEntity> s_dbLightEntity = {};

uint32_t db_add_light_entity(const LightEntity &lightEntity) { return db_add_pool(s_dbLightEntity, lightEntity); }
LightEntity *db_get_light_entity(uint32_t index) { return db_get_pool(s_dbLightEntity, index); }
uint32_t db_get_light_entity_count() { return db_get_pool_count(s_dbLightEntity); }
//======================================================================================================================

//===GFX_LIGHT==========================================================================================================
#define MAX_DB_LIGHTS 64
static Pool<GfxLight> s_dbGfxLight = {};

uint32_t db_add_light(const GfxLight &light) { return db_add_pool(s_dbGfxLight, light); }
GfxLight *db_get_light(uint32_t index) { return db_get_pool(s_dbGfxLight, index); }
uint32_t db_get_light_count() { return db_get_pool_count(s_dbGfxLight); }
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void db_pools_create() {
    db_pool_alloc(s_dbCameras, {.itemCount = MAX_DB_CAMERAS, .poolName = "Pool Camera"});
    db_pool_alloc(s_dbCameraEntities, {.itemCount = MAX_DB_CAMERA_ENTITIES, .poolName = "Pool Camera Entity"});
    db_pool_alloc(s_dbTransforms, {.itemCount = MAX_DB_TRANSFORMS, .poolName = "Pool Transforms"});
    db_pool_alloc(s_dbDescriptorSet, {.itemCount = MAX_DB_VK_DESCRIPTOR_SETS, .poolName = "Pool Vk Descriptor sets"});
    db_pool_alloc(s_dbTextures, {.itemCount = MAX_DB_GFX_TEXTURES, .poolName = "Pool Gfx Texture"});
    db_pool_alloc(s_dbMeshes, {.itemCount = MAX_DB_GFX_MESHES, .poolName = "Pool Gfx Mesh"});
    db_pool_alloc(s_dbLitMaterials, {.itemCount = MAX_DB_LIT_MATERIALS, .poolName = "Pool Lit Material"});
    db_pool_alloc(s_dbSkyMaterials, {.itemCount = MAX_DB_SKY_MATERIALS, .poolName = "Pool Sky Material"});
    db_pool_alloc(s_dbSkyEntities, {.itemCount = MAX_DB_SKY_ENTITIES, .poolName = "Pool Sky Entity"});
    db_pool_alloc(s_dbLitEntities, {.itemCount = MAX_DB_LIT_ENTITIES, .poolName = "Pool Lit Entity"});
    db_pool_alloc(s_dbLightEntity, {.itemCount = MAX_DB_LIGHT_ENTITIES, .poolName = "Pool light Entity"});
    db_pool_alloc(s_dbGfxLight, {.itemCount = MAX_DB_LIGHTS, .poolName = "Pool Gfx Lights"});
}

void db_pools_cleanup() {
    for (uint8_t i = 0; i < s_allocationTableCount; ++i) {
        free(s_allocationTable[i].poolInfo->data);
    }
}
//======================================================================================================================