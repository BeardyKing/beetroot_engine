#ifndef BEETROOT_GFX_COMMAND_H
#define BEETROOT_GFX_COMMAND_H

#include <vulkan/vulkan_core.h>

//===API================================================================================================================
void gfx_command_begin_immediate_recording();
void gfx_command_end_immediate_recording();

void gfx_command_begin_rendering(VkCommandBuffer &cmdBuffer, const VkRenderingInfoKHR &renderingInfo);
void gfx_command_end_rendering(VkCommandBuffer &cmdBuffer);

void gfx_command_insert_memory_barrier(
        VkCommandBuffer &cmdBuffer,
        const VkImage &image,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkImageSubresourceRange subresourceRange);
//======================================================================================================================

#endif //BEETROOT_GFX_COMMAND_H
