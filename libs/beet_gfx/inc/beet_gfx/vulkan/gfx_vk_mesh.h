#ifndef BEETROOT_GFX_MESH_H
#define BEETROOT_GFX_MESH_H

#include <cstdint>

#include <beet_shared/feature_defines.h>

#include <vulkan/vulkan_core.h>

#include <beet_math/vec2.h>
#include <beet_math/vec3.h>
#include <beet_math/vec4.h>

#include <vector>


//===PUBLIC_STRUCTS=====================================================================================================
struct GfxVertex {
    vec3f pos;
    vec3f normal;
    vec2f uv;
    vec3f color;
};

struct GfxInstanceData {
    vec3f pos;
    vec3f rot;
    float scale;
    uint32_t texIndex;
};

struct RawMesh {
    GfxVertex *vertexData;
    uint32_t *indexData;

    uint32_t vertexCount;
    uint32_t indexCount;
};

struct GfxMesh {
    uint32_t vertCount;
    VkBuffer vertBuffer;
    VkDeviceMemory vertMemory;

    uint32_t indexCount;
    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;
};

struct PackageEntry {
    uint32_t meshIndex;
    uint32_t materialIndex;
};

struct RawMaterial {
    char albedoPath[256] = {};
    vec2f albedoCoords = {};
};

struct AssetPackage {
    std::vector<PackageEntry> packageTable = {};
    std::vector<GfxMesh> meshes = {};
    std::vector<RawMaterial> materials = {};
};
//======================================================================================================================

//===API================================================================================================================
void gfx_mesh_create_cube_immediate(GfxMesh &outMesh);
void gfx_mesh_create_octahedron_immediate(GfxMesh &outMesh);
#if CHECK_FEATURE(FEATURE_IN_DEV_RUNTIME_GLTF_LOADING)
std::vector<GfxMesh> gfx_mesh_load_gltf();
AssetPackage asset_package_load_gltf(const char *inPath);
#endif //IN_DEV_RUNTIME_GLTF_LOADING
void gfx_mesh_create_immediate(const RawMesh &rawMesh, GfxMesh &outMesh);
void gfx_mesh_cleanup(GfxMesh &mesh);
//======================================================================================================================

#endif //BEETROOT_GFX_MESH_H
