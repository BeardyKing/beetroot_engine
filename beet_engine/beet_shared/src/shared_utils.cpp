#include <beet_shared/shared_utils.h>

#include <cassert>
#include <cstdlib>

//===API================================================================================================================
size_t align_to(size_t value, size_t alignment) {
    ASSERT_MSG(alignment > 0, "Err: Alignment must be a positive non-zero value")
    const size_t aligned_value = ((value + alignment - 1) / alignment) * alignment;
    return aligned_value;
}

float align_to(float value, float alignment) {
    ASSERT_MSG(alignment > 0, "Err: Alignment must be a positive non-zero value")
    float aligned_value = ceilf(value / alignment) * alignment;
    return aligned_value;
}

uint32_t count_set_bits(const uint32_t n) {
    uint32_t target = n;
    uint32_t count = 0;
    while (target) {
        count++;
        target >>= 1;
    }
    return count;
}

uint32_t colour_set_red(uint32_t color, const uint8_t inRed) {
    color &= 0x00FFFFFF;
    color |= (uint32_t) inRed << 24;
    return color;
}

uint32_t colour_set_green(uint32_t color, const uint8_t inGreen) {
    color &= 0xFF00FFFF;
    color |= (uint32_t) inGreen << 16;
    return color;
}

uint32_t colour_set_blue(uint32_t color, const uint8_t inBlue) {
    color &= 0xFFFF00FF;
    color |= (uint32_t) inBlue << 8;
    return color;
}

uint32_t colour_set_alpha(uint32_t color, const uint8_t inAlpha) {
    color &= 0xFFFFFF00;
    color |= (uint32_t) inAlpha;
    return color;
}
//======================================================================================================================