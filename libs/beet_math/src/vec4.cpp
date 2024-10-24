#include <beet_math/vec4.h>

//===API================================================================================================================
uint32_t pack_vec4f_to_uint32_t(const vec4f &vec) {
    uint32_t packedValue = {0};
    packedValue |= (uint8_t(vec.x * 0xFF) << 24);
    packedValue |= (uint8_t(vec.y * 0xFF) << 16);
    packedValue |= (uint8_t(vec.z * 0xFF) << 8);
    packedValue |= (uint8_t(vec.w * 0xFF) << 0);
    return packedValue;
}

vec4f unpack_uint32_t_to_vec4f(uint32_t packedValue) {
    return vec4f(
            float((packedValue >> 24) & 0xFF) / 255.0f,
            float((packedValue >> 16) & 0xFF) / 255.0f,
            float((packedValue >> 8) & 0xFF) / 255.0f,
            float((packedValue >> 0) & 0xFF) / 255.0f
    );
}

//======================================================================================================================