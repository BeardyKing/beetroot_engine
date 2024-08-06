#ifndef BEETROOT_SHARED_UTILS_H
#define BEETROOT_SHARED_UTILS_H

#include <cstdint>

//===API================================================================================================================
size_t align_to(size_t value, size_t alignment);
float align_to(float value, float alignment);

uint32_t count_set_bits(uint32_t n);

uint32_t colour_set_red(uint32_t color, uint8_t inRed);
uint32_t colour_set_green(uint32_t color, uint8_t inGreen);
uint32_t colour_set_blue(uint32_t color, uint8_t inBlue);
uint32_t colour_set_alpha(uint32_t color, uint8_t inAlpha);
//======================================================================================================================

#endif //BEETROOT_SHARED_UTILS_H
