#ifndef BEETROOT_GFX_SAMPLERS_H
#define BEETROOT_GFX_SAMPLERS_H

#include <vulkan/vulkan_core.h>

//===PUBLIC_STRUCTS=====================================================================================================
enum TextureSamplerType : int32_t {
    Invalid = 0,
    LinearRepeat = 1,
    LinearMirror = 2,
    PointRepeat = 3,

    COUNT,
};

struct TextureSamplers {
    VkSampler samplers[TextureSamplerType::COUNT];
};
//======================================================================================================================

//===API================================================================================================================
TextureSamplers *gfx_samplers();
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_samplers();
void gfx_cleanup_samplers();
//===INIT_&_SHUTDOWN====================================================================================================

#endif //BEETROOT_GFX_SAMPLERS_H
