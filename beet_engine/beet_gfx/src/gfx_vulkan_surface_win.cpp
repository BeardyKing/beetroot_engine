#include <beet_shared/platform_defines.h>

#if PLATFORM_WINDOWS

#include <beet_shared/assert.h>
#include <beet_gfx/gfx_vulkan_surface.h>

#include <Windows.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
void gfx_create_surface(void *windowHandle, const VkInstance *instance, VkSurfaceKHR *outSurface) {
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {
            VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR
    };

    ASSERT_MSG(windowHandle != nullptr, "Err: invalid window handle");
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = HWND(windowHandle);

    VkResult result = vkCreateWin32SurfaceKHR(*instance, &surfaceInfo, nullptr, outSurface);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create vulkan surface");
}
// cleanup exists in gfx_backed_vulkan as it doesn't use any windows APIs

#endif