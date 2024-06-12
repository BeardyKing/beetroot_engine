#ifndef BEETROOT_GFX_INTERFACE_H
#define BEETROOT_GFX_INTERFACE_H

#include <cstdint>

//===API================================================================================================================
void gfx_update(const double &deltaTime);
uint32_t gfx_buffer_index();
uint32_t gfx_swap_chain_index();
uint32_t gfx_last_swap_chain_index();
vec2i gfx_screen_size();
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create(void *windowHandle);
void gfx_cleanup();
//======================================================================================================================

#endif //BEETROOT_GFX_INTERFACE_H
