#ifndef BEETROOT_GFX_INTERFACE_H
#define BEETROOT_GFX_INTERFACE_H

//===api=====================
void gfx_update(const double &deltaTime);

//===init & shutdown=========
void gfx_create(void *windowHandle);
void gfx_cleanup();

#endif //BEETROOT_GFX_INTERFACE_H
