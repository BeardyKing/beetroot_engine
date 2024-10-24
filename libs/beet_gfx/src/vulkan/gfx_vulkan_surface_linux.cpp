#ifndef KAH_GFX_VULKAN_SURFACE_LINUX_C_H
#define KAH_GFX_VULKAN_SURFACE_LINUX_C_H


//===API================================================================================================================
#include <beet_shared/feature_defines.h>

#if CHECK_FEATURE(FEATURE_PLATFORM_LINUX)

#include <beet_gfx/vulkan/gfx_vulkan_surface.h>
#include <beet_shared/assert.h>

#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

struct XLibHandles // must mirror struct in kah_core/src/window_linux.c
{
    Display* display;
    Window window;
};

void gfx_create_surface(void* windowAndDisplayHandle, const VkInstance* instance, VkSurfaceKHR* outSurface)
{
    ASSERT_MSG(windowAndDisplayHandle != nullptr, "Err: invalid window & display struct");
    struct XLibHandles* xLibHandles = (struct XLibHandles*)windowAndDisplayHandle;

    VkXlibSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = xLibHandles->display,
        .window = xLibHandles->window,
    };

    VkResult result = vkCreateXlibSurfaceKHR(*instance, &surfaceInfo, nullptr, outSurface);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create vulkan surface");
}

// cleanup exists in gfx_backed_vulkan as it doesn't use any XLib APIs

//======================================================================================================================


#endif
#endif //KAH_GFX_VULKAN_SURFACE_LINUX_C_H
