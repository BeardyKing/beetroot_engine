#include <beet_gfx/gfx_mesh.h>
#include <beet_shared/assert.h>
#include <beet_gfx/gfx_buffer.h>
#include <beet_gfx/gfx_command.h>
#include <beet_gfx/gfx_types.h>

extern VulkanBackend g_vulkanBackend;

void gfx_mesh_create_immediate(const RawMesh &rawMesh, GfxMesh &outMesh) {
    ASSERT((rawMesh.vertexCount > 0) && (rawMesh.indexCount > 0));

    struct StagingBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };
    StagingBuffer vertexStaging = {};
    StagingBuffer indexStaging = {};

    const size_t vertexBufferSize = sizeof(GfxVertex) * rawMesh.vertexCount;
    const size_t indexBufferSize = sizeof(uint32_t) * rawMesh.indexCount;

    outMesh.indexCount = rawMesh.indexCount;
    outMesh.vertCount = rawMesh.vertexCount;

    // Create staging buffers
    VkResult vertexCreateStageRes = gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBufferSize,
            vertexStaging.buffer,
            vertexStaging.memory,
            rawMesh.vertexData
    );
    ASSERT(vertexCreateStageRes == VK_SUCCESS);

    VkResult indexCreateStageRes = gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indexBufferSize,
            indexStaging.buffer,
            indexStaging.memory,
            rawMesh.indexData
    );
    ASSERT(indexCreateStageRes == VK_SUCCESS);

    // Create device local buffers
    const VkResult vertexCreateDeviceLocalRes = gfx_buffer_create(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBufferSize,
            outMesh.vertBuffer,
            outMesh.vertMemory,
            nullptr
    );
    ASSERT(vertexCreateDeviceLocalRes == VK_SUCCESS);
    // Index buffer
    const VkResult indexCreateDeviceLocalRes = gfx_buffer_create(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBufferSize,
            outMesh.indexBuffer,
            outMesh.indexMemory,
            nullptr
    );
    ASSERT(indexCreateDeviceLocalRes == VK_SUCCESS);

    gfx_command_begin_immediate_recording();
    {
        VkBufferCopy copyRegion = {};
        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(g_vulkanBackend.immediateCommandBuffer, vertexStaging.buffer, outMesh.vertBuffer, 1, &copyRegion);

        copyRegion.size = indexBufferSize;
        vkCmdCopyBuffer(g_vulkanBackend.immediateCommandBuffer, indexStaging.buffer, outMesh.indexBuffer, 1, &copyRegion);
    }
    gfx_command_end_immediate_recording();

    vkDestroyBuffer(g_vulkanBackend.device, vertexStaging.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, vertexStaging.memory, nullptr);
    vkDestroyBuffer(g_vulkanBackend.device, indexStaging.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, indexStaging.memory, nullptr);
}

void gfx_mesh_create_octahedron_immediate(GfxMesh &outMesh) {
    const uint32_t vertexCount = 24;
    GfxVertex vertexData[vertexCount] = {
            //===POS=============//===NORMAL========================//===UV=====
            {{+0.0, +0.5, +0.0}, {-0.577350, 0.577350,  0.577350},  {0.5, 1.0}},
            {{+0.0, +0.5, +0.0}, {-0.577350, 0.577350,  -0.577350}, {0.5, 1.0}},
            {{+0.0, +0.5, +0.0}, {0.577350,  0.577350,  0.577350},  {0.5, 1.0}},
            {{+0.0, +0.5, +0.0}, {0.577350,  0.577350,  -0.577350}, {0.5, 1.0}},
            {{+0.5, +0.0, -0.0}, {0.577350,  -0.577350, 0.577350},  {0.0, 0.5}},
            {{+0.5, +0.0, -0.0}, {0.577350,  -0.577350, -0.577350}, {0.0, 0.5}},
            {{+0.5, +0.0, -0.0}, {0.577350,  0.577350,  0.577350},  {0.0, 0.5}},
            {{+0.5, +0.0, -0.0}, {0.577350,  0.577350,  -0.577350}, {0.0, 0.5}},
            {{+0.0, -0.5, -0.0}, {-0.577350, -0.577350, 0.577350},  {0.5, 0.0}},
            {{+0.0, -0.5, -0.0}, {-0.577350, -0.577350, -0.577350}, {0.5, 0.0}},
            {{+0.0, -0.5, -0.0}, {0.577350,  -0.577350, 0.577350},  {0.5, 0.0}},
            {{+0.0, -0.5, -0.0}, {0.577350,  -0.577350, -0.577350}, {0.5, 0.0}},
            {{-0.5, +0.0, -0.0}, {-0.577350, -0.577350, 0.577350},  {1.0, 0.5}},
            {{-0.5, +0.0, -0.0}, {-0.577350, -0.577350, -0.577350}, {1.9, 0.5}},
            {{-0.5, +0.0, -0.0}, {-0.577350, 0.577350,  0.577350},  {1.0, 0.5}},
            {{-0.5, +0.0, -0.0}, {-0.577350, 0.577350,  -0.577350}, {1.0, 0.5}},
            {{+0.0, -0.0, +0.5}, {-0.577350, -0.577350, 0.577350},  {1.0, 0.0}},
            {{+0.0, -0.0, +0.5}, {-0.577350, 0.577350,  0.577350},  {1.0, 1.0}},
            {{+0.0, -0.0, +0.5}, {0.577350,  -0.577350, 0.577350},  {0.0, 0.0}},
            {{+0.0, -0.0, +0.5}, {0.577350,  0.577350,  0.577350},  {0.0, 1.0}},
            {{+0.0, +0.0, -0.5}, {-0.577350, -0.577350, -0.577350}, {0.5, 0.5}},
            {{+0.0, +0.0, -0.5}, {-0.577350, 0.577350,  -0.577350}, {0.5, 0.5}},
            {{+0.0, +0.0, -0.5}, {0.577350,  -0.577350, -0.577350}, {0.5, 0.5}},
            {{+0.0, +0.0, -0.5}, {0.577350,  0.577350,  -0.577350}, {0.5, 0.5}},
    };

    const uint32_t indexCount = 24;
    uint32_t indexData[indexCount] = {
            2, 19, 6,
            4, 18, 10,
            8, 16, 12,
            14, 17, 0,
            11, 22, 5,
            13, 20, 9,
            1, 21, 15,
            7, 23, 3,
    };

    const RawMesh rawMesh = {
            vertexData,
            indexData,
            vertexCount,
            indexCount,
    };

    gfx_mesh_create_immediate(rawMesh, outMesh);
}

void gfx_mesh_create_cube_immediate(GfxMesh &outMesh) {
    const uint32_t vertexCount = 24;
    GfxVertex vertexData[vertexCount] = {
            //===POS================//===NORMAL=========//===UV======
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    };

    const uint32_t indexCount = 36;
    uint32_t indexData[indexCount] = {
            0, 3, 2,
            2, 1, 0,
            4, 5, 6,
            6, 7, 4,
            11, 8, 9,
            9, 10, 11,
            12, 13, 14,
            14, 15, 12,
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
    };

    const RawMesh rawMesh = {
            vertexData,
            indexData,
            vertexCount,
            indexCount,
    };

    gfx_mesh_create_immediate(rawMesh, outMesh);
}

void gfx_mesh_cleanup(GfxMesh &mesh) {
    vkDestroyBuffer(g_vulkanBackend.device, mesh.vertBuffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, mesh.vertMemory, nullptr);
    vkDestroyBuffer(g_vulkanBackend.device, mesh.indexBuffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, mesh.indexMemory, nullptr);
    mesh = {};

    //TODO:GFX We don't re-add this as a free slot in the texture pool i.e.
    //we could address this pretty simply in a few ways.
    //create free-list for each pool and next time we try and create a new texture to check if any free list spaces are free
    //we move the last image loaded into the newly free position and fix up and dependency, this will break any cached references.
}