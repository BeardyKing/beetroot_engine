#ifndef BEETROOT_GFX_COMMAND_H
#define BEETROOT_GFX_COMMAND_H

#include <vulkan/vulkan_core.h>

void gfx_command_begin_immediate_recording();
void gfx_command_end_immediate_recording();

void gfx_command_begin_rendering(VkCommandBuffer &cmdBuffer, const VkRenderingInfo &renderingInfo);
void gfx_command_end_rendering(VkCommandBuffer &cmdBuffer);

#endif //BEETROOT_GFX_COMMAND_H
