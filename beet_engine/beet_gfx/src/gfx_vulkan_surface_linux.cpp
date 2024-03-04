#include <beet_shared/platform_defines.h>

#if PLATFORM_LINUX

#include <beet_gfx/gfx_vulkan_surface.h>
#include <beet_shared/assert.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

void gfx_create_surface(void *windowHandle, const VkInstance *instance, VkSurfaceKHR *outSurface) {
    VkWaylandSurfaceCreateInfoKHR surfaceInfo = {
            VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR
    };

    //TODO: Linux Add wayland windowing
    ASSERT_MSG(windowHandle != nullptr, "Err: invalid window handle");
    surfaceInfo.display = (wl_display *) windowHandle; //handle?
    surfaceInfo.surface = nullptr;

    VkResult result = vkCreateWaylandSurfaceKHR(*instance, &surfaceInfo, nullptr, outSurface);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create vulkan surface");
}
// cleanup exists in gfx_backed_vulkan as it doesn't use any windows APIs

#endif
