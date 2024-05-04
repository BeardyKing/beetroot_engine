#ifndef BEETROOT_BEET_TYPES_H
#define BEETROOT_BEET_TYPES_H

#include <beet_math/vec3.h>
#include <beet_math/vec4.h>
#include <beet_math/vec2.h>
#include <cstdint>

constexpr uint8_t DEBUG_NAME_MAX = 128;

//===PUBLIC_STRUCTS=====================================================================================================
struct LitEntity {
    uint32_t transformIndex;
    uint32_t meshIndex;
    uint32_t materialIndex;
#if BEET_DEBUG
    char debug_name[DEBUG_NAME_MAX] = {"unnamed\0"};
#endif //BEET_DEBUG
};

struct SkyEntity {
    uint32_t meshIndex;
    uint32_t materialIndex;
};

struct CameraEntity {
    uint32_t transformIndex;
    uint32_t cameraIndex;
#if BEET_DEBUG
    char debug_name[DEBUG_NAME_MAX] = {"default camera\0"};
#endif //BEET_DEBUG
};

struct Transform {
    vec3f position{0.0f, 0.0f, 0.0f};
    vec3f rotation{0.0f, 0.0f, 0.0f};
    vec3f scale{1.0f, 1.0f, 1.0f};
};

struct Camera {
    float fov{60.0f};
    float zNear{0.1f};
    float zFar{800.0f};
};

struct LitMaterial {
    uint32_t descriptorSetIndex{0};
    uint32_t albedoIndex{0};
    //TODO:GFX
    //uint32_t normalIndex{0};
    //uint32_t metallicIndex{0};
    //uint32_t roughnessIndex{0};
    //uint32_t occlusionIndex{0};
    //
    //vec4f albedoColor{1.0f, 1.0f, 1.0f, 1.0f};
    //vec2f textureTiling{1.0f, 1.0f};
    //
    //float albedoScalar{1.0f};
    //float normalScalar{1.0f};
    //float metallicScalar{1.0f};
    //float roughnessScalar{1.0f};
    //float occlusionScalar{1.0f};
};

struct SkyMaterial {
    uint32_t descriptorSetIndex{0};
    uint32_t octahedralMapIndex{0};
};
//======================================================================================================================

#endif //BEETROOT_BEET_TYPES_H
