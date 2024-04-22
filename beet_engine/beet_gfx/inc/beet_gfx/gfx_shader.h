#ifndef BEETROOT_GFX_SHADER_H
#define BEETROOT_GFX_SHADER_H

#include <vulkan/vulkan.h>

//===API================================================================================================================
VkShaderModule gfx_load_shader_binary(const char *path);
VkPipelineShaderStageCreateInfo gfx_load_shader(const char *path, VkShaderStageFlagBits stage);
//======================================================================================================================

#endif //BEETROOT_GFX_SHADER_H
