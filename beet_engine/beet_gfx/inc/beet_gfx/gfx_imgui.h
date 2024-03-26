#ifndef BEETROOT_GFX_IMGUI_H
#define BEETROOT_GFX_IMGUI_H

//TODO: will extend to EDITOR/RUNTIME instead of DEBUG/RELEASE.
#include <vulkan/vulkan_core.h>

#define BEET_GFX_IMGUI BEET_DEBUG

#if BEET_GFX_IMGUI

void gfx_create_imgui(void* windowHandle);
void gfx_cleanup_imgui();

void gfx_imgui_begin();
void gfx_imgui_end();
void gfx_imgui_draw(VkCommandBuffer &cmdBuffer);

void gfx_imgui_demo_window();

#endif

#endif //BEETROOT_GFX_IMGUI_H
