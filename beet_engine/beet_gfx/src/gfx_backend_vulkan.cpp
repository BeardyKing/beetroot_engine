#include <cstdlib>
#include <vulkan/vulkan.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>
#include <beet_gfx/gfx_vulkan_platform_defines.h>
#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/gfx_vulkan_surface.h>
#include <beet_shared/shared_utils.h>
#include <vector>

static const char *BEET_VK_PHYSICAL_DEVICE_TYPE_MAPPING[] = {
        "VK_PHYSICAL_DEVICE_TYPE_OTHER",
        "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_CPU",
};

constexpr VkSurfaceFormatKHR BEET_TARGET_SWAPCHAIN_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

static struct UserArguments {
    uint32_t selectedPhysicalDeviceIndex = {};
    bool vsync = {true};
} g_userArguments;

struct SwapChainBuffers {
    VkImage image;
    VkImageView view;
};

struct DepthImage {
    VkImage image;
    VkDeviceMemory deviceMemory;
    VkImageView view;
};

struct VulkanSwapChain {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    uint32_t imageCount;
    VkImage *images = {nullptr};
    SwapChainBuffers *buffers = {nullptr};

    // TODO: consider this being a ptr to some global size instead
    uint32_t width = 1280;
    uint32_t height = 720;
};

struct QueueFamilyIndices {
    uint32_t graphics = {UINT32_MAX};
    uint32_t compute = {UINT32_MAX};
    uint32_t transfer = {UINT32_MAX};
    uint32_t present = {UINT32_MAX};
};

static struct VulkanDebug {
    VkDebugUtilsMessengerEXT debugUtilsMessenger = {VK_NULL_HANDLE};
} g_vulkanDebug;

struct TargetVulkanFormats {
    // TODO: this struct should only last a single "frame" would be a good candidate for the inevitable gfx backend arena allocator
    VkSurfaceFormatKHR surfaceFormat = {};
    VkFormat depthFormat = {};
} g_vulkanTargetFormats;

static struct VulkanBackend {
    VkInstance instance = {VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice = {VK_NULL_HANDLE};
    VkDevice device = {VK_NULL_HANDLE};
    VkQueue queue = {VK_NULL_HANDLE};

    VkCommandPool graphicsCommandPool = {VK_NULL_HANDLE};
    VkCommandBuffer graphicsCommandBuffers[BEET_VK_COMMAND_BUFFER_COUNT] = {};
    VkRenderPass renderPass = {VK_NULL_HANDLE};
    VkFence graphicsFenceWait[BEET_VK_COMMAND_BUFFER_COUNT] = {};

    DepthImage depthStencil = {};

    VkExtensionProperties *supportedExtensions = {};
    uint32_t extensionsCount = {};
    VkLayerProperties *supportedValidationLayers = {};
    uint32_t validationLayersCount = {};

    VulkanSwapChain swapChain = {};
    QueueFamilyIndices queueFamilyIndices = {};
    VkPipelineCache pipelineCache = {VK_NULL_HANDLE};

    VkPhysicalDeviceProperties deviceProperties = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};
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

    free(physicalDevices);
}

void gfx_cleanup_surface() {
    // gfx_create_surface(...) exists in gfx_vulkan_surface.h / gfx_vulkan_surface_windows.cpp
    ASSERT_MSG(g_vulkanBackend.swapChain.surface != VK_NULL_HANDLE, "Err: VkSurface has already been destroyed");
    vkDestroySurfaceKHR(g_vulkanBackend.instance, g_vulkanBackend.swapChain.surface, nullptr);
    g_vulkanBackend.swapChain.surface = VK_NULL_HANDLE;
}

static VkBool32 VKAPI_PTR validation_message_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageWarningLevel,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void */*userData*/) {

    switch (messageWarningLevel) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            log_error(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            log_warning(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            log_info(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            log_verbose(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        default: {
            ASSERT(callbackData && callbackData->pMessageIdName && callbackData->pMessage);
        }
    }
    return VK_FALSE;
}

void gfx_cleanup_debug_callbacks() {
    ASSERT_MSG(g_vulkanDebug.debugUtilsMessenger != VK_NULL_HANDLE, "Err: debug utils messenger has already been destroyed");
    vkDestroyDebugUtilsMessengerEXT_Func(g_vulkanBackend.instance, g_vulkanDebug.debugUtilsMessenger, nullptr);
    g_vulkanDebug.debugUtilsMessenger = VK_NULL_HANDLE;
}

void gfx_create_debug_callbacks() {
    VkInstance &instance = g_vulkanBackend.instance;
    vkCreateDebugUtilsMessengerEXT_Func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, BEET_VK_CREATE_DEBUG_UTIL_EXT);
    ASSERT_MSG(vkCreateDebugUtilsMessengerEXT_Func, "Err: failed to setup debug callback %s", BEET_VK_CREATE_DEBUG_UTIL_EXT);

    vkDestroyDebugUtilsMessengerEXT_Func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, BEET_VK_DESTROY_DEBUG_UTIL_EXT);
    ASSERT_MSG(vkDestroyDebugUtilsMessengerEXT_Func, "Err: failed to setup debug callback %s", BEET_VK_DESTROY_DEBUG_UTIL_EXT);

    vkSetDebugUtilsObjectNameEXT_Func = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetInstanceProcAddr(instance, BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT);
    ASSERT_MSG(vkSetDebugUtilsObjectNameEXT_Func, "Err: failed to setup debug callback %s", BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT);

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messengerCreateInfo.messageSeverity = BEET_VK_DEBUG_UTILS_MESSAGE_SEVERITY;
    messengerCreateInfo.messageType = BEET_VK_DEBUG_UTILS_MESSAGE_TYPE;
    messengerCreateInfo.pfnUserCallback = validation_message_callback;

    vkCreateDebugUtilsMessengerEXT_Func(instance, &messengerCreateInfo, nullptr, &g_vulkanDebug.debugUtilsMessenger);
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
    VkExtensionProperties *selectedPhysicalDeviceExtensions = (VkExtensionProperties *) malloc(sizeof(VkExtensionProperties) * devicePropertyCount);

    if (devicePropertyCount > 0) {
        vkEnumerateDeviceExtensionProperties(g_vulkanBackend.physicalDevice, nullptr, &devicePropertyCount, selectedPhysicalDeviceExtensions);
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanBackend.physicalDevice, &queueFamilyCount, nullptr);
    ASSERT(queueFamilyCount != 0);

    VkQueueFamilyProperties *queueFamilies = (VkQueueFamilyProperties *) malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
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

    VkPhysicalDeviceFeatures2 deviceFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    deviceFeatures.features.samplerAnisotropy = VK_TRUE;

    uint32_t deviceExtensionCount = 0;
    const uint32_t maxSupportedDeviceExtensions = 2;
    const char *enabledDeviceExtensions[maxSupportedDeviceExtensions];
    enabledDeviceExtensions[deviceExtensionCount] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    deviceExtensionCount++;

#if BEET_VK_COMPILE_VERSION_1_3
    const uint32_t runtimeVulkanVersion = g_vulkanBackend.deviceProperties.apiVersion;
    if (runtimeVulkanVersion >= BEET_VK_API_VERSION_1_3) {
        enabledDeviceExtensions[deviceExtensionCount] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
        deviceExtensionCount++;
    }
#endif

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.pNext = &deviceFeatures;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = deviceExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions;
    deviceCreateInfo.queueCreateInfoCount = currentQueueCount;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;

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

    free(queueFamilies);
    free(selectedPhysicalDeviceExtensions);
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

    VkPresentModeKHR *presentModes = (VkPresentModeKHR *) malloc(sizeof(VkPresentModeKHR) * presentModeCount);
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

    free(presentModes);
    return selectedPresentMode;
}

VkSurfaceFormatKHR select_surface_format() {
    uint32_t surfaceFormatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &surfaceFormatsCount, nullptr);

    VkSurfaceFormatKHR *surfaceFormats = (VkSurfaceFormatKHR *) malloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &surfaceFormatsCount, surfaceFormats);

    // try select best surface format
    VkSurfaceFormatKHR selectedSurfaceFormat{};
    if ((surfaceFormatsCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
        selectedSurfaceFormat = BEET_TARGET_SWAPCHAIN_FORMAT;
    } else {
        for (uint32_t i = 0; i < surfaceFormatsCount; ++i) {
            if ((surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
                (surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
                selectedSurfaceFormat = surfaceFormats[i];
                break;
            }
        }
    }
    free(surfaceFormats);
    return selectedSurfaceFormat;
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
    g_vulkanTargetFormats.surfaceFormat = select_surface_format();
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

    if (g_vulkanBackend.swapChain.images != nullptr) {
        free(g_vulkanBackend.swapChain.images);
    }
    g_vulkanBackend.swapChain.images = (VkImage *) malloc(sizeof(VkImage) * g_vulkanBackend.swapChain.imageCount);
    VkResult swapChainImageResult = vkGetSwapchainImagesKHR(g_vulkanBackend.device, g_vulkanBackend.swapChain.swapChain, &g_vulkanBackend.swapChain.imageCount,
                                                            &g_vulkanBackend.swapChain.images[0]);
    ASSERT_MSG(swapChainImageResult == VK_SUCCESS, "Err: failed to re-create swap chain images")

    if (g_vulkanBackend.swapChain.buffers != nullptr) {
        free(g_vulkanBackend.swapChain.buffers);
    }
    g_vulkanBackend.swapChain.buffers = (SwapChainBuffers *) malloc(sizeof(SwapChainBuffers) * g_vulkanBackend.swapChain.imageCount);
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

    free(g_vulkanBackend.swapChain.images);
    g_vulkanBackend.swapChain.images = nullptr;
    free(g_vulkanBackend.swapChain.buffers);
    g_vulkanBackend.swapChain.buffers = nullptr;
}

void gfx_create_command_buffers() {
    VkCommandBufferAllocateInfo commandBufferInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferInfo.commandPool = g_vulkanBackend.graphicsCommandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = BEET_VK_COMMAND_BUFFER_COUNT;

    VkResult cmdBuffResult = vkAllocateCommandBuffers(g_vulkanBackend.device, &commandBufferInfo, g_vulkanBackend.graphicsCommandBuffers);
    ASSERT_MSG(cmdBuffResult == VK_SUCCESS, "Err: failed to create graphics command buffer");
}

void gfx_cleanup_command_buffers() {
    vkFreeCommandBuffers(g_vulkanBackend.device, g_vulkanBackend.graphicsCommandPool, BEET_VK_COMMAND_BUFFER_COUNT, g_vulkanBackend.graphicsCommandBuffers);
    for (uint32_t i = 0; i < BEET_VK_COMMAND_BUFFER_COUNT; ++i) {
        g_vulkanBackend.graphicsCommandBuffers[i] = VK_NULL_HANDLE;
    }
}

void gfx_create_fences() {
    const VkFenceCreateInfo fenceCreateInfo = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            nullptr,
            VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (uint32_t i = 0; i < BEET_VK_COMMAND_BUFFER_COUNT; ++i) {
        const VkResult fenceRes = vkCreateFence(g_vulkanBackend.device, &fenceCreateInfo, nullptr, &g_vulkanBackend.graphicsFenceWait[i]);
        ASSERT_MSG(fenceRes == VK_SUCCESS, "Err: failed to create graphics fence [%u]", i);
    }
}

void gfx_cleanup_fences() {
    for (uint32_t i = 0; i < BEET_VK_COMMAND_BUFFER_COUNT; ++i) {
        vkDestroyFence(g_vulkanBackend.device, g_vulkanBackend.graphicsFenceWait[i], nullptr);
        g_vulkanBackend.graphicsFenceWait[i] = VK_NULL_HANDLE;
    }
}

VkFormat find_depth_format(const VkImageTiling &desiredTilingFormat) {
    constexpr VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    constexpr uint32_t FORMAT_CANDIDATE_COUNT = 5;
    constexpr VkFormat candidates[FORMAT_CANDIDATE_COUNT]{
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM,
    };

    for (uint32_t i = 0; i < FORMAT_CANDIDATE_COUNT; ++i) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(g_vulkanBackend.physicalDevice, candidates[i], &props);

        if ((desiredTilingFormat == VK_IMAGE_TILING_LINEAR) &&
            ((props.linearTilingFeatures & features) == features)) {
            return candidates[i];
        }
        if ((desiredTilingFormat == VK_IMAGE_TILING_OPTIMAL) &&
            ((props.optimalTilingFeatures & features) == features)) {
            return candidates[i];
        }
    }
    ASSERT_MSG(false, "Err: could not find supported depth format");
    return VK_FORMAT_UNDEFINED;
}

uint32_t gfx_get_memory_type(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < g_vulkanBackend.deviceMemoryProperties.memoryTypeCount; i++) {
        if ((memoryTypeBits & 1) == 1) {
            if ((g_vulkanBackend.deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        memoryTypeBits >>= 1;
    }
    ASSERT_MSG(false, "Err: fid not find memory type of target %u", properties);
    return UINT32_MAX; // TODO:TEST: validate this is the correct value to return to crash the backend.
}

void gfx_create_depth_stencil_buffer() {
    g_vulkanTargetFormats.depthFormat = find_depth_format(VK_IMAGE_TILING_OPTIMAL);

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
    allocInfo.memoryTypeIndex = gfx_get_memory_type(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    const VkResult allocRes = vkAllocateMemory(g_vulkanBackend.device, &allocInfo, nullptr, &g_vulkanBackend.depthStencil.deviceMemory);
    ASSERT_MSG(allocRes == VK_SUCCESS, "Err: failed to allocate memory for depth stencil")

    const VkResult bindRes = vkBindImageMemory(g_vulkanBackend.device, g_vulkanBackend.depthStencil.image, g_vulkanBackend.depthStencil.deviceMemory, 0);
    ASSERT_MSG(bindRes == VK_SUCCESS, "Err: failed to bind depth stencil");
}

void gfx_cleanup_depth_stencil_buffer() {
    vkDestroyImageView(g_vulkanBackend.device, g_vulkanBackend.depthStencil.view, nullptr);
    vkDestroyImage(g_vulkanBackend.device, g_vulkanBackend.depthStencil.image, nullptr);
    vkFreeMemory(g_vulkanBackend.device, g_vulkanBackend.depthStencil.deviceMemory, nullptr);
}

void gfx_create_render_pass() {
    constexpr uint32_t ATTACHMENT_SIZE = 2;
    VkAttachmentDescription attachments[2] = {0};
    attachments[0].format = g_vulkanTargetFormats.surfaceFormat.format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[1].format = g_vulkanTargetFormats.depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthStencilAttachmentRef = {};
    depthStencilAttachmentRef.attachment = 1;
    depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentRef;
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentRef;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;

    constexpr uint32_t SUBPASS_DEPENDENCY_SIZE = 2;
    VkSubpassDependency dependencies[SUBPASS_DEPENDENCY_SIZE] = {0};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = ATTACHMENT_SIZE;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = SUBPASS_DEPENDENCY_SIZE;
    renderPassInfo.pDependencies = dependencies;

    const VkResult result = vkCreateRenderPass(g_vulkanBackend.device, &renderPassInfo, nullptr, &g_vulkanBackend.renderPass);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create render pass");
}

void gfx_cleanup_render_pass() {
    ASSERT_MSG(g_vulkanBackend.renderPass != VK_NULL_HANDLE, "Err: render pass has already been destroyed");
    vkDestroyRenderPass(g_vulkanBackend.device, g_vulkanBackend.renderPass, nullptr);
    g_vulkanBackend.renderPass = VK_NULL_HANDLE;
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

void gfx_update(const double &deltaTime) {}

void gfx_create(void *windowHandle) {
    gfx_create_instance();
    gfx_create_debug_callbacks();
    gfx_create_physical_device();
    gfx_create_surface(windowHandle, &g_vulkanBackend.instance, &g_vulkanBackend.swapChain.surface);
    gfx_create_queues();
    gfx_create_command_pool();
    gfx_create_swap_chain();
    gfx_create_command_buffers();
    gfx_create_fences();
    gfx_create_depth_stencil_buffer();
    gfx_create_render_pass();
    gfx_create_pipeline_cache();
}

void gfx_cleanup() {
    gfx_cleanup_pipeline_cache();
    gfx_cleanup_render_pass();
    gfx_cleanup_depth_stencil_buffer();
    gfx_cleanup_fences();
    gfx_cleanup_command_buffers();
    gfx_cleanup_swap_chain();
    gfx_cleanup_command_pool();
    gfx_cleanup_queues();
    gfx_cleanup_surface();
    gfx_cleanup_physical_device();
    gfx_cleanup_debug_callbacks();
    gfx_cleanup_instance();

    free(g_vulkanBackend.supportedExtensions);
    free(g_vulkanBackend.supportedValidationLayers);
}