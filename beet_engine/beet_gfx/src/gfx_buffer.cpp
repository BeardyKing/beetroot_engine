#include <beet_gfx/gfx_buffer.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_pipeline.h>
#include <beet_gfx/gfx_mesh.h>
#include <beet_gfx/gfx_shader.h>
#include <beet_gfx/gfx_descriptors.h>
#include <beet_gfx/gfx_utils.h>

#include <beet_shared/assert.h>

#include <vulkan/vulkan_core.h>

extern VulkanBackend g_vulkanBackend;

VkResult gfx_buffer_create(const VkBufferUsageFlags &usageFlags, const VkMemoryPropertyFlags &memoryPropertyFlags, GfxBuffer &outBuffer, const VkDeviceSize size, void *inData) {
    VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;
    ASSERT(bufferCreateInfo.size > 0);

    const VkResult createResult = vkCreateBuffer(g_vulkanBackend.device, &bufferCreateInfo, nullptr, &outBuffer.buffer);
    ASSERT(createResult == VK_SUCCESS);

    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    vkGetBufferMemoryRequirements(g_vulkanBackend.device, outBuffer.buffer, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = gfx_utils_get_memory_type(memReqs.memoryTypeBits, memoryPropertyFlags);

    // when outBuffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAllocInfo.pNext = &allocFlagsInfo;
    }
    const VkResult allocResult = vkAllocateMemory(g_vulkanBackend.device, &memAllocInfo, nullptr, &outBuffer.memory);
    ASSERT(allocResult == VK_SUCCESS);

    outBuffer.alignment = memReqs.alignment;
    outBuffer.size = size;
    outBuffer.usageFlags = usageFlags;
    outBuffer.memoryPropertyFlags = memoryPropertyFlags;

    if (inData != nullptr) {
        void *mapped;
        const VkResult VkMapRes = vkMapMemory(g_vulkanBackend.device, outBuffer.memory, 0, size, 0, &mapped);
        ASSERT(VkMapRes == VK_SUCCESS);

        memcpy(mapped, inData, size);
        // when host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mappedRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
            mappedRange.memory = outBuffer.memory;
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(g_vulkanBackend.device, 1, &mappedRange);
        }
        vkUnmapMemory(g_vulkanBackend.device, outBuffer.memory);
    }

    // Initialize a default descriptor that covers the whole buffer size
    outBuffer.descriptor.offset = 0;
    outBuffer.descriptor.buffer = outBuffer.buffer;
    outBuffer.descriptor.range = VK_WHOLE_SIZE;

    return vkBindBufferMemory(g_vulkanBackend.device, outBuffer.buffer, outBuffer.memory, 0);
}

VkResult
gfx_buffer_create(const VkBufferUsageFlags &usageFlags, const VkMemoryPropertyFlags &memoryPropertyFlags, const VkDeviceSize &size, VkBuffer &outBuffer, VkDeviceMemory &memory,
                  void *inData) {
    VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;
    ASSERT(bufferCreateInfo.size > 0);

    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    const VkResult createBuffer = vkCreateBuffer(g_vulkanBackend.device, &bufferCreateInfo, nullptr, &outBuffer);
    ASSERT(createBuffer == VK_SUCCESS);

    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

    vkGetBufferMemoryRequirements(g_vulkanBackend.device, outBuffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = gfx_utils_get_memory_type(memReqs.memoryTypeBits, memoryPropertyFlags);

    // when outBuffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    const VkResult allocRes = vkAllocateMemory(g_vulkanBackend.device, &memAlloc, nullptr, &memory);
    ASSERT(allocRes == VK_SUCCESS);

    if (inData != nullptr) {
        void *mapped;
        const VkResult VkMapRes = vkMapMemory(g_vulkanBackend.device, memory, 0, size, 0, &mapped);
        ASSERT(VkMapRes == VK_SUCCESS);

        memcpy(mapped, inData, size);
        // when host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mappedRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
            mappedRange.memory = memory;
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(g_vulkanBackend.device, 1, &mappedRange);
        }
        vkUnmapMemory(g_vulkanBackend.device, memory);
    }

    const VkResult bindRes = vkBindBufferMemory(g_vulkanBackend.device, outBuffer, memory, 0);
    return bindRes;
}