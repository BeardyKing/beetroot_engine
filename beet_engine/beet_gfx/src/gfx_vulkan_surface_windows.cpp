#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <beet_gfx/gfx_vulkan_surface.h>
#include <beet_shared/assert.h>

void gfx_create_surface(void *windowHandle, const VkInstance *instance, VkSurfaceKHR *outSurface) {
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {
            VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR
    };

    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = HWND(windowHandle);

    VkResult result = vkCreateWin32SurfaceKHR(*instance, &surfaceInfo, nullptr, outSurface);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create vulkan surface");
}

// cleanup exists in gfx_backed_vulkan as it doesn't use any windows APIs
