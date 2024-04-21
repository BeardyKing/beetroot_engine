#include <beet_gfx/gfx_shader.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_converter.h>

#include <beet_shared/assert.h>

#include <vulkan/vulkan_core.h>

#include <fstream>

extern VulkanBackend g_vulkanBackend;

VkShaderModule gfx_load_shader_binary(const char *path) {
    //TODO Refactor this to using a new binary FS api that uses fstat as we shouldn't use tellg on a binary.
    std::ifstream is(path, std::ios::binary | std::ios::in | std::ios::ate);
    ASSERT_MSG(is.is_open(), "Err: Failed to open shader %s", path);
    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);
    char *shaderCode = (char *) malloc(sizeof(char) * size);
    is.read(shaderCode, size);
    is.close();
    ASSERT(size > 0);

    VkShaderModuleCreateInfo moduleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = size,
            .pCode = (uint32_t *) shaderCode,
    };
    VkShaderModule shaderModule = {};
    const VkResult moduleResult = vkCreateShaderModule(g_vulkanBackend.device, &moduleCreateInfo, nullptr, &shaderModule);
    ASSERT(moduleResult == VK_SUCCESS);

    free(shaderCode);
    return shaderModule;
}

VkPipelineShaderStageCreateInfo gfx_load_shader(const char *path, VkShaderStageFlagBits stage) {
#if BEET_CONVERT_ON_DEMAND
    gfx_convert_shader_spv(path);
#endif //BEET_CONVERT_ON_DEMAND

    VkPipelineShaderStageCreateInfo shaderStage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage,
            .module = gfx_load_shader_binary(path),
            .pName = "main",
    };
    assert(shaderStage.module != VK_NULL_HANDLE);
    return shaderStage;
}