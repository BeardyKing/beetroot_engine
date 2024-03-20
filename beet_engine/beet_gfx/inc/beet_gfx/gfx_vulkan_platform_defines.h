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

//===extensions==============
static constexpr int32_t BEET_VK_EXTENSION_COUNT = 3;
static constexpr const char *BEET_VK_EXTENSIONS[BEET_VK_EXTENSION_COUNT]{
        VK_KHR_SURFACE_EXTENSION_NAME,
        BEET_VK_SURFACE_EXTENSION,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};

//==debug====================
#if BEET_DEBUG
// TODO: Move to command line argument
static constexpr int32_t BEET_DEBUG_VK_FORCE_GPU_SELECTION = -1; // ignore [-1] force select [0..UINT32_MAX]
#endif

//===validation==============
static constexpr char BEET_VK_LAYER_VALIDATION[] = "VK_LAYER_KHRONOS_validation";

static constexpr int32_t BEET_VK_VALIDATION_COUNT = 1;
static constexpr const char *beetVulkanValidations[BEET_VK_EXTENSION_COUNT]{
        BEET_VK_LAYER_VALIDATION,
};

//===debug msg callbacks=====
static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_Func;
static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT_Func;
static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT_Func;

static constexpr char BEET_VK_CREATE_DEBUG_UTIL_EXT[] = "vkCreateDebugUtilsMessengerEXT";
static constexpr char BEET_VK_DESTROY_DEBUG_UTIL_EXT[] = "vkDestroyDebugUtilsMessengerEXT";
static constexpr char BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT[] = "vkSetDebugUtilsObjectNameEXT";

static constexpr VkDebugUtilsMessageSeverityFlagsEXT BEET_VK_DEBUG_UTILS_MESSAGE_SEVERITY =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

static constexpr VkDebugUtilsMessageTypeFlagsEXT BEET_VK_DEBUG_UTILS_MESSAGE_TYPE =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

//===api version=============
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

#endif //BEETROOT_GFX_VULKAN_PLATFORM_DEFINES_H
