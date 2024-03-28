#ifndef BEETROOT_GFX_IMGUI_H
#define BEETROOT_GFX_IMGUI_H

//TODO: will extend to EDITOR/RUNTIME instead of DEBUG/RELEASE.
#include <vulkan/vulkan_core.h>

#define BEET_GFX_IMGUI BEET_DEBUG

#if BEET_GFX_IMGUI

//===api=====================
void gfx_imgui_begin();
void gfx_imgui_end();
void gfx_imgui_draw(VkCommandBuffer &cmdBuffer);

void gfx_imgui_demo_window();
void* gfx_imgui_get_win32_proc_function_pointer();

//===init & shutdown=========
void gfx_create_imgui(void* windowHandle);
void gfx_cleanup_imgui();

#endif //BEET_GFX_IMGUI
#endif //BEETROOT_GFX_IMGUI_H
