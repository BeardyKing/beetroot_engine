#include <cstdlib>
#include <vulkan/vulkan.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>
#include <beet_gfx/gfx_vulkan_platform_defines.h>
#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/gfx_vulkan_surface.h>

static const char *BEET_VK_PHYSICAL_DEVICE_TYPE_MAPPING[] = {
        "VK_PHYSICAL_DEVICE_TYPE_OTHER",
        "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_CPU",
};

static struct UserArguments {
    uint32_t selectedPhysicalDeviceIndex = {};
    bool vsync = {true};
} g_userArguments;

struct VulkanSwapChain {
    VkSurfaceKHR surface;
};

static struct VulkanBackend {
    VkInstance instance = {VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice = {VK_NULL_HANDLE};
    VkPhysicalDeviceProperties deviceProperties = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};

    VkExtensionProperties *supportedExtensions = {};
    uint32_t extensionsCount = {};
    VkLayerProperties *supportedValidationLayers = {};
    uint32_t validationLayersCount = {};

    VulkanSwapChain swapChain = {};

} g_vulkanBackend;

bool gfx_find_supported_extension(const char *extensionName) {
    for (uint32_t i = 0; i < g_vulkanBackend.extensionsCount; ++i) {
        if (strcmp(g_vulkanBackend.supportedExtensions[i].extensionName, extensionName) == 0) {
            return true;
        }
    }
    return false;
}

bool gfx_find_supported_validation(const char *layerName) {
    for (uint32_t i = 0; i < g_vulkanBackend.validationLayersCount; ++i) {
        if (strcmp(g_vulkanBackend.supportedValidationLayers[i].layerName, layerName) == 0) {
            return true;
        }
    }
    return false;
}

void gfx_cleanup_instance() {
    ASSERT_MSG(g_vulkanBackend.instance != VK_NULL_HANDLE, "Err: VkInstance has already been destroyed");
    vkDestroyInstance(g_vulkanBackend.instance, nullptr);
    g_vulkanBackend.instance = VK_NULL_HANDLE;
}

void gfx_create_instance() {
    vkEnumerateInstanceExtensionProperties(nullptr, &g_vulkanBackend.extensionsCount, nullptr);
    g_vulkanBackend.supportedExtensions = (VkExtensionProperties *) malloc(sizeof(VkExtensionProperties) * g_vulkanBackend.extensionsCount);
    if (g_vulkanBackend.extensionsCount > 0) {
        vkEnumerateInstanceExtensionProperties(nullptr, &g_vulkanBackend.extensionsCount, g_vulkanBackend.supportedExtensions);
    }

    for (uint32_t i = 0; i < g_vulkanBackend.extensionsCount; ++i) {
        log_verbose(MSG_GFX, "Extension: %s \n", g_vulkanBackend.supportedExtensions[i].extensionName);
    }

    for (uint8_t i = 0; i < BEET_VK_EXTENSION_COUNT; i++) {
        const bool result = gfx_find_supported_extension(BEET_VK_EXTENSIONS[i]);
        ASSERT_MSG(result, "Err: failed find support for extension [%s]", BEET_VK_EXTENSIONS[i]);
    }

    vkEnumerateInstanceLayerProperties(&g_vulkanBackend.validationLayersCount, nullptr);
    g_vulkanBackend.supportedValidationLayers = (VkLayerProperties *) malloc(sizeof(VkLayerProperties) * g_vulkanBackend.validationLayersCount);
    if (g_vulkanBackend.validationLayersCount > 0) {
        vkEnumerateInstanceLayerProperties(&g_vulkanBackend.validationLayersCount, g_vulkanBackend.supportedValidationLayers);
    }

    for (uint32_t i = 0; i < g_vulkanBackend.validationLayersCount; ++i) {
        log_verbose(MSG_GFX, "Layer: %s - Desc: %s \n",
                    g_vulkanBackend.supportedValidationLayers[i].layerName,
                    g_vulkanBackend.supportedValidationLayers[i].description);
    }

    for (uint8_t i = 0; i < BEET_VK_VALIDATION_COUNT; i++) {
        const bool result = gfx_find_supported_validation(beetVulkanValidations[i]);
        ASSERT_MSG(result, "Err: failed find support for validation layer [%s]", beetVulkanValidations[i]);
    }

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "VK_BEETROOT_ENGINE";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "VK_BEETROOT_ENGINE";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = BEET_MAX_VK_API_VERSION;

    VkInstanceCreateInfo instInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = BEET_VK_EXTENSION_COUNT;
    instInfo.ppEnabledExtensionNames = BEET_VK_EXTENSIONS;
    instInfo.enabledLayerCount = BEET_VK_VALIDATION_COUNT;
    instInfo.ppEnabledLayerNames = beetVulkanValidations;

    const auto result = vkCreateInstance(&instInfo, nullptr, &g_vulkanBackend.instance);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create vulkan instance");
}

void gfx_cleanup_physical_device() {
    ASSERT_MSG(g_vulkanBackend.physicalDevice != VK_NULL_HANDLE, "Err: VkPhysicalDevice has already been invalidated");
    g_vulkanBackend.physicalDevice = VK_NULL_HANDLE;
}

void gfx_create_physical_device() {
    uint32_t deviceCount = 0;

    vkEnumeratePhysicalDevices(g_vulkanBackend.instance, &deviceCount, nullptr);
    ASSERT_MSG(deviceCount != 0, "Err: did not find any vulkan compatible physical devices");

    VkPhysicalDevice *physicalDevices = (VkPhysicalDevice *) malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(g_vulkanBackend.instance, &deviceCount, physicalDevices);

    //TODO:GFX: Add fallback support for `best` GPU based on intended workload, if no argument is provided we fallback to device [0]
    uint32_t selectedDevice = g_userArguments.selectedPhysicalDeviceIndex;
#if BEET_DEBUG
    if (BEET_DEBUG_VK_FORCE_GPU_SELECTION > -1) {
        selectedDevice = BEET_DEBUG_VK_FORCE_GPU_SELECTION;
        log_warning(MSG_GFX, "Using debug ONLY feature `BEET_VK_FORCE_GPU_SELECTION` selecting device [%i]\n", selectedDevice)
    }
#endif

    ASSERT_MSG(selectedDevice < deviceCount, "Err: selecting physical vulkan device that is out of range");
    g_vulkanBackend.physicalDevice = physicalDevices[selectedDevice];

    vkGetPhysicalDeviceProperties(g_vulkanBackend.physicalDevice, &g_vulkanBackend.deviceProperties);
    vkGetPhysicalDeviceFeatures(g_vulkanBackend.physicalDevice, &g_vulkanBackend.deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(g_vulkanBackend.physicalDevice, &g_vulkanBackend.deviceMemoryProperties);

    const uint32_t apiVersionMajor = VK_API_VERSION_MAJOR(g_vulkanBackend.deviceProperties.apiVersion);
    const uint32_t apiVersionMinor = VK_API_VERSION_MINOR(g_vulkanBackend.deviceProperties.apiVersion);
    const uint32_t apiVersionPatch = VK_API_VERSION_PATCH(g_vulkanBackend.deviceProperties.apiVersion);

    const uint32_t driverVersionMajor = VK_API_VERSION_MAJOR(g_vulkanBackend.deviceProperties.driverVersion);
    const uint32_t driverVersionMinor = VK_API_VERSION_MINOR(g_vulkanBackend.deviceProperties.driverVersion);
    const uint32_t driverVersionPatch = VK_API_VERSION_PATCH(g_vulkanBackend.deviceProperties.driverVersion);

    const char *deviceType = BEET_VK_PHYSICAL_DEVICE_TYPE_MAPPING[g_vulkanBackend.deviceProperties.deviceType];

    log_info(MSG_GFX, "\nName:\t\t%s \nType:\t\t%s \nVersion:\t%u.%u.%u \nDriver: \t%u.%u.%u\n",
             g_vulkanBackend.deviceProperties.deviceName,
             deviceType,
             apiVersionMajor,
             apiVersionMinor,
             apiVersionPatch,
             driverVersionMajor,
             driverVersionMinor,
             driverVersionPatch
    );

    free(physicalDevices);
}

void gfx_cleanup_surface() {
    // gfx_create_surface(...) exists in gfx_vulkan_surface.h / gfx_vulkan_surface_windows.cpp
    ASSERT_MSG(g_vulkanBackend.swapChain.surface != VK_NULL_HANDLE, "Err: VkSurface has already been destroyed");
    vkDestroySurfaceKHR(g_vulkanBackend.instance, g_vulkanBackend.swapChain.surface, nullptr);
    g_vulkanBackend.swapChain.surface = VK_NULL_HANDLE;
}

void gfx_update(const double &deltaTime) {}

void gfx_create(void *windowHandle) {
    gfx_create_instance();
    gfx_create_physical_device();
    gfx_create_surface(windowHandle, &g_vulkanBackend.instance, &g_vulkanBackend.swapChain.surface);
}

void gfx_cleanup() {
    gfx_cleanup_surface();
    gfx_cleanup_physical_device();
    gfx_cleanup_instance();

    free(g_vulkanBackend.supportedExtensions);
    free(g_vulkanBackend.supportedValidationLayers);
}