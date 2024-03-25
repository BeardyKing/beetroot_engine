#include <beet_gfx/gfx_command.h>
#include <beet_gfx/gfx_types.h>

#include <vulkan/vulkan_core.h>

extern VulkanBackend g_vulkanBackend;
extern PFN_vkCmdBeginRenderingKHR g_vkCmdBeginRenderingKHR_Func;
extern PFN_vkCmdEndRenderingKHR g_vkCmdEndRenderingKHR_Func;

void gfx_command_begin_immediate_recording() {
    VkCommandBufferBeginInfo cmdBufBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(g_vulkanBackend.immediateCommandBuffer, &cmdBufBeginInfo);
}

void gfx_command_end_immediate_recording() {
    vkEndCommandBuffer(g_vulkanBackend.immediateCommandBuffer);

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_vulkanBackend.immediateCommandBuffer;

    //TODO:GFX replace immediate submit with transfer immediate submit.
    vkQueueSubmit(g_vulkanBackend.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_vulkanBackend.queue);
}

void gfx_command_begin_rendering(VkCommandBuffer &cmdBuffer, const VkRenderingInfo &renderingInfo) {
    g_vkCmdBeginRenderingKHR_Func(cmdBuffer, &renderingInfo);
}

void gfx_command_end_rendering(VkCommandBuffer &cmdBuffer) {
    g_vkCmdEndRenderingKHR_Func(cmdBuffer);
}