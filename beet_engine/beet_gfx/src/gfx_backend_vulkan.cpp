#include <vulkan/vulkan.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>
#include <beet_shared/shared_utils.h>
#include <beet_shared/memory.h>
#include <beet_shared/texture_formats.h>
#include <beet_shared/dds_loader.h>
#include <beet_shared/c_string.h>
#include <beet_shared/db_types.h>

#include <beet_gfx/gfx_vulkan_platform_defines.h>
#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/gfx_vulkan_surface.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_samplers.h>
#include <beet_gfx/gfx_mesh.h>
#include <beet_gfx/gfx_buffer.h>
#include <beet_gfx/gfx_command.h>
#include <beet_gfx/gfx_utils.h>
#include <beet_gfx/gfx_function_pointers.h>
#include <beet_gfx/gfx_debug.h>
#include <beet_gfx/gfx_imgui.h>

#include <beet_math/quat.h>
#include <beet_math/utilities.h>

#include <fstream>
#include <cstring>
#include "beet_gfx/gfx_lit.h"

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
} g_userArguments = {};


//TODO: replace with db arr of structs with id lookups
static struct Objects {
    struct CamObj {
        Camera cam;
        Transform transform;
    } camObj;
} g_dbEntities = {};

VulkanBackend g_vulkanBackend = {};
TargetVulkanFormats g_vulkanTargetFormats = {};
GlobTextures g_textures = {};
GlobMeshes g_meshes = {};

bool gfx_find_supported_extension(const char *extensionName) {
    for (uint32_t i = 0; i < g_vulkanBackend.extensionsCount; ++i) {
        if (c_str_equal(g_vulkanBackend.supportedExtensions[i].extensionName, extensionName)) {
            return true;
        }
    }
    return false;
}

bool gfx_find_supported_validation(const char *layerName) {
    for (uint32_t i = 0; i < g_vulkanBackend.validationLayersCount; ++i) {
        if (c_str_equal(g_vulkanBackend.supportedValidationLayers[i].layerName, layerName)) {
            return true;
        }
    }
    return false;
}

void gfx_cleanup_instance() {
    ASSERT_MSG(g_vulkanBackend.instance != VK_NULL_HANDLE, "Err: VkInstance has already been destroyed");
    vkDestroyInstance(g_vulkanBackend.instance, nullptr);
    g_vulkanBackend.instance = VK_NULL_HANDLE;

    mem_free(g_vulkanBackend.supportedExtensions);
    mem_free(g_vulkanBackend.supportedValidationLayers);
}

void gfx_create_instance() {
    vkEnumerateInstanceExtensionProperties(nullptr, &g_vulkanBackend.extensionsCount, nullptr);
    ASSERT(g_vulkanBackend.supportedExtensions == nullptr);
    g_vulkanBackend.supportedExtensions = (VkExtensionProperties *) mem_zalloc(sizeof(VkExtensionProperties) * g_vulkanBackend.extensionsCount);
    if (g_vulkanBackend.extensionsCount > 0) {
        vkEnumerateInstanceExtensionProperties(nullptr, &g_vulkanBackend.extensionsCount, g_vulkanBackend.supportedExtensions);
    }

    for (uint32_t i = 0; i < g_vulkanBackend.extensionsCount; ++i) {
        log_verbose(MSG_GFX, "Instance extension: %s \n", g_vulkanBackend.supportedExtensions[i].extensionName);
    }

    for (uint8_t i = 0; i < BEET_VK_INSTANCE_EXTENSION_COUNT; i++) {
        const bool result = gfx_find_supported_extension(BEET_VK_INSTANCE_EXTENSIONS[i]);
        ASSERT_MSG(result, "Err: failed find support for extension [%s]", BEET_VK_INSTANCE_EXTENSIONS[i]);
    }

    vkEnumerateInstanceLayerProperties(&g_vulkanBackend.validationLayersCount, nullptr);
    ASSERT(g_vulkanBackend.supportedValidationLayers == nullptr);
    g_vulkanBackend.supportedValidationLayers = (VkLayerProperties *) mem_zalloc(sizeof(VkLayerProperties) * g_vulkanBackend.validationLayersCount);
    if (g_vulkanBackend.validationLayersCount > 0) {
        vkEnumerateInstanceLayerProperties(&g_vulkanBackend.validationLayersCount, g_vulkanBackend.supportedValidationLayers);
    }

    for (uint32_t i = 0; i < g_vulkanBackend.validationLayersCount; ++i) {
        log_verbose(MSG_GFX, "Layer: %s - Desc: %s \n", g_vulkanBackend.supportedValidationLayers[i].layerName, g_vulkanBackend.supportedValidationLayers[i].description);
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
    instInfo.enabledExtensionCount = BEET_VK_INSTANCE_EXTENSION_COUNT;
    instInfo.ppEnabledExtensionNames = BEET_VK_INSTANCE_EXTENSIONS;
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

    VkPhysicalDevice *physicalDevices = (VkPhysicalDevice *) mem_zalloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(g_vulkanBackend.instance, &deviceCount, physicalDevices);

    // TODO:GFX: Add fallback support for `best` GPU based on intended workload, if no argument is provided we fallback to device [0]
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

    mem_free(physicalDevices);
}

void gfx_cleanup_surface() {
    // gfx_create_surface(...) exists in gfx_vulkan_surface.h / gfx_vulkan_surface_windows.cpp
    ASSERT_MSG(g_vulkanBackend.swapChain.surface != VK_NULL_HANDLE, "Err: VkSurface has already been destroyed");
    vkDestroySurfaceKHR(g_vulkanBackend.instance, g_vulkanBackend.swapChain.surface, nullptr);
    g_vulkanBackend.swapChain.surface = VK_NULL_HANDLE;
}

void gfx_cleanup_queues() {
    ASSERT_MSG(g_vulkanBackend.device != VK_NULL_HANDLE, "Err: device has already been destroyed");
    vkDestroyDevice(g_vulkanBackend.device, nullptr);
    g_vulkanBackend.device = VK_NULL_HANDLE;

    g_vulkanBackend.queue = VK_NULL_HANDLE;
//    g_vulkanBackend.queuePresent = nullptr;
//    g_vulkanBackend.queueTransfer = nullptr;
//    TODO: COMPUTE QUEUE
}

void gfx_create_queues() {
    uint32_t devicePropertyCount = 0;
    vkEnumerateDeviceExtensionProperties(g_vulkanBackend.physicalDevice, nullptr, &devicePropertyCount, nullptr);
    VkExtensionProperties *selectedPhysicalDeviceExtensions = (VkExtensionProperties *) mem_zalloc(sizeof(VkExtensionProperties) * devicePropertyCount);

    if (devicePropertyCount > 0) {
        vkEnumerateDeviceExtensionProperties(g_vulkanBackend.physicalDevice, nullptr, &devicePropertyCount, selectedPhysicalDeviceExtensions);
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanBackend.physicalDevice, &queueFamilyCount, nullptr);
    ASSERT(queueFamilyCount != 0);

    VkQueueFamilyProperties *queueFamilies = (VkQueueFamilyProperties *) mem_zalloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanBackend.physicalDevice, &queueFamilyCount, queueFamilies);

    struct QueueInfo {
        uint32_t targetFlags = {UINT32_MAX};
        uint32_t queueIndex = {UINT32_MAX};
        uint32_t currentFlags = {UINT32_MAX};
        bool supportsPresent = {false};
    };

    QueueInfo graphicsQueueInfo;
    graphicsQueueInfo.targetFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

    QueueInfo transferQueueInfo;
    transferQueueInfo.targetFlags = VK_QUEUE_TRANSFER_BIT;

    QueueInfo presentQueueInfo;
    presentQueueInfo.targetFlags = 0;

    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {
        VkBool32 supportsPresent = 0;
        VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(
                g_vulkanBackend.physicalDevice,
                queueFamilyIndex,
                g_vulkanBackend.swapChain.surface,
                &supportsPresent
        );

        const uint32_t currentQueueFlags = queueFamilies[queueFamilyIndex].queueFlags;

        if ((presentResult >= 0) && (supportsPresent == VK_TRUE)) {
            if (count_set_bits(currentQueueFlags) < count_set_bits(presentQueueInfo.currentFlags)
                || currentQueueFlags != graphicsQueueInfo.queueIndex) {
                presentQueueInfo.currentFlags = currentQueueFlags;
                presentQueueInfo.queueIndex = queueFamilyIndex;
                presentQueueInfo.supportsPresent = supportsPresent;
            }
        }

        if (currentQueueFlags & graphicsQueueInfo.targetFlags) {
            if (count_set_bits(currentQueueFlags) < count_set_bits(graphicsQueueInfo.currentFlags)) {
                graphicsQueueInfo.currentFlags = currentQueueFlags;
                graphicsQueueInfo.queueIndex = queueFamilyIndex;
                graphicsQueueInfo.supportsPresent = supportsPresent;
                continue;
            }
        }

        if (currentQueueFlags & transferQueueInfo.targetFlags) {
            if (count_set_bits(currentQueueFlags) < count_set_bits(transferQueueInfo.currentFlags)) {
                transferQueueInfo.currentFlags = currentQueueFlags;
                transferQueueInfo.queueIndex = queueFamilyIndex;
                transferQueueInfo.supportsPresent = supportsPresent;
                continue;
            }
        }
    }

    log_verbose(MSG_GFX, "graphics queue index: %u \n", graphicsQueueInfo.queueIndex);
//    log_verbose(MSG_GFX, "present queue index:  %u \n", presentQueueInfo.queueIndex);
//    log_verbose(MSG_GFX, "transfer queue index: %u \n", transferQueueInfo.queueIndex);
//    TODO: COMPUTE QUEUE

    ASSERT_MSG(graphicsQueueInfo.queueIndex != UINT32_MAX, "Err: did not find graphics queue");
//    ASSERT_MSG(presentQueueInfo.queueIndex != UINT32_MAX, "Err: did not find present queue");
//    ASSERT_MSG(transferQueueInfo.queueIndex != UINT32_MAX, "Err: did not find transfer queue");
//    TODO: COMPUTE QUEUE

    const float queuePriority = 1.0f;
    const uint32_t maxSupportedQueueCount = 3;
    uint32_t currentQueueCount = 0;

    VkDeviceQueueCreateInfo queueCreateInfo[maxSupportedQueueCount] = {};
    if (presentQueueInfo.queueIndex != graphicsQueueInfo.queueIndex) {
        queueCreateInfo[currentQueueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[currentQueueCount].queueFamilyIndex = presentQueueInfo.queueIndex;
        queueCreateInfo[currentQueueCount].queueCount = 1;
        queueCreateInfo[currentQueueCount].pQueuePriorities = &queuePriority;
        ++currentQueueCount;
    }

    queueCreateInfo[currentQueueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[currentQueueCount].queueFamilyIndex = graphicsQueueInfo.queueIndex;
    queueCreateInfo[currentQueueCount].queueCount = 1;
    queueCreateInfo[currentQueueCount].pQueuePriorities = &queuePriority;
    ++currentQueueCount;

    queueCreateInfo[currentQueueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[currentQueueCount].queueFamilyIndex = transferQueueInfo.queueIndex;
    queueCreateInfo[currentQueueCount].queueCount = 1;
    queueCreateInfo[currentQueueCount].pQueuePriorities = &queuePriority;
    ++currentQueueCount;

    uint32_t deviceExtensionCount = 0;
    const char *enabledDeviceExtensions[BEET_VK_MAX_DEVICE_EXTENSION_COUNT] = {};
    for (int32_t i = 0; i < BEET_VK_REQUIRED_DEVICE_EXTENSION_COUNT; ++i) {
        enabledDeviceExtensions[i] = BEET_VK_REQUIRED_DEVICE_EXTENSIONS[i];
        deviceExtensionCount++;
    }

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeaturesKHR{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = nullptr,
            .dynamicRendering = VK_TRUE,
    };
    void *pNextRoot = &dynamicRenderingFeaturesKHR;

    const VkPhysicalDeviceFeatures2 deviceFeatures2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = pNextRoot,
            .features = {
                    .samplerAnisotropy = VK_TRUE,
            },
    };

#if BEET_VK_COMPILE_VERSION_1_3
    const uint32_t runtimeVulkanVersion = g_vulkanBackend.deviceProperties.apiVersion;
    if (runtimeVulkanVersion >= BEET_VK_API_VERSION_1_3) {
        enabledDeviceExtensions[deviceExtensionCount] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
        deviceExtensionCount++;
    }
#endif

    const VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &deviceFeatures2,
            .queueCreateInfoCount = currentQueueCount,
            .pQueueCreateInfos = queueCreateInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = deviceExtensionCount,
            .ppEnabledExtensionNames = enabledDeviceExtensions,
    };

    vkCreateDevice(g_vulkanBackend.physicalDevice, &deviceCreateInfo, nullptr, &g_vulkanBackend.device);
    ASSERT_MSG(g_vulkanBackend.device, "Err: failed to create vkDevice");

    vkGetDeviceQueue(g_vulkanBackend.device, graphicsQueueInfo.queueIndex, 0, &g_vulkanBackend.queue);
//    vkGetDeviceQueue(g_vulkanBackend.device, presentQueueInfo.queueIndex, 0, &g_vulkanBackend.queuePresent);
//    vkGetDeviceQueue(g_vulkanBackend.device, transferQueueInfo.queueIndex, 0, &g_vulkanBackend.queueTransfer);
//    TODO: COMPUTE QUEUE

    g_vulkanBackend.queueFamilyIndices.graphics = graphicsQueueInfo.queueIndex;
//    g_gfxDevice->presentQueueIndex = presentQueueInfo.queueIndex;
//    g_gfxDevice->transferQueueIndex = transferQueueInfo.queueIndex;
//    TODO: COMPUTE QUEUE

    ASSERT_MSG(g_vulkanBackend.queue, "Err: failed to create graphics queue");
//    ASSERT_MSG(g_gfxDevice->vkPresentQueue, "Err: failed to create present queue");
//    ASSERT_MSG(g_gfxDevice->vkTransferQueue, "Err: failed to create transfer queue");
//    TODO: COMPUTE QUEUE

    mem_free(queueFamilies);
    mem_free(selectedPhysicalDeviceExtensions);
}

void gfx_cleanup_command_pool() {
    vkDestroyCommandPool(g_vulkanBackend.device, g_vulkanBackend.graphicsCommandPool, nullptr);
    g_vulkanBackend.graphicsCommandPool = VK_NULL_HANDLE;
}

void gfx_create_command_pool() {
    VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolInfo.queueFamilyIndex = g_vulkanBackend.queueFamilyIndices.graphics;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult cmdPoolResult = vkCreateCommandPool(g_vulkanBackend.device, &commandPoolInfo, nullptr, &g_vulkanBackend.graphicsCommandPool);
    ASSERT_MSG(cmdPoolResult == VK_SUCCESS, "Err: failed to create graphics command pool");
}

VkPresentModeKHR select_present_mode() {
    uint32_t presentModeCount = {0};
    const VkResult presentRes = vkGetPhysicalDeviceSurfacePresentModesKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &presentModeCount, nullptr);
    ASSERT_MSG(presentRes == VK_SUCCESS, "Err: failed to get physical device surface present modes");
    ASSERT(presentModeCount > 0);

    VkPresentModeKHR *presentModes = (VkPresentModeKHR *) mem_zalloc(sizeof(VkPresentModeKHR) * presentModeCount);
    const VkResult populateRes = vkGetPhysicalDeviceSurfacePresentModesKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &presentModeCount, presentModes);
    ASSERT_MSG(populateRes == VK_SUCCESS, "Err: failed to populate present modes array");

    VkPresentModeKHR selectedPresentMode = {VK_PRESENT_MODE_FIFO_KHR};
    VkPresentModeKHR preferredMode = g_userArguments.vsync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        if (presentModes[i] == preferredMode) {
            selectedPresentMode = presentModes[i];
            break;
        }
    }

    mem_free(presentModes);
    return selectedPresentMode;
}


VkCompositeAlphaFlagBitsKHR select_composite_alpha_format(const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    constexpr uint32_t compositeAlphaFlagsSize = 4;
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[compositeAlphaFlagsSize] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto &compositeAlphaFlag: compositeAlphaFlags) {
        if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
            compositeAlpha = compositeAlphaFlag;
            break;
        };
    }
    return compositeAlpha;
}

void gfx_create_swap_chain() {
    const VkSwapchainKHR oldSwapChain = g_vulkanBackend.swapChain.swapChain;

    VkSurfaceCapabilitiesKHR surfaceCapabilities = {0};
    const VkResult surfRes = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &surfaceCapabilities);
    ASSERT_MSG(surfRes == VK_SUCCESS, "Err: failed to get physical device surface capabilities");

    VkExtent2D swapChainExtent = {};
    if (surfaceCapabilities.currentExtent.width == UINT32_MAX) {
        swapChainExtent.width = g_vulkanBackend.swapChain.width;
        swapChainExtent.height = g_vulkanBackend.swapChain.height;
    } else {
        swapChainExtent = surfaceCapabilities.currentExtent;
        g_vulkanBackend.swapChain.width = surfaceCapabilities.currentExtent.width;
        g_vulkanBackend.swapChain.height = surfaceCapabilities.currentExtent.height;
    }

    const VkPresentModeKHR swapChainPresentMode = select_present_mode();
    g_vulkanTargetFormats.surfaceFormat = gfx_utils_select_surface_format();
    const VkCompositeAlphaFlagBitsKHR compositeAlphaFormat = select_composite_alpha_format(surfaceCapabilities);

    uint32_t targetSwapChainImageCount = surfaceCapabilities.minImageCount + 1;
    if ((surfaceCapabilities.maxImageCount > 0) && (targetSwapChainImageCount > surfaceCapabilities.maxImageCount)) {
        targetSwapChainImageCount = surfaceCapabilities.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR surfaceTransform;
    if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        surfaceTransform = surfaceCapabilities.currentTransform;
    }

    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    // swapChainInfo.pNext
    // swapChainInfo.flags
    swapChainInfo.surface = g_vulkanBackend.swapChain.surface;
    swapChainInfo.minImageCount = targetSwapChainImageCount;
    swapChainInfo.imageFormat = g_vulkanTargetFormats.surfaceFormat.format;
    swapChainInfo.imageColorSpace = g_vulkanTargetFormats.surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = swapChainExtent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // imageSharingMode
    swapChainInfo.queueFamilyIndexCount = 0;
    // pQueueFamilyIndices
    swapChainInfo.preTransform = (VkSurfaceTransformFlagBitsKHR) surfaceTransform;
    swapChainInfo.compositeAlpha = compositeAlphaFormat;
    swapChainInfo.presentMode = swapChainPresentMode;
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = oldSwapChain;

    if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        swapChainInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapChainInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    const VkResult swapChainRes = vkCreateSwapchainKHR(g_vulkanBackend.device, &swapChainInfo, nullptr, &g_vulkanBackend.swapChain.swapChain);
    ASSERT_MSG(swapChainRes == VK_SUCCESS, "Err: failed to create swap chain");

    //cleanup vulkan resources
    if (oldSwapChain != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; i++) {
            vkDestroyImageView(g_vulkanBackend.device, g_vulkanBackend.swapChain.buffers[i].view, nullptr);
        }
        vkDestroySwapchainKHR(g_vulkanBackend.device, oldSwapChain, nullptr);
    }
    vkGetSwapchainImagesKHR(g_vulkanBackend.device, g_vulkanBackend.swapChain.swapChain, &g_vulkanBackend.swapChain.imageCount, nullptr);

    ASSERT(g_vulkanBackend.swapChain.images == nullptr);
    g_vulkanBackend.swapChain.images = (VkImage *) mem_zalloc(sizeof(VkImage) * g_vulkanBackend.swapChain.imageCount);
    VkResult swapChainImageResult = vkGetSwapchainImagesKHR(g_vulkanBackend.device, g_vulkanBackend.swapChain.swapChain,
                                                            &g_vulkanBackend.swapChain.imageCount,
                                                            &g_vulkanBackend.swapChain.images[0]);
    ASSERT_MSG(swapChainImageResult == VK_SUCCESS, "Err: failed to re-create swap chain images")

    ASSERT(g_vulkanBackend.swapChain.buffers == nullptr);
    g_vulkanBackend.swapChain.buffers = (SwapChainBuffers *) mem_zalloc(sizeof(SwapChainBuffers) * g_vulkanBackend.swapChain.imageCount);
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; i++) {
        VkImageViewCreateInfo swapChainImageViewInfo = {};
        swapChainImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        swapChainImageViewInfo.pNext = nullptr;
        swapChainImageViewInfo.format = g_vulkanTargetFormats.surfaceFormat.format;
        swapChainImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        swapChainImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        swapChainImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        swapChainImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        swapChainImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        swapChainImageViewInfo.subresourceRange.baseMipLevel = 0;
        swapChainImageViewInfo.subresourceRange.levelCount = 1;
        swapChainImageViewInfo.subresourceRange.baseArrayLayer = 0;
        swapChainImageViewInfo.subresourceRange.layerCount = 1;
        swapChainImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        swapChainImageViewInfo.flags = 0;

        g_vulkanBackend.swapChain.buffers[i].image = g_vulkanBackend.swapChain.images[i];
        swapChainImageViewInfo.image = g_vulkanBackend.swapChain.buffers[i].image;

        const VkResult imageViewResult = vkCreateImageView(g_vulkanBackend.device, &swapChainImageViewInfo, nullptr, &g_vulkanBackend.swapChain.buffers[i].view);
        ASSERT_MSG(imageViewResult == VK_SUCCESS, "Failed to create image view %u", i);
    }
}

void gfx_cleanup_swap_chain() {
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; i++) {
        vkDestroyImageView(g_vulkanBackend.device, g_vulkanBackend.swapChain.buffers[i].view, nullptr);
    }

    ASSERT_MSG(g_vulkanBackend.swapChain.surface != VK_NULL_HANDLE, "Err: swapchain surface has already been destroyed");
    vkDestroySwapchainKHR(g_vulkanBackend.device, g_vulkanBackend.swapChain.swapChain, nullptr);
    g_vulkanBackend.swapChain.swapChain = VK_NULL_HANDLE;

    mem_free(g_vulkanBackend.swapChain.images);
    g_vulkanBackend.swapChain.images = nullptr;

    mem_free(g_vulkanBackend.swapChain.buffers);
    g_vulkanBackend.swapChain.buffers = nullptr;
}

void gfx_create_command_buffers() {
    // === GRAPHICS ===
    ASSERT(g_vulkanBackend.graphicsCommandBuffers == nullptr);
    g_vulkanBackend.graphicsCommandBuffers = (VkCommandBuffer *) mem_zalloc(sizeof(VkCommandBuffer) * g_vulkanBackend.swapChain.imageCount);

    VkCommandBufferAllocateInfo commandBufferInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferInfo.commandPool = g_vulkanBackend.graphicsCommandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = g_vulkanBackend.swapChain.imageCount;

    VkResult cmdBuffResult = vkAllocateCommandBuffers(g_vulkanBackend.device, &commandBufferInfo, g_vulkanBackend.graphicsCommandBuffers);
    ASSERT_MSG(cmdBuffResult == VK_SUCCESS, "Err: failed to create graphics command buffer");

    // === IMMEDIATE ===
    ASSERT(g_vulkanBackend.immediateCommandBuffer == VK_NULL_HANDLE);
    commandBufferInfo.commandBufferCount = 1;
    VkResult cmdImmediateBufferResult = vkAllocateCommandBuffers(g_vulkanBackend.device, &commandBufferInfo, &g_vulkanBackend.immediateCommandBuffer);
    ASSERT_MSG(cmdImmediateBufferResult == VK_SUCCESS, "Err: failed to create immediate command buffer");
}

void gfx_cleanup_command_buffers() {
    // === GRAPHICS ===
    vkFreeCommandBuffers(g_vulkanBackend.device, g_vulkanBackend.graphicsCommandPool, g_vulkanBackend.swapChain.imageCount, g_vulkanBackend.graphicsCommandBuffers);
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; ++i) {
        g_vulkanBackend.graphicsCommandBuffers[i] = VK_NULL_HANDLE;
    }

    mem_free(g_vulkanBackend.graphicsCommandBuffers);
    g_vulkanBackend.graphicsCommandBuffers = nullptr;

    // === IMMEDIATE ===
    vkFreeCommandBuffers(g_vulkanBackend.device, g_vulkanBackend.graphicsCommandPool, 1, &g_vulkanBackend.immediateCommandBuffer);
    g_vulkanBackend.immediateCommandBuffer = VK_NULL_HANDLE;
}

void gfx_create_fences() {
    const VkFenceCreateInfo fenceCreateInfo = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            nullptr,
            VK_FENCE_CREATE_SIGNALED_BIT
    };

    ASSERT(g_vulkanBackend.graphicsFenceWait == nullptr);
    g_vulkanBackend.graphicsFenceWait = (VkFence *) mem_zalloc(sizeof(VkFence) * g_vulkanBackend.swapChain.imageCount);
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; ++i) {
        const VkResult fenceRes = vkCreateFence(g_vulkanBackend.device, &fenceCreateInfo, nullptr, &g_vulkanBackend.graphicsFenceWait[i]);
        ASSERT_MSG(fenceRes == VK_SUCCESS, "Err: failed to create graphics fence [%u]", i);
    }
}

void gfx_cleanup_fences() {
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; ++i) {
        vkDestroyFence(g_vulkanBackend.device, g_vulkanBackend.graphicsFenceWait[i], nullptr);
        g_vulkanBackend.graphicsFenceWait[i] = VK_NULL_HANDLE;
    }
    mem_free(g_vulkanBackend.graphicsFenceWait);
    g_vulkanBackend.graphicsFenceWait = nullptr;
}

void gfx_create_depth_stencil_buffer() {
    g_vulkanTargetFormats.depthFormat = gfx_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);

    VkImageCreateInfo depthImageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.format = g_vulkanTargetFormats.depthFormat;
    depthImageInfo.extent = {g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height, 1};
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    const VkResult imgRes = vkCreateImage(g_vulkanBackend.device, &depthImageInfo, nullptr, &g_vulkanBackend.depthStencil.image);
    ASSERT_MSG(imgRes == VK_SUCCESS, "Err: failed to create depth stencil image");

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(g_vulkanBackend.device, g_vulkanBackend.depthStencil.image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = gfx_utils_get_memory_type(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    const VkResult allocRes = vkAllocateMemory(g_vulkanBackend.device, &allocInfo, nullptr, &g_vulkanBackend.depthStencil.deviceMemory);
    ASSERT_MSG(allocRes == VK_SUCCESS, "Err: failed to allocate memory for depth stencil")

    const VkResult bindRes = vkBindImageMemory(g_vulkanBackend.device, g_vulkanBackend.depthStencil.image, g_vulkanBackend.depthStencil.deviceMemory, 0);
    ASSERT_MSG(bindRes == VK_SUCCESS, "Err: failed to bind depth stencil");

    VkImageViewCreateInfo depthImageViewInfo{};
    depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthImageViewInfo.image = g_vulkanBackend.depthStencil.image;
    depthImageViewInfo.format = g_vulkanTargetFormats.depthFormat;
    depthImageViewInfo.subresourceRange.baseMipLevel = 0;
    depthImageViewInfo.subresourceRange.levelCount = 1;
    depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
    depthImageViewInfo.subresourceRange.layerCount = 1;
    depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (g_vulkanTargetFormats.depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        depthImageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    const VkResult createRes = vkCreateImageView(g_vulkanBackend.device, &depthImageViewInfo, nullptr, &g_vulkanBackend.depthStencil.view);
    ASSERT_MSG(createRes == VK_SUCCESS, "Err: failed to create depth image view");
}

void gfx_cleanup_depth_stencil_buffer() {
    vkDestroyImageView(g_vulkanBackend.device, g_vulkanBackend.depthStencil.view, nullptr);
    vkDestroyImage(g_vulkanBackend.device, g_vulkanBackend.depthStencil.image, nullptr);
    vkFreeMemory(g_vulkanBackend.device, g_vulkanBackend.depthStencil.deviceMemory, nullptr);
}


void gfx_create_pipeline_cache() {
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    const VkResult cacheRes = vkCreatePipelineCache(g_vulkanBackend.device, &pipelineCacheCreateInfo, nullptr, &g_vulkanBackend.pipelineCache);
    ASSERT_MSG(cacheRes == VK_SUCCESS, "Err: failed to create pipeline cache")
}

void gfx_cleanup_pipeline_cache() {
    ASSERT_MSG(g_vulkanBackend.pipelineCache != VK_NULL_HANDLE, "Err: pipeline cache has already been destroyed");
    vkDestroyPipelineCache(g_vulkanBackend.device, g_vulkanBackend.pipelineCache, nullptr);
    g_vulkanBackend.pipelineCache = VK_NULL_HANDLE;
}

void gfx_flush() {
    if (g_vulkanBackend.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(g_vulkanBackend.device);
    }
}

void gfx_render_frame() {
    g_vulkanBackend.submitInfo.commandBufferCount = 1;
    g_vulkanBackend.submitInfo.pCommandBuffers = &g_vulkanBackend.graphicsCommandBuffers[g_vulkanBackend.currentBufferIndex];
    const VkResult submitRes = vkQueueSubmit(g_vulkanBackend.queue, 1, &g_vulkanBackend.submitInfo, VK_NULL_HANDLE);
    ASSERT(submitRes == VK_SUCCESS);
}

VkResult gfx_acquire_next_swap_chain_image() {
    return vkAcquireNextImageKHR(
            g_vulkanBackend.device,
            g_vulkanBackend.swapChain.swapChain,
            UINT64_MAX,
            g_vulkanBackend.semaphores.presentDone,
            VK_NULL_HANDLE,
            &g_vulkanBackend.currentBufferIndex
    );
}

VkResult gfx_present() {
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &g_vulkanBackend.swapChain.swapChain;
    presentInfo.pImageIndices = &g_vulkanBackend.currentBufferIndex;

    if (g_vulkanBackend.semaphores.renderDone != VK_NULL_HANDLE) {
        presentInfo.pWaitSemaphores = &g_vulkanBackend.semaphores.renderDone;
        presentInfo.waitSemaphoreCount = 1;
    }

    return vkQueuePresentKHR(g_vulkanBackend.queue, &presentInfo);
}

void begin_command_recording(const VkCommandBuffer &cmdBuffer) {
    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    auto result = vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);
    ASSERT_MSG(result == VK_SUCCESS, "Err: Vulkan failed to begin command buffer recording");
}

void end_command_recording(const VkCommandBuffer &cmdBuffer) {
    vkEndCommandBuffer(cmdBuffer);
}

bool query_has_valid_extent_size() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &surfaceCapabilities);
    return surfaceCapabilities.currentExtent.width != 0 || surfaceCapabilities.currentExtent.height != 0;
}

void gfx_window_resize() {
    if (!query_has_valid_extent_size()) {
        return;
    }
    gfx_flush();

    gfx_cleanup_depth_stencil_buffer();
    gfx_cleanup_swap_chain();

    gfx_create_swap_chain();
    gfx_create_depth_stencil_buffer();
}

void gfx_update_uniform_buffers() {
    struct UniformBufferLayout {
        glm::mat4 projection;
        glm::mat4 view;
    };

    //TODO: UPDATE UBO with camera info i.e. view & proj.
    const Camera &camera = g_dbEntities.camObj.cam;
    const Transform &camTransform = g_dbEntities.camObj.transform;

    const vec3f camForward = glm::quat(camTransform.rotation) * WORLD_FORWARD;
    const vec3f lookTarget = camTransform.position + camForward;

    const mat4 view = lookAt(camTransform.position, lookTarget, WORLD_UP);
    const vec2f screen = {g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height};
    mat4 proj = perspective(as_radians(camera.fov), (float) screen.x / (float) screen.y, camera.zNear, camera.zFar);
    proj[1][1] *= -1; // flip view proj, need to switch to the vulkan glm define to fix this.
    mat4 viewProj = proj * view;

    const UniformBufferLayout uniformBuffData{
            .projection = proj,
            .view = view,
    };
    memcpy(g_vulkanBackend.uniformBuffer.mappedData, &uniformBuffData, sizeof(uniformBuffData));
}


void gfx_barrier_insert_memory_barrier(
        VkCommandBuffer &cmdBuffer,
        const VkImage &image,
        const VkAccessFlags srcAccessMask,
        const VkAccessFlags dstAccessMask,
        const VkImageLayout oldImageLayout,
        const VkImageLayout newImageLayout,
        const VkPipelineStageFlags srcStageMask,
        const VkPipelineStageFlags dstStageMask,
        const VkImageSubresourceRange subresourceRange) {

    VkImageMemoryBarrier imageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldImageLayout,
            .newLayout = newImageLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subresourceRange,
    };
    vkCmdPipelineBarrier(cmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void gfx_dynamic_render(VkCommandBuffer &cmdBuffer) {

    //TODO: replace cmdIndexed variables with references.
    const uint32_t cmdIndex = g_vulkanBackend.currentBufferIndex;
    gfx_barrier_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.swapChain.buffers[cmdIndex].image,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            }
    );
    gfx_barrier_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.depthStencil.image,
            0,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            }
    );

    const VkRenderingAttachmentInfoKHR colorAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = g_vulkanBackend.swapChain.buffers[cmdIndex].view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{
                    .color = {{0.5f, 0.092f, 0.167f, 1.0f}}
            },
    };

    const VkRenderingAttachmentInfoKHR depthStencilAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = g_vulkanBackend.depthStencil.view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {
                    .depthStencil = {1.0f, 0}
            },
    };

    const VkRenderingInfoKHR renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {0, 0, g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment,
            .pDepthAttachment = &depthStencilAttachment,
            .pStencilAttachment = &depthStencilAttachment,
    };

    {
        gfx_command_begin_rendering(cmdBuffer, renderingInfo);
        {
            const VkViewport viewport = {0, 0, float(g_vulkanBackend.swapChain.width), float(g_vulkanBackend.swapChain.height), 0.0f, 1.0f};
            vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

            const VkRect2D scissor = {0, 0, g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height};
            vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

            gfx_lit_draw(cmdBuffer);
#if BEET_GFX_IMGUI
            gfx_imgui_draw(cmdBuffer);
#endif // BEET_GFX_IMGUI
        }
        gfx_command_end_rendering(cmdBuffer);
    }

    gfx_barrier_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.swapChain.buffers[cmdIndex].image,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    );
}


void gfx_create_semaphores() {
    VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    const VkResult presentRes = vkCreateSemaphore(g_vulkanBackend.device, &semaphoreInfo, nullptr, &g_vulkanBackend.semaphores.presentDone);
    ASSERT_MSG(presentRes == VK_SUCCESS, "Err: failed to create present semaphore");

    const VkResult renderRes = vkCreateSemaphore(g_vulkanBackend.device, &semaphoreInfo, nullptr, &g_vulkanBackend.semaphores.renderDone);
    ASSERT_MSG(renderRes == VK_SUCCESS, "Err: failed to create render semaphore");

    g_vulkanBackend.submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    g_vulkanBackend.submitInfo.pWaitDstStageMask = &g_vulkanBackend.submitPipelineStages;
    g_vulkanBackend.submitInfo.waitSemaphoreCount = 1;
    g_vulkanBackend.submitInfo.pWaitSemaphores = &g_vulkanBackend.semaphores.presentDone;
    g_vulkanBackend.submitInfo.signalSemaphoreCount = 1;
    g_vulkanBackend.submitInfo.pSignalSemaphores = &g_vulkanBackend.semaphores.renderDone;
}

void gfx_cleanup_semaphores() {
    vkDestroySemaphore(g_vulkanBackend.device, g_vulkanBackend.semaphores.presentDone, nullptr);
    vkDestroySemaphore(g_vulkanBackend.device, g_vulkanBackend.semaphores.renderDone, nullptr);
    g_vulkanBackend.submitInfo = {};
}

VkFormat beet_image_format_to_vk(TextureFormat textureFormat) {
    switch (textureFormat) {
        case TextureFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA16:
            return VK_FORMAT_R16G16B16A16_UNORM;

        case TextureFormat::BC1RGBA:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case TextureFormat::BC2:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case TextureFormat::BC3:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case TextureFormat::BC4:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case TextureFormat::BC5:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case TextureFormat::BC6H:
            return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case TextureFormat::BC7:
            return VK_FORMAT_BC7_UNORM_BLOCK;
        default: SANITY_CHECK();
    };
    SANITY_CHECK();
    return VK_FORMAT_UNDEFINED;
}

void set_image_layout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask) {
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // Image layout is undefined (or does not matter)
            // Only valid as initial layout
            // No flags required, listed only for completeness
            imageMemoryBarrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            // Image is preinitialized
            // Only valid as initial layout for linear images, preserves memory contents
            // Make sure host writes have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image is a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image is a depth/stencil attachment
            // Make sure any writes to the depth/stencil buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image is a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image is a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image is read by a shader
            // Make sure any shader reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used as a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used as a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image layout will be used as a depth/stencil attachment
            // Make sure any writes to depth/stencil buffer have been finished
            imageMemoryBarrier.dstAccessMask =
                    imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read in a shader (sampler, input attachment)
            // Make sure any writes to the image have been finished
            if (imageMemoryBarrier.srcAccessMask == 0) {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
            cmdbuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
}


void gfx_texture_create_immediate(VkCommandBuffer &commandBuffer, const char *path, GfxTexture &outTexture) {
    outTexture.imageSamplerType = TextureSamplerType::Linear;

    RawImage myImage{};
    load_dds_image(path, &myImage);
    auto rawImageData = (unsigned char *) myImage.data;

    const uint32_t sizeX = myImage.width;
    const uint32_t sizeY = myImage.height;
    const uint32 mipMapCount = myImage.mipMapCount;
    const VkDeviceSize imageSize = myImage.dataSize;
    const VkFormat format = beet_image_format_to_vk(myImage.textureFormat);

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(g_vulkanBackend.physicalDevice, format, &formatProperties);

    VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    VkMemoryRequirements memoryRequirements;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkBufferCreateInfo stagingBufInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    stagingBufInfo.size = imageSize;
    stagingBufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    const VkResult createStageBuffRes = vkCreateBuffer(g_vulkanBackend.device, &stagingBufInfo, nullptr, &stagingBuffer);
    ASSERT(createStageBuffRes == VK_SUCCESS);

    vkGetBufferMemoryRequirements(g_vulkanBackend.device, stagingBuffer, &memoryRequirements);
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = gfx_utils_get_memory_type(memoryRequirements.memoryTypeBits,
                                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    const VkResult memAllocResult = vkAllocateMemory(g_vulkanBackend.device, &memoryAllocateInfo, nullptr, &stagingMemory);
    ASSERT(memAllocResult == VK_SUCCESS);

    const VkResult memBindResult = vkBindBufferMemory(g_vulkanBackend.device, stagingBuffer, stagingMemory, 0);
    ASSERT(memBindResult == VK_SUCCESS);

    // map raw image data to staging buffer
    uint8_t *data;
    const VkResult mapResult(vkMapMemory(g_vulkanBackend.device, stagingMemory, 0, memoryRequirements.size, 0, (void **) &data));
    ASSERT(mapResult == VK_SUCCESS)

    memcpy(data, rawImageData, imageSize);
    vkUnmapMemory(g_vulkanBackend.device, stagingMemory);

    VkBufferImageCopy *bufferCopyRegions = (VkBufferImageCopy *) mem_zalloc(mipMapCount * sizeof(VkBufferImageCopy));
    uint32_t offset = 0;
    for (uint32_t i = 0; i < mipMapCount; i++) {
        // set up a buffer image copy structure for the current mip level
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = i;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = myImage.width >> i;
        bufferCopyRegion.imageExtent.height = myImage.height >> i;
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.bufferOffset = offset;
        bufferCopyRegions[i] = bufferCopyRegion;
        offset += myImage.mipDataSizes[i];
    }

    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = sizeX;
    imageCreateInfo.extent.height = sizeY;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = mipMapCount;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.flags = 0;

    const VkResult createImageRes = vkCreateImage(g_vulkanBackend.device, &imageCreateInfo, nullptr, &outTexture.image);
    ASSERT(createImageRes == VK_SUCCESS);

    vkGetImageMemoryRequirements(g_vulkanBackend.device, outTexture.image, &memoryRequirements);

    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = gfx_utils_get_memory_type(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    const VkResult memAllocRes = vkAllocateMemory(g_vulkanBackend.device, &memoryAllocateInfo, nullptr, &outTexture.deviceMemory);
    ASSERT(memAllocRes == VK_SUCCESS);

    const VkResult bindRes = vkBindImageMemory(g_vulkanBackend.device, outTexture.image, outTexture.deviceMemory, 0);
    ASSERT(bindRes == VK_SUCCESS);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipMapCount;
    subresourceRange.layerCount = 1;

    gfx_command_begin_immediate_recording();

    set_image_layout(
            commandBuffer,
            g_textures.uvGrid.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            subresourceRange,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    );

    // Copy mips from staging buffer
    vkCmdCopyBufferToImage(
            commandBuffer,
            stagingBuffer,
            g_textures.uvGrid.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(mipMapCount),
            bufferCopyRegions
    );

    set_image_layout(
            commandBuffer,
            g_textures.uvGrid.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            subresourceRange,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    );

    outTexture.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    gfx_command_end_immediate_recording();

    vkDestroyBuffer(g_vulkanBackend.device, stagingBuffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, stagingMemory, nullptr);

    VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = beet_image_format_to_vk(myImage.textureFormat);;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    view.subresourceRange.levelCount = mipMapCount;
    view.image = outTexture.image;

    const VkResult imageViewRes = vkCreateImageView(g_vulkanBackend.device, &view, nullptr, &outTexture.view);
    ASSERT(imageViewRes == VK_SUCCESS);

    outTexture.descriptor.imageView = outTexture.view;
    outTexture.descriptor.sampler = gfx_samplers()->samplers[outTexture.imageSamplerType];
    outTexture.descriptor.imageLayout = outTexture.layout;
}

void gfx_mesh_cleanup(GfxMesh &mesh) {
    vkDestroyBuffer(g_vulkanBackend.device, mesh.vertBuffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, mesh.vertMemory, nullptr);
    vkDestroyBuffer(g_vulkanBackend.device, mesh.indexBuffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, mesh.indexMemory, nullptr);
}

void gfx_texture_cleanup(GfxTexture &texture) {
    vkDestroyImageView(g_vulkanBackend.device, texture.view, nullptr);
    vkDestroyImage(g_vulkanBackend.device, texture.image, nullptr);
    vkFreeMemory(g_vulkanBackend.device, texture.deviceMemory, nullptr);
}

void gfx_load_packages() {
    const char *pathUVGrid = "../assets/textures/UV_Grid/UV_Grid_test.dds";
    gfx_texture_create_immediate(g_vulkanBackend.immediateCommandBuffer, pathUVGrid, g_textures.uvGrid);
    gfx_cube_create_immediate(g_meshes.cube);
};

void gfx_unload_packages() {
    gfx_texture_cleanup(g_textures.uvGrid);
    gfx_mesh_cleanup(g_meshes.cube);
}

void gfx_create_uniform_buffers() {
    const VkResult uniformResult = gfx_buffer_create(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            g_vulkanBackend.uniformBuffer,
            sizeof(UniformData),
            nullptr
    );
    ASSERT(uniformResult == VK_SUCCESS);

    const VkResult mapResult = vkMapMemory(g_vulkanBackend.device, g_vulkanBackend.uniformBuffer.memory, 0, VK_WHOLE_SIZE, 0, &g_vulkanBackend.uniformBuffer.mappedData);
    ASSERT(mapResult == VK_SUCCESS);
}

void gfx_cleanup_uniform_buffers() {
    vkDestroyBuffer(g_vulkanBackend.device, g_vulkanBackend.uniformBuffer.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, g_vulkanBackend.uniformBuffer.memory, nullptr);
}

void gfx_create(void *windowHandle) {
    gfx_create_instance();
    gfx_create_debug_callbacks();
    gfx_create_physical_device();
    gfx_create_surface(windowHandle, &g_vulkanBackend.instance, &g_vulkanBackend.swapChain.surface);
    gfx_create_queues();
    gfx_create_command_pool();
    gfx_create_semaphores();
    gfx_create_swap_chain();
    gfx_create_command_buffers();
    gfx_create_fences();
    gfx_create_depth_stencil_buffer();
    gfx_create_pipeline_cache();
    gfx_create_samplers();
    gfx_create_function_pointers();

    gfx_load_packages();
    gfx_create_uniform_buffers();

#if BEET_GFX_IMGUI
    gfx_create_imgui(windowHandle);
#endif //BEET_GFX_IMGUI
    gfx_create_lit();
}

void gfx_update(const double &deltaTime) {
    gfx_update_uniform_buffers();
    const VkResult nextRes = gfx_acquire_next_swap_chain_image();

    if (nextRes == VK_ERROR_OUT_OF_DATE_KHR) {
        gfx_window_resize();
        return;
    } else if (nextRes < 0) {
        ASSERT(nextRes == VK_SUCCESS)
    }
    VkCommandBuffer cmdBuffer = g_vulkanBackend.graphicsCommandBuffers[g_vulkanBackend.currentBufferIndex];
    vkResetCommandBuffer(cmdBuffer, 0);
    begin_command_recording(cmdBuffer);
    {
        gfx_dynamic_render(cmdBuffer);
    }
    end_command_recording(cmdBuffer);
    //TODO: build draw commands
    gfx_render_frame();
    gfx_present();
    gfx_flush();
}

void gfx_cleanup() {
    gfx_cleanup_uniform_buffers();
    gfx_unload_packages();

    gfx_cleanup_lit();
#if BEET_GFX_IMGUI
    gfx_cleanup_imgui();
#endif //BEET_GFX_IMGUI
    gfx_cleanup_samplers();
    gfx_cleanup_pipeline_cache();
    gfx_cleanup_depth_stencil_buffer();
    gfx_cleanup_fences();
    gfx_cleanup_command_buffers();
    gfx_cleanup_swap_chain();
    gfx_cleanup_semaphores();
    gfx_cleanup_command_pool();
    gfx_cleanup_queues();
    gfx_cleanup_surface();
    gfx_cleanup_physical_device();
    gfx_cleanup_debug_callbacks();
    gfx_cleanup_instance();
    gfx_cleanup_function_pointers();
}