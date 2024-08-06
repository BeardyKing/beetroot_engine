#include <vulkan/vulkan.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>
#include <beet_shared/shared_utils.h>
#include <beet_shared/memory.h>
#include <beet_shared/c_string.h>
#include <beet_shared/beet_types.h>

#include <beet_gfx/gfx_vulkan_platform_defines.h>
#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/gfx_vulkan_surface.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_samplers.h>
#include <beet_gfx/gfx_buffer.h>
#include <beet_gfx/gfx_command.h>
#include <beet_gfx/gfx_utils.h>
#include <beet_gfx/gfx_function_pointers.h>
#include <beet_gfx/gfx_debug.h>
#include <beet_gfx/gfx_imgui.h>
#include <beet_gfx/gfx_lit.h>
#include <beet_gfx/gfx_sky.h>
#include <beet_gfx/db_asset.h>
#include <beet_gfx/gfx_converter.h>
#include <beet_gfx/gfx_line.h>
#include <beet_gfx/gfx_triangle_strip.h>

#include <beet_math/quat.h>
#include <beet_math/utilities.h>

#include <fstream>
#include <cstring>

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
    VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_8_BIT;
} g_userArguments = {};

VulkanBackend g_vulkanBackend = {};
TargetVulkanFormats g_vulkanTargetFormats = {};

static struct {
    uint32_t currentFrame = {0};
} s_vulkanBackendInternal;

//===INTERNAL_FUNCTIONS=================================================================================================
static bool gfx_find_supported_extension(const char *extensionName) {
    for (uint32_t i = 0; i < g_vulkanBackend.extensionsCount; ++i) {
        if (c_str_equal(g_vulkanBackend.supportedExtensions[i].extensionName, extensionName)) {
            return true;
        }
    }
    return false;
}

static bool gfx_find_supported_validation(const char *layerName) {
    for (uint32_t i = 0; i < g_vulkanBackend.validationLayersCount; ++i) {
        if (c_str_equal(g_vulkanBackend.supportedValidationLayers[i].layerName, layerName)) {
            return true;
        }
    }
    return false;
}

static void gfx_cleanup_instance() {
    ASSERT_MSG(g_vulkanBackend.instance != VK_NULL_HANDLE, "Err: VkInstance has already been destroyed");
    vkDestroyInstance(g_vulkanBackend.instance, nullptr);
    g_vulkanBackend.instance = VK_NULL_HANDLE;

    mem_free(g_vulkanBackend.supportedExtensions);
    mem_free(g_vulkanBackend.supportedValidationLayers);
}

static void gfx_create_instance() {
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

    VkApplicationInfo applicationInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "VK_BEETROOT_ENGINE",
            .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
            .pEngineName = "VK_BEETROOT_ENGINE",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0),
            .apiVersion = BEET_MAX_VK_API_VERSION,
    };

    VkInstanceCreateInfo instanceInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = BEET_VK_VALIDATION_COUNT,
            .ppEnabledLayerNames = beetVulkanValidations,
            .enabledExtensionCount = BEET_VK_INSTANCE_EXTENSION_COUNT,
            .ppEnabledExtensionNames = BEET_VK_INSTANCE_EXTENSIONS,
    };

    const auto result = vkCreateInstance(&instanceInfo, nullptr, &g_vulkanBackend.instance);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create vulkan instance");
}

static void gfx_cleanup_physical_device() {
    ASSERT_MSG(g_vulkanBackend.physicalDevice != VK_NULL_HANDLE, "Err: VkPhysicalDevice has already been invalidated");
    g_vulkanBackend.physicalDevice = VK_NULL_HANDLE;
}

static VkSampleCountFlagBits set_target_sample_count(const VkPhysicalDeviceProperties deviceProperties, VkSampleCountFlagBits target) {
    VkSampleCountFlags supportedSampleCount = min(deviceProperties.limits.framebufferColorSampleCounts, deviceProperties.limits.framebufferDepthSampleCounts);
    return (supportedSampleCount & target) ? target : VK_SAMPLE_COUNT_1_BIT;
}

static void gfx_create_physical_device() {
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
    g_vulkanBackend.sampleCount = set_target_sample_count(g_vulkanBackend.deviceProperties, g_userArguments.msaa);

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

static void gfx_cleanup_surface() {
    // gfx_create_surface(...) exists in gfx_vulkan_surface.h / gfx_vulkan_surface_windows.cpp
    ASSERT_MSG(g_vulkanBackend.swapChain.surface != VK_NULL_HANDLE, "Err: VkSurface has already been destroyed");
    vkDestroySurfaceKHR(g_vulkanBackend.instance, g_vulkanBackend.swapChain.surface, nullptr);
    g_vulkanBackend.swapChain.surface = VK_NULL_HANDLE;
}

static void gfx_cleanup_queues() {
    ASSERT_MSG(g_vulkanBackend.device != VK_NULL_HANDLE, "Err: device has already been destroyed");
    vkDestroyDevice(g_vulkanBackend.device, nullptr);
    g_vulkanBackend.device = VK_NULL_HANDLE;

    g_vulkanBackend.queue = VK_NULL_HANDLE;
//    g_vulkanBackend.queuePresent = nullptr;
//    g_vulkanBackend.queueTransfer = nullptr;
//    TODO: COMPUTE QUEUE
}

static void gfx_create_queues() {
    uint32_t devicePropertyCount = 0;
    vkEnumerateDeviceExtensionProperties(g_vulkanBackend.physicalDevice, nullptr, &devicePropertyCount, nullptr);
    VkExtensionProperties *selectedPhysicalDeviceExtensions = (VkExtensionProperties *) mem_zalloc(sizeof(VkExtensionProperties) * devicePropertyCount);

    if (devicePropertyCount > 0) {
        vkEnumerateDeviceExtensionProperties(g_vulkanBackend.physicalDevice, nullptr, &devicePropertyCount, selectedPhysicalDeviceExtensions);
    }

    for (uint32_t i = 0; i < devicePropertyCount; ++i) {
        log_verbose(MSG_GFX, "Device extensions: %s \n", selectedPhysicalDeviceExtensions[i].extensionName);
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
    void *pNextRoot0 = &dynamicRenderingFeaturesKHR;

    const VkPhysicalDeviceFeatures2 deviceFeatures2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = pNextRoot0,
            .features = {
                    .wideLines = VK_TRUE,
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

static void gfx_cleanup_command_pool() {
    vkDestroyCommandPool(g_vulkanBackend.device, g_vulkanBackend.graphicsCommandPool, nullptr);
    g_vulkanBackend.graphicsCommandPool = VK_NULL_HANDLE;
}

static void gfx_create_command_pool() {
    VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolInfo.queueFamilyIndex = g_vulkanBackend.queueFamilyIndices.graphics;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult cmdPoolResult = vkCreateCommandPool(g_vulkanBackend.device, &commandPoolInfo, nullptr, &g_vulkanBackend.graphicsCommandPool);
    ASSERT_MSG(cmdPoolResult == VK_SUCCESS, "Err: failed to create graphics command pool");
}

static VkPresentModeKHR select_present_mode() {
    uint32_t presentModeCount = {0};
    const VkResult presentRes = vkGetPhysicalDeviceSurfacePresentModesKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &presentModeCount, nullptr);
    ASSERT_MSG(presentRes == VK_SUCCESS, "Err: failed to get physical device surface present modes");
    ASSERT(presentModeCount > 0);

    VkPresentModeKHR *presentModes = (VkPresentModeKHR *) mem_zalloc(sizeof(VkPresentModeKHR) * presentModeCount);
    const VkResult populateRes = vkGetPhysicalDeviceSurfacePresentModesKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &presentModeCount, presentModes);
    ASSERT_MSG(populateRes == VK_SUCCESS, "Err: failed to populate present modes array");

    VkPresentModeKHR selectedPresentMode = {VK_PRESENT_MODE_FIFO_KHR};
    VkPresentModeKHR preferredMode = g_userArguments.vsync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;
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

static void gfx_create_swap_chain() {
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

    uint32_t targetSwapChainImageCount = surfaceCapabilities.minImageCount + 1; // TODO: consider user option for target.
    targetSwapChainImageCount = min(targetSwapChainImageCount, BEET_SWAP_CHAIN_IMAGE_MAX);
    if ((surfaceCapabilities.maxImageCount > 0) && (targetSwapChainImageCount > surfaceCapabilities.maxImageCount)) {
        targetSwapChainImageCount = surfaceCapabilities.maxImageCount;
        assert(targetSwapChainImageCount <= BEET_SWAP_CHAIN_IMAGE_MAX);
    }
    g_vulkanBackend.swapChain.imageCount = targetSwapChainImageCount;

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

    VkResult swapChainImageResult = vkGetSwapchainImagesKHR(
            g_vulkanBackend.device,
            g_vulkanBackend.swapChain.swapChain,
            &g_vulkanBackend.swapChain.imageCount,
            &g_vulkanBackend.swapChain.images[0]
    );
    ASSERT_MSG(swapChainImageResult == VK_SUCCESS, "Err: failed to re-create swap chain images")

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

static void gfx_cleanup_swap_chain() {
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; i++) {
        vkDestroyImageView(g_vulkanBackend.device, g_vulkanBackend.swapChain.buffers[i].view, nullptr);
    }

    ASSERT_MSG(g_vulkanBackend.swapChain.surface != VK_NULL_HANDLE, "Err: swapchain surface has already been destroyed");
    vkDestroySwapchainKHR(g_vulkanBackend.device, g_vulkanBackend.swapChain.swapChain, nullptr);
    g_vulkanBackend.swapChain.swapChain = VK_NULL_HANDLE;
}

static void gfx_create_command_buffers() {
    const VkCommandBufferAllocateInfo gfxCommandBufferInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = g_vulkanBackend.graphicsCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = BEET_BUFFER_COUNT,
    };

    VkResult cmdBuffResult = vkAllocateCommandBuffers(g_vulkanBackend.device, &gfxCommandBufferInfo, g_vulkanBackend.graphicsCommandBuffers);
    ASSERT_MSG(cmdBuffResult == VK_SUCCESS, "Err: failed to create graphics command buffer");

    // === IMMEDIATE ===
    VkCommandBufferAllocateInfo immediateCommandBufferInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = g_vulkanBackend.graphicsCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };
    ASSERT(g_vulkanBackend.immediateCommandBuffer == VK_NULL_HANDLE);
    VkResult cmdImmediateBufferResult = vkAllocateCommandBuffers(g_vulkanBackend.device, &immediateCommandBufferInfo, &g_vulkanBackend.immediateCommandBuffer);
    ASSERT_MSG(cmdImmediateBufferResult == VK_SUCCESS, "Err: failed to create immediate command buffer");
}

static void gfx_cleanup_command_buffers() {
    // === GRAPHICS ===
    vkFreeCommandBuffers(g_vulkanBackend.device, g_vulkanBackend.graphicsCommandPool, BEET_BUFFER_COUNT, g_vulkanBackend.graphicsCommandBuffers);
    for (uint32_t i = 0; i < BEET_BUFFER_COUNT; ++i) {
        g_vulkanBackend.graphicsCommandBuffers[i] = VK_NULL_HANDLE;
    }

    // === IMMEDIATE ===
    vkFreeCommandBuffers(g_vulkanBackend.device, g_vulkanBackend.graphicsCommandPool, 1, &g_vulkanBackend.immediateCommandBuffer);
    g_vulkanBackend.immediateCommandBuffer = VK_NULL_HANDLE;
}

static void gfx_create_fences() {
    const VkFenceCreateInfo fenceCreateInfo = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            nullptr,
            VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; ++i) {
        const VkResult fenceRes = vkCreateFence(g_vulkanBackend.device, &fenceCreateInfo, nullptr, &g_vulkanBackend.graphicsFenceWait[i]);
        ASSERT_MSG(fenceRes == VK_SUCCESS, "Err: failed to create graphics fence [%u]", i);
    }
}

static void gfx_cleanup_fences() {
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; ++i) {
        vkDestroyFence(g_vulkanBackend.device, g_vulkanBackend.graphicsFenceWait[i], nullptr);
        g_vulkanBackend.graphicsFenceWait[i] = VK_NULL_HANDLE;
    }
}

static void gfx_create_color_buffer() {
    g_vulkanTargetFormats.colorFormat = g_vulkanTargetFormats.surfaceFormat.format; // we should consider adding a new find best format function

    VkImageCreateInfo colorImageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = g_vulkanTargetFormats.colorFormat,
            .extent = {.width = g_vulkanBackend.swapChain.width, .height = g_vulkanBackend.swapChain.height, .depth = 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = g_vulkanBackend.sampleCount,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VkResult createImgRes = vkCreateImage(g_vulkanBackend.device, &colorImageInfo, nullptr, &g_vulkanBackend.colorBuffer.image);
    ASSERT(createImgRes == VK_SUCCESS);

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(g_vulkanBackend.device, g_vulkanBackend.colorBuffer.image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = gfx_utils_get_memory_type(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    const VkResult allocRes = vkAllocateMemory(g_vulkanBackend.device, &allocInfo, nullptr, &g_vulkanBackend.colorBuffer.deviceMemory);
    ASSERT_MSG(allocRes == VK_SUCCESS, "Err: failed to allocate memory for color buffer")

    const VkResult bindRes = vkBindImageMemory(g_vulkanBackend.device, g_vulkanBackend.colorBuffer.image, g_vulkanBackend.colorBuffer.deviceMemory, 0);
    ASSERT_MSG(bindRes == VK_SUCCESS, "Err: failed to bind color buffer");

    VkImageViewCreateInfo colorImageViewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = g_vulkanBackend.colorBuffer.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = g_vulkanTargetFormats.colorFormat,
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            },
    };

    const VkResult createRes = vkCreateImageView(g_vulkanBackend.device, &colorImageViewInfo, nullptr, &g_vulkanBackend.colorBuffer.view);
    ASSERT_MSG(createRes == VK_SUCCESS, "Err: failed to create color image view");
}

static void gfx_cleanup_color_buffer() {
    vkDestroyImageView(g_vulkanBackend.device, g_vulkanBackend.colorBuffer.view, nullptr);
    vkDestroyImage(g_vulkanBackend.device, g_vulkanBackend.colorBuffer.image, nullptr);
    vkFreeMemory(g_vulkanBackend.device, g_vulkanBackend.colorBuffer.deviceMemory, nullptr);
}


static void gfx_create_depth_stencil_buffer() {
    g_vulkanTargetFormats.depthFormat = gfx_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);
    const VkImageCreateInfo depthImageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = g_vulkanTargetFormats.depthFormat,
            .extent = {
                    .width = g_vulkanBackend.swapChain.width,
                    .height = g_vulkanBackend.swapChain.height,
                    .depth =  1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = g_vulkanBackend.sampleCount,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    };

    const VkResult imgRes = vkCreateImage(g_vulkanBackend.device, &depthImageInfo, nullptr, &g_vulkanBackend.depthStencilBuffer.image);
    ASSERT_MSG(imgRes == VK_SUCCESS, "Err: failed to create depth stencil image");

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(g_vulkanBackend.device, g_vulkanBackend.depthStencilBuffer.image, &memoryRequirements);

    const VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = gfx_utils_get_memory_type(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    const VkResult allocRes = vkAllocateMemory(g_vulkanBackend.device, &allocInfo, nullptr, &g_vulkanBackend.depthStencilBuffer.deviceMemory);
    ASSERT_MSG(allocRes == VK_SUCCESS, "Err: failed to allocate memory for depth stencil")

    const VkResult bindRes = vkBindImageMemory(g_vulkanBackend.device, g_vulkanBackend.depthStencilBuffer.image, g_vulkanBackend.depthStencilBuffer.deviceMemory, 0);
    ASSERT_MSG(bindRes == VK_SUCCESS, "Err: failed to bind depth stencil");

    VkImageViewCreateInfo depthImageViewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = g_vulkanBackend.depthStencilBuffer.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = g_vulkanTargetFormats.depthFormat,
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            }
    };

    if (g_vulkanTargetFormats.depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        depthImageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    const VkResult createRes = vkCreateImageView(g_vulkanBackend.device, &depthImageViewInfo, nullptr, &g_vulkanBackend.depthStencilBuffer.view);
    ASSERT_MSG(createRes == VK_SUCCESS, "Err: failed to create depth image view");
}


static void gfx_cleanup_depth_stencil_buffer() {
    vkDestroyImageView(g_vulkanBackend.device, g_vulkanBackend.depthStencilBuffer.view, nullptr);
    vkDestroyImage(g_vulkanBackend.device, g_vulkanBackend.depthStencilBuffer.image, nullptr);
    vkFreeMemory(g_vulkanBackend.device, g_vulkanBackend.depthStencilBuffer.deviceMemory, nullptr);
}

static void gfx_create_resolve_depth_buffer() {
    g_vulkanTargetFormats.depthFormat = gfx_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);
    const VkImageCreateInfo depthImageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = g_vulkanTargetFormats.depthFormat,
            .extent = {
                    .width = g_vulkanBackend.swapChain.width,
                    .height = g_vulkanBackend.swapChain.height,
                    .depth =  1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    const VkResult imgRes = vkCreateImage(g_vulkanBackend.device, &depthImageInfo, nullptr, &g_vulkanBackend.resolvedDepthBuffer.image);
    ASSERT_MSG(imgRes == VK_SUCCESS, "Err: failed to create depth stencil image");

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(g_vulkanBackend.device, g_vulkanBackend.resolvedDepthBuffer.image, &memoryRequirements);

    const VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = gfx_utils_get_memory_type(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    const VkResult allocRes = vkAllocateMemory(g_vulkanBackend.device, &allocInfo, nullptr, &g_vulkanBackend.resolvedDepthBuffer.deviceMemory);
    ASSERT_MSG(allocRes == VK_SUCCESS, "Err: failed to allocate memory for depth stencil")

    const VkResult bindRes = vkBindImageMemory(g_vulkanBackend.device, g_vulkanBackend.resolvedDepthBuffer.image, g_vulkanBackend.resolvedDepthBuffer.deviceMemory, 0);
    ASSERT_MSG(bindRes == VK_SUCCESS, "Err: failed to bind depth stencil");

    VkImageViewCreateInfo depthImageViewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = g_vulkanBackend.resolvedDepthBuffer.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = g_vulkanTargetFormats.depthFormat,
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            }
    };

    const VkResult createRes = vkCreateImageView(g_vulkanBackend.device, &depthImageViewInfo, nullptr, &g_vulkanBackend.resolvedDepthBuffer.view);
    ASSERT_MSG(createRes == VK_SUCCESS, "Err: failed to create depth image view");
}

static void gfx_cleanup_resolve_depth_stencil_buffer() {
    vkDestroyImageView(g_vulkanBackend.device, g_vulkanBackend.resolvedDepthBuffer.view, nullptr);
    vkDestroyImage(g_vulkanBackend.device, g_vulkanBackend.resolvedDepthBuffer.image, nullptr);
    vkFreeMemory(g_vulkanBackend.device, g_vulkanBackend.resolvedDepthBuffer.deviceMemory, nullptr);
}

static void gfx_create_pipeline_cache() {
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    const VkResult cacheRes = vkCreatePipelineCache(g_vulkanBackend.device, &pipelineCacheCreateInfo, nullptr, &g_vulkanBackend.pipelineCache);
    ASSERT_MSG(cacheRes == VK_SUCCESS, "Err: failed to create pipeline cache")
}

static void gfx_cleanup_pipeline_cache() {
    ASSERT_MSG(g_vulkanBackend.pipelineCache != VK_NULL_HANDLE, "Err: pipeline cache has already been destroyed");
    vkDestroyPipelineCache(g_vulkanBackend.device, g_vulkanBackend.pipelineCache, nullptr);
    g_vulkanBackend.pipelineCache = VK_NULL_HANDLE;
}

static void gfx_flush() {
    if (g_vulkanBackend.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(g_vulkanBackend.device);
    }
}

static void gfx_render_frame(const VkCommandBuffer &cmdBuffer) {
    constexpr VkPipelineStageFlags submitPipelineStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &g_vulkanBackend.semaphores[gfx_last_swap_chain_index()].presentDone,
            .pWaitDstStageMask = &submitPipelineStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &g_vulkanBackend.semaphores[gfx_swap_chain_index()].renderDone,
    };

    const VkResult submitRes = vkQueueSubmit(g_vulkanBackend.queue, 1, &submitInfo, g_vulkanBackend.graphicsFenceWait[gfx_swap_chain_index()]);
    ASSERT(submitRes == VK_SUCCESS);
}

static VkResult gfx_acquire_next_swap_chain_image() {
    return vkAcquireNextImageKHR(
            g_vulkanBackend.device,
            g_vulkanBackend.swapChain.swapChain,
            UINT64_MAX,
            g_vulkanBackend.semaphores[gfx_last_swap_chain_index()].presentDone,
            VK_NULL_HANDLE,
            &g_vulkanBackend.swapChain.currentImageIndex
    );
}

static VkResult gfx_present() {
    VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &g_vulkanBackend.swapChain.swapChain,
            .pImageIndices = &g_vulkanBackend.swapChain.currentImageIndex,
    };

    if (g_vulkanBackend.semaphores[gfx_swap_chain_index()].renderDone != VK_NULL_HANDLE) {
        presentInfo.pWaitSemaphores = &g_vulkanBackend.semaphores[gfx_swap_chain_index()].renderDone;
        presentInfo.waitSemaphoreCount = 1;
    }

    return vkQueuePresentKHR(g_vulkanBackend.queue, &presentInfo);
}

static void begin_command_recording(const VkCommandBuffer &cmdBuffer) {
    const VkCommandBufferBeginInfo cmdBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
    };

    const VkResult result = vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);
    ASSERT_MSG(result == VK_SUCCESS, "Err: Vulkan failed to begin command buffer recording");
}

static void end_command_recording(const VkCommandBuffer &cmdBuffer) {
    vkEndCommandBuffer(cmdBuffer);
}

static bool query_has_valid_extent_size() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &surfaceCapabilities);
    return surfaceCapabilities.currentExtent.width != 0 || surfaceCapabilities.currentExtent.height != 0;
}

static void gfx_window_resize() {
    if (!query_has_valid_extent_size()) {
        return;
    }
    gfx_flush();

    gfx_cleanup_color_buffer();
    gfx_cleanup_depth_stencil_buffer();
    gfx_cleanup_resolve_depth_stencil_buffer();
    gfx_cleanup_swap_chain();

    gfx_create_swap_chain();
    gfx_create_color_buffer();
    gfx_create_depth_stencil_buffer();
    gfx_create_resolve_depth_buffer();
}

static void gfx_update_uniform_buffers() {
    //TODO: UPDATE UBO with camera info i.e. view & proj.
    const CameraEntity &camEntity = *db_get_camera_entity(0);
    const Camera &camera = *db_get_camera(camEntity.cameraIndex);
    const Transform &camTransform = *db_get_transform(camEntity.transformIndex);

    const vec3f camForward = glm::quat(camTransform.rotation) * WORLD_FORWARD;
    const vec3f lookTarget = camTransform.position + camForward;

    const mat4f view = lookAt(camTransform.position, lookTarget, WORLD_UP);
    const vec2f screen = {g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height};
    mat4f proj = perspective(as_radians(camera.fov), (float) screen.x / (float) screen.y, camera.zNear, camera.zFar);
    proj[1][1] *= -1; // flip view proj, need to switch to the vulkan glm define to fix this.
    mat4f viewProj = proj * view;

    const SceneUBO uniformBuffData{
            .projection = proj,
            .view = view,
            .position = camTransform.position,
            .unused_0 = {},
    };
    memcpy(g_vulkanBackend.uniformBuffer.mappedData, &uniformBuffData, sizeof(SceneUBO));
}

void blit_depth_stencil_to_resolved_depth(VkCommandBuffer &cmdBuffer,
                                             const GfxImageBuffer &srcBuffer,
                                             GfxImageBuffer &dstBuffer) {
    const uint32_t width = g_vulkanBackend.swapChain.width;
    const uint32_t height = g_vulkanBackend.swapChain.height;

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    // Transition the source image layout to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
    gfx_command_insert_memory_barrier(
            cmdBuffer,
            srcBuffer.image,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            subresourceRange
    );

    // Transition the destination image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
    gfx_command_insert_memory_barrier(
            cmdBuffer,
            dstBuffer.image,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            subresourceRange
    );

    // Define the source and destination regions for the blit operation.
    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {static_cast<int32_t>(width), static_cast<int32_t>(height), 1};

    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstOffsets[0] = {0, 0, 0};
    blitRegion.dstOffsets[1] = {static_cast<int32_t>(width), static_cast<int32_t>(height), 1};

    // Issue the blit command.
    vkCmdBlitImage(
            cmdBuffer,
            srcBuffer.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstBuffer.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blitRegion,
            VK_FILTER_NEAREST // Use nearest filtering
    );

    // Transition the source image layout back to VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.
    gfx_command_insert_memory_barrier(
            cmdBuffer,
            srcBuffer.image,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            subresourceRange
    );

    // Transition the destination image layout to VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL.
    gfx_command_insert_memory_barrier(
            cmdBuffer,
            dstBuffer.image,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            subresourceRange
    );
}

void resolve_depth_stencil_to_resolved_depth(VkCommandBuffer &cmdBuffer,
                                             const GfxImageBuffer &srcBuffer,
                                             GfxImageBuffer &dstBuffer) {
    const uint32_t width = g_vulkanBackend.swapChain.width;
    const uint32_t height = g_vulkanBackend.swapChain.height;

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            srcBuffer.image,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}
    );

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            dstBuffer.image,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}
    );
    VkImageResolve resolveRegion{
            .srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .layerCount = 1,},
            .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT , .layerCount = 1,},
            .extent = {.width = width, .height = height, .depth = 1}
    };

    vkCmdResolveImage(
            cmdBuffer,
            srcBuffer.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstBuffer.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &resolveRegion
    );

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            srcBuffer.image,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}
    );

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            dstBuffer.image,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}
    );
}


void resolve_color_buffer_to_swapchain(VkCommandBuffer &cmdBuffer) {
    gfx_command_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.colorBuffer.image,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    );

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.swapChain.buffers[gfx_swap_chain_index()].image,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    );

    VkImageResolve resolveRegion{
            .srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1,},
            .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1,},
            .extent {.width = g_vulkanBackend.swapChain.width, .height = g_vulkanBackend.swapChain.height, .depth = 1,}
    };

    vkCmdResolveImage(
            cmdBuffer,
            g_vulkanBackend.colorBuffer.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            g_vulkanBackend.swapChain.buffers[gfx_swap_chain_index()].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &resolveRegion
    );
}

[[maybe_unused]] void blit_color_buffer_to_swapchain(VkCommandBuffer &cmdBuffer) {

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.swapChain.buffers[gfx_swap_chain_index()].image,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    );

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.colorBuffer.image,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    );

    const VkImageBlit blitRegion{
            .srcSubresource {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1},
            .srcOffsets = {
                    {.x = 0, .y = 0, .z = 0},
                    {.x = int32_t(g_vulkanBackend.swapChain.width), .y = int32_t(g_vulkanBackend.swapChain.height), .z = 1}
            },
            .dstSubresource {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1},
            .dstOffsets = {
                    {.x = 0, .y = 0, .z = 0},
                    {.x = int32_t(g_vulkanBackend.swapChain.width), .y = int32_t(g_vulkanBackend.swapChain.height), .z = 1}
            },
    };

    vkCmdBlitImage(
            cmdBuffer,
            g_vulkanBackend.colorBuffer.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            g_vulkanBackend.swapChain.buffers[gfx_swap_chain_index()].image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blitRegion,
            VK_FILTER_LINEAR
    );
}

static void gfx_dynamic_render(VkCommandBuffer &cmdBuffer) {
    const bool isMultisampling = (g_vulkanBackend.sampleCount != VK_SAMPLE_COUNT_1_BIT);

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            isMultisampling ? g_vulkanBackend.colorBuffer.image : g_vulkanBackend.swapChain.buffers[gfx_swap_chain_index()].image,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1,}
    );
    gfx_command_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.depthStencilBuffer.image,
            0,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}
    );

    const VkRenderingAttachmentInfoKHR colorAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = isMultisampling ? g_vulkanBackend.colorBuffer.view : g_vulkanBackend.swapChain.buffers[gfx_swap_chain_index()].view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color = {{0.5f, 0.092f, 0.167f, 1.0f}},},
    };

    const VkRenderingAttachmentInfoKHR depthStencilAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = g_vulkanBackend.depthStencilBuffer.view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {.depthStencil = {.depth = 1.0f, .stencil = 0}},
    };

    const VkRenderingInfoKHR renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {.offset = {0, 0}, .extent = {g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height}},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment,
            .pDepthAttachment = &depthStencilAttachment,
            .pStencilAttachment = &depthStencilAttachment,
    };

    if(isMultisampling){
        resolve_depth_stencil_to_resolved_depth(cmdBuffer, g_vulkanBackend.depthStencilBuffer, g_vulkanBackend.resolvedDepthBuffer);
    }
    else{
        blit_depth_stencil_to_resolved_depth(cmdBuffer, g_vulkanBackend.depthStencilBuffer, g_vulkanBackend.resolvedDepthBuffer);
    }

    {
        gfx_command_begin_rendering(cmdBuffer, renderingInfo);
        {
            const VkViewport viewport = {0, 0, float(g_vulkanBackend.swapChain.width), float(g_vulkanBackend.swapChain.height), 0.0f, 1.0f};
            vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

            const VkRect2D scissor = {0, 0, g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height};
            vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

            gfx_sky_draw(cmdBuffer);
            gfx_lit_draw(cmdBuffer);
            gfx_triangle_strip_draw(cmdBuffer);
            gfx_line_draw(cmdBuffer);
#if BEET_GFX_IMGUI
            gfx_imgui_draw(cmdBuffer);
#endif // BEET_GFX_IMGUI
        }
        gfx_command_end_rendering(cmdBuffer);
    }

    if (isMultisampling) {
        resolve_color_buffer_to_swapchain(cmdBuffer);
    }

    gfx_command_insert_memory_barrier(
            cmdBuffer,
            g_vulkanBackend.swapChain.buffers[gfx_swap_chain_index()].image,
            isMultisampling ? VK_ACCESS_TRANSFER_WRITE_BIT : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            isMultisampling ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            isMultisampling ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    );
}

static void gfx_create_semaphores() {
    for (int i = 0; i < BEET_SWAP_CHAIN_IMAGE_MAX; ++i) {
        VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        const VkResult presentRes = vkCreateSemaphore(g_vulkanBackend.device, &semaphoreInfo, nullptr, &g_vulkanBackend.semaphores[i].presentDone);
        ASSERT_MSG(presentRes == VK_SUCCESS, "Err: failed to create present semaphore");

        const VkResult renderRes = vkCreateSemaphore(g_vulkanBackend.device, &semaphoreInfo, nullptr, &g_vulkanBackend.semaphores[i].renderDone);
        ASSERT_MSG(renderRes == VK_SUCCESS, "Err: failed to create render semaphore");
    }
}

static void gfx_cleanup_semaphores() {
    for (int i = 0; i < BEET_SWAP_CHAIN_IMAGE_MAX; ++i) {
        vkDestroySemaphore(g_vulkanBackend.device, g_vulkanBackend.semaphores[i].presentDone, nullptr);
        vkDestroySemaphore(g_vulkanBackend.device, g_vulkanBackend.semaphores[i].renderDone, nullptr);
    }
}

static void gfx_create_uniform_buffers() {
    const VkResult uniformResult = gfx_buffer_create(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            g_vulkanBackend.uniformBuffer,
            sizeof(SceneUBO),
            nullptr
    );
    ASSERT(uniformResult == VK_SUCCESS);

    const VkResult mapResult = vkMapMemory(g_vulkanBackend.device, g_vulkanBackend.uniformBuffer.memory, 0, VK_WHOLE_SIZE, 0, &g_vulkanBackend.uniformBuffer.mappedData);
    ASSERT(mapResult == VK_SUCCESS);
}

static void gfx_cleanup_uniform_buffers() {
    vkDestroyBuffer(g_vulkanBackend.device, g_vulkanBackend.uniformBuffer.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, g_vulkanBackend.uniformBuffer.memory, nullptr);
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create(void *windowHandle) {
#if BEET_CONVERT_ON_DEMAND
    gfx_converter_init(BEET_CMAKE_PIPELINE_ASSETS_DIR, BEET_CMAKE_RUNTIME_ASSETS_DIR);
#endif //BEET_CONVERT_ON_DEMAND

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
    gfx_create_color_buffer();
    gfx_create_depth_stencil_buffer();
    gfx_create_resolve_depth_buffer();
    gfx_create_pipeline_cache();
    gfx_create_samplers();
    gfx_create_function_pointers();

    gfx_create_uniform_buffers();

#if BEET_GFX_IMGUI
    gfx_create_imgui(windowHandle);
#endif //BEET_GFX_IMGUI
    gfx_create_sky();
    gfx_create_lit();
    gfx_create_line();
    gfx_create_triangle_strip();
}

void gfx_cleanup() {
    gfx_cleanup_uniform_buffers();

    gfx_cleanup_triangle_strip();
    gfx_cleanup_line();
    gfx_cleanup_lit();
    gfx_cleanup_sky();
#if BEET_GFX_IMGUI
    gfx_cleanup_imgui();
#endif //BEET_GFX_IMGUI
    gfx_cleanup_samplers();
    gfx_cleanup_pipeline_cache();
    gfx_cleanup_color_buffer();
    gfx_cleanup_depth_stencil_buffer();
    gfx_cleanup_resolve_depth_stencil_buffer();
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
//======================================================================================================================

//===API================================================================================================================
void gfx_update(const double &deltaTime) {
    g_vulkanBackend.swapChain.lastImageIndex = gfx_swap_chain_index();
    const VkResult nextRes = gfx_acquire_next_swap_chain_image();
    if (nextRes == VK_ERROR_OUT_OF_DATE_KHR) {
        gfx_window_resize();
        return;
    } else if (nextRes < 0) {
        ASSERT(nextRes == VK_SUCCESS)
    }
    vkWaitForFences(g_vulkanBackend.device, 1, &g_vulkanBackend.graphicsFenceWait[gfx_swap_chain_index()], true, UINT64_MAX);
    vkResetFences(g_vulkanBackend.device, 1, &g_vulkanBackend.graphicsFenceWait[gfx_swap_chain_index()]);

    gfx_update_uniform_buffers();

    VkCommandBuffer cmdBuffer = g_vulkanBackend.graphicsCommandBuffers[gfx_buffer_index()];
    vkResetCommandBuffer(cmdBuffer, 0);
    begin_command_recording(cmdBuffer);
    {
        gfx_dynamic_render(cmdBuffer);
    }
    end_command_recording(cmdBuffer);
    gfx_render_frame(cmdBuffer);

    gfx_present();
    gfx_flush();

    s_vulkanBackendInternal.currentFrame++;
}

uint32_t gfx_buffer_index() {
    return (s_vulkanBackendInternal.currentFrame % BEET_BUFFER_COUNT);
}

uint32_t gfx_swap_chain_index() {
    return (g_vulkanBackend.swapChain.currentImageIndex);
}

uint32_t gfx_last_swap_chain_index() {
    return (g_vulkanBackend.swapChain.lastImageIndex);
}

vec2i gfx_screen_size() {
    return {g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height};
}

uint32_t get_multisample_count(){
    return (uint32_t)g_userArguments.msaa;
}

//======================================================================================================================