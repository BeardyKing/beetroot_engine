#include <beet_gfx/gfx_samplers.h>
#include <beet_gfx/gfx_types.h>

#include <beet_shared/texture_formats.h>
#include <beet_shared/assert.h>

extern VulkanBackend g_vulkanBackend;

static TextureSamplers g_textureSamplers = {};

TextureSamplers *gfx_samplers() {
    return &g_textureSamplers;
}

void gfx_build_samplers() {
    {
        VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = BEET_MAX_MIP_COUNT;
        VkResult linearResult = vkCreateSampler(g_vulkanBackend.device, &samplerInfo, nullptr, &g_textureSamplers.samplers[TextureSamplerType::Linear]);
        ASSERT_MSG(linearResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
    {
        VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = BEET_MAX_MIP_COUNT;
        VkResult pointResult = vkCreateSampler(g_vulkanBackend.device, &samplerInfo, nullptr, &g_textureSamplers.samplers[TextureSamplerType::Point]);
        ASSERT_MSG(pointResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
}

void cleanup_samplers() {
    vkDestroySampler(g_vulkanBackend.device, g_textureSamplers.samplers[TextureSamplerType::Linear], nullptr);
    g_textureSamplers.samplers[TextureSamplerType::Linear] = VK_NULL_HANDLE;

    vkDestroySampler(g_vulkanBackend.device, g_textureSamplers.samplers[TextureSamplerType::Point], nullptr);
    g_textureSamplers.samplers[TextureSamplerType::Point] = VK_NULL_HANDLE;
}

void gfx_create_samplers() {
    gfx_build_samplers();
}

void gfx_cleanup_samplers() {
    cleanup_samplers();
}