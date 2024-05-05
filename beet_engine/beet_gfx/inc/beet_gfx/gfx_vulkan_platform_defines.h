#ifndef BEETROOT_GFX_VULKAN_PLATFORM_DEFINES_H
#define BEETROOT_GFX_VULKAN_PLATFORM_DEFINES_H

#include <cstdint>
#include <beet_shared/platform_defines.h>

#if PLATFORM_WINDOWS

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#define BEET_VK_SURFACE_EXTENSION VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#if PLATFORM_LINUX
#include <vulkan/vulkan_wayland.h>

#define BEET_VK_SURFACE_EXTENSION VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#endif

//===TARGETS============================================================================================================
constexpr VkSurfaceFormatKHR BEET_TARGET_SWAPCHAIN_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
//======================================================================================================================

//===EXTENSIONS=========================================================================================================
static constexpr int32_t BEET_VK_INSTANCE_EXTENSION_COUNT = 4;
static constexpr const char *BEET_VK_INSTANCE_EXTENSIONS[BEET_VK_INSTANCE_EXTENSION_COUNT]{
        VK_KHR_SURFACE_EXTENSION_NAME,
        BEET_VK_SURFACE_EXTENSION,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
};

static constexpr int32_t BEET_VK_MAX_DEVICE_EXTENSION_COUNT = 64;
static constexpr int32_t BEET_VK_REQUIRED_DEVICE_EXTENSION_COUNT = 6;
static constexpr const char* BEET_VK_REQUIRED_DEVICE_EXTENSIONS[7]{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_MAINTENANCE2_EXTENSION_NAME,
        VK_KHR_MULTIVIEW_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
        VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME
};
//======================================================================================================================

//===DEBUG==============================================================================================================
#if BEET_DEBUG
// TODO: Move to command line argument
static constexpr int32_t BEET_DEBUG_VK_FORCE_GPU_SELECTION = -1; // ignore [-1] force select [0..UINT32_MAX]
#endif
//======================================================================================================================

//===VALIDATION=========================================================================================================
static constexpr char BEET_VK_LAYER_VALIDATION[] = "VK_LAYER_KHRONOS_validation";

static constexpr int32_t BEET_VK_VALIDATION_COUNT = 1;
static constexpr const char *beetVulkanValidations[BEET_VK_VALIDATION_COUNT]{
        BEET_VK_LAYER_VALIDATION,
};
//======================================================================================================================

//===API_VERSION========================================================================================================
// to be used for runtime version checking.
static constexpr uint32_t BEET_VK_API_VERSION_1_3 VK_MAKE_API_VERSION(0, 1, 3, 0);
static constexpr uint32_t BEET_VK_API_VERSION_1_2 VK_MAKE_API_VERSION(0, 1, 2, 0);
static constexpr uint32_t BEET_VK_API_VERSION_1_1 VK_MAKE_API_VERSION(0, 1, 1, 0);
static constexpr uint32_t BEET_VK_API_VERSION_1_0 VK_MAKE_API_VERSION(0, 1, 0, 0);

#if defined(VK_VERSION_1_3)
#define BEET_VK_COMPILE_VERSION_1_3 1
#define BEET_VK_COMPILE_VERSION_1_2 1
#define BEET_VK_COMPILE_VERSION_1_1 1
#define BEET_VK_COMPILE_VERSION_1_0 1

#define BEET_MAX_VK_API_VERSION BEET_VK_API_VERSION_1_3
static_assert(BEET_VK_API_VERSION_1_3 == VK_API_VERSION_1_3);

#elif defined(VK_VERSION_1_2)
#define BEET_VK_COMPILE_VERSION_1_3 0
#define BEET_VK_COMPILE_VERSION_1_1 1
#define BEET_VK_COMPILE_VERSION_1_0 1
#define BEET_VK_COMPILE_VERSION_1_2 1

#define BEET_MAX_VK_API_VERSION BEET_VK_API_VERSION_1_2
static_assert(BEET_VK_API_VERSION_1_2 == VK_API_VERSION_1_2);

#elif defined(VK_VERSION_1_1)
#define BEET_VK_COMPILE_VERSION_1_3 0
#define BEET_VK_COMPILE_VERSION_1_2 0
#define BEET_VK_COMPILE_VERSION_1_1 1
#define BEET_VK_COMPILE_VERSION_1_0 1

#define BEET_MAX_VK_API_VERSION VK_API_VERSION_1_1
static_assert(BEET_VK_API_VERSION_1_1 == VK_API_VERSION_1_1);

#elif defined(VK_VERSION_1_0)
#define BEET_VK_COMPILE_VERSION_1_3 0
#define BEET_VK_COMPILE_VERSION_1_2 0
#define BEET_VK_COMPILE_VERSION_1_1 0
#define BEET_VK_COMPILE_VERSION_1_0 1

#define BEET_MAX_VK_API_VERSION VK_API_VERSION_1_0
static_assert(BEET_VK_API_VERSION_1_0 == VK_API_VERSION_1_0);

#else
SANITY_CHECK()
#endif
//======================================================================================================================

#endif //BEETROOT_GFX_VULKAN_PLATFORM_DEFINES_H
