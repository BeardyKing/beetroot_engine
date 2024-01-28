#ifndef BEETROOT_GFX_MESH_H
#define BEETROOT_GFX_MESH_H

#include <cstdint>

#include <vulkan/vulkan_core.h>

struct Vertex {
    vec3f pos;
    vec3f color;
    vec2f texCoord;
};

struct RawMesh {
    Vertex *vertexData;
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
