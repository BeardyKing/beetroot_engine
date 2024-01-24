#ifndef BEETROOT_GFX_SAMPLERS_H
#define BEETROOT_GFX_SAMPLERS_H

#include <vulkan/vulkan_core.h>

enum TextureSamplerType{
    Linear = 0,
    Point = 1,

    COUNT,
};

struct TextureSamplers {
    VkSampler samplers[TextureSamplerType::COUNT];
};

TextureSamplers* gfx_samplers();

void gfx_create_samplers();
void gfx_cleanup_samplers();

#endif //BEETROOT_GFX_SAMPLERS_H
