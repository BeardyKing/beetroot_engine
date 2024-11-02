#ifndef BEETROOT_GFX_IMGUI_H
#define BEETROOT_GFX_IMGUI_H

#include <vulkan/vulkan_core.h>
#include <beet_shared/feature_defines.h>

#if CHECK_FEATURE(FEATURE_GFX_IMGUI)

//===API================================================================================================================
void gfx_imgui_begin();
void gfx_imgui_end();
void gfx_imgui_draw(VkCommandBuffer &cmdBuffer);

void gfx_imgui_demo_window();
void *gfx_imgui_get_proc_function_pointer();
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_imgui(void *windowHandle);
void gfx_cleanup_imgui();
//======================================================================================================================

#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)
#endif //BEETROOT_GFX_IMGUI_H
