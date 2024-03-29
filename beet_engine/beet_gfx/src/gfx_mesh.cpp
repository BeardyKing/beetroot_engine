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

void gfx_cube_create_immediate(GfxMesh &outMesh) {
    const uint32_t vertexCount = 24;
    static GfxVertex vertexData[vertexCount] = {
            //===POS================//===COLOUR=========//===UV======
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
    static uint32_t indexData[indexCount] = {
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

    RawMesh rawMesh = {
            vertexData,
            indexData,
            vertexCount,
            indexCount,
    };

    gfx_mesh_create_immediate(rawMesh, outMesh);
}