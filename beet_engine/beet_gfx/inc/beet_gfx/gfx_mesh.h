#ifndef BEETROOT_GFX_MESH_H
#define BEETROOT_GFX_MESH_H

#include <cstdint>

#include <vulkan/vulkan_core.h>

#include <beet_math/vec2.h>
#include <beet_math/vec3.h>
#include <beet_math/vec4.h>

struct GfxVertex {
    vec3f pos;
    vec3f normal;
    vec2f uv;
    vec4f color;
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

#endif //BEETROOT_GFX_MESH_H
