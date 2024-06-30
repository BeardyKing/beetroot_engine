#include <beet_gfx/gfx_samplers.h>
#include <beet_gfx/gfx_types.h>

#include <beet_shared/texture_formats.h>
#include <beet_shared/assert.h>

//===INTERNAL_STRUCTS===================================================================================================
extern VulkanBackend g_vulkanBackend;
static TextureSamplers s_textureSamplers = {};
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static void gfx_build_samplers() {
    {
        const VkSamplerCreateInfo samplerInfo = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_TRUE,
                .maxAnisotropy = 16,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod = 0.0f,
                .maxLod = BEET_MAX_MIP_COUNT,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
        };
        const VkResult linearResult = vkCreateSampler(g_vulkanBackend.device, &samplerInfo, nullptr, &s_textureSamplers.samplers[TextureSamplerType::LinearRepeat]);
        ASSERT_MSG(linearResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
    {
        const VkSamplerCreateInfo samplerInfo = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_TRUE,
                .maxAnisotropy = 16,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod = 0.0f,
                .maxLod = BEET_MAX_MIP_COUNT,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
        };
        const VkResult linearResult = vkCreateSampler(g_vulkanBackend.device, &samplerInfo, nullptr, &s_textureSamplers.samplers[TextureSamplerType::LinearMirror]);
        ASSERT_MSG(linearResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
    {
        const VkSamplerCreateInfo samplerInfo = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_NEAREST,
                .minFilter = VK_FILTER_NEAREST,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_TRUE,
                .maxAnisotropy = 16,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod = 0.0f,
                .maxLod = BEET_MAX_MIP_COUNT,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
        };
        const VkResult pointResult = vkCreateSampler(g_vulkanBackend.device, &samplerInfo, nullptr, &s_textureSamplers.samplers[TextureSamplerType::PointRepeat]);
        ASSERT_MSG(pointResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
    {
        const VkSamplerCreateInfo samplerInfo = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_FALSE,
                .maxAnisotropy = 1,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod = 0.0f,
                .maxLod = 1.0f,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
        };
        const VkResult pointResult = vkCreateSampler(g_vulkanBackend.device, &samplerInfo, nullptr, &s_textureSamplers.samplers[TextureSamplerType::DepthStencil]);
        ASSERT_MSG(pointResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
}

static void cleanup_samplers() {
    vkDestroySampler(g_vulkanBackend.device, s_textureSamplers.samplers[TextureSamplerType::LinearRepeat], nullptr);
    s_textureSamplers.samplers[TextureSamplerType::LinearRepeat] = VK_NULL_HANDLE;

    vkDestroySampler(g_vulkanBackend.device, s_textureSamplers.samplers[TextureSamplerType::LinearMirror], nullptr);
    s_textureSamplers.samplers[TextureSamplerType::LinearMirror] = VK_NULL_HANDLE;

    vkDestroySampler(g_vulkanBackend.device, s_textureSamplers.samplers[TextureSamplerType::PointRepeat], nullptr);
    s_textureSamplers.samplers[TextureSamplerType::PointRepeat] = VK_NULL_HANDLE;

    vkDestroySampler(g_vulkanBackend.device, s_textureSamplers.samplers[TextureSamplerType::DepthStencil], nullptr);
    s_textureSamplers.samplers[TextureSamplerType::DepthStencil] = VK_NULL_HANDLE;
}
//======================================================================================================================

//===API================================================================================================================
TextureSamplers *gfx_samplers() {
    return &s_textureSamplers;
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_samplers() {
    gfx_build_samplers();
}

void gfx_cleanup_samplers() {
    cleanup_samplers();
}
//======================================================================================================================
