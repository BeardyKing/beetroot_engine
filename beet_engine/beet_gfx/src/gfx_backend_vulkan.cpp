#include <cstdlib>
#include <vulkan/vulkan.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>
#include <beet_gfx/gfx_vulkan_platform_defines.h>
#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/gfx_vulkan_surface.h>
#include <beet_shared/shared_utils.h>

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

struct QueueFamilyIndices {
    uint32_t graphics = {UINT32_MAX};
    uint32_t compute = {UINT32_MAX};
    uint32_t transfer = {UINT32_MAX};
    uint32_t present = {UINT32_MAX};
};

static struct VulkanDebug {
    VkDebugUtilsMessengerEXT debugUtilsMessenger = {VK_NULL_HANDLE};
} g_vulkanDebug;

static struct VulkanBackend {
    VkInstance instance = {VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice = {VK_NULL_HANDLE};
    VkDevice device = {VK_NULL_HANDLE};
    VkQueue queue = {VK_NULL_HANDLE};

    VkCommandPool commandPool = {VK_NULL_HANDLE};

    VkPhysicalDeviceProperties deviceProperties = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};

    VkExtensionProperties *supportedExtensions = {};
    uint32_t extensionsCount = {};
    VkLayerProperties *supportedValidationLayers = {};
    uint32_t validationLayersCount = {};

    VulkanSwapChain swapChain = {};
    QueueFamilyIndices queueFamilyIndices = {};

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
    vkDestroyDevice(g_vulkanBackend.device, nullptr);
    g_vulkanBackend.queue = nullptr;
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
    {
        enabledDeviceExtensions[deviceExtensionCount] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        deviceExtensionCount++;
    }

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
    vkDestroyCommandPool(g_vulkanBackend.device, g_vulkanBackend.commandPool, nullptr);
    g_vulkanBackend.commandPool = VK_NULL_HANDLE;
}

void gfx_create_command_pool() {
    VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolInfo.queueFamilyIndex = g_vulkanBackend.queueFamilyIndices.graphics;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult cmdPoolResult = vkCreateCommandPool(g_vulkanBackend.device, &commandPoolInfo, nullptr, &g_vulkanBackend.commandPool);
    ASSERT_MSG(cmdPoolResult == VK_SUCCESS, "Err: failed to create graphics command pool");
}

void gfx_update(const double &deltaTime) {}

void gfx_create(void *windowHandle) {
    gfx_create_instance();
    gfx_create_debug_callbacks();
    gfx_create_physical_device();
    gfx_create_surface(windowHandle, &g_vulkanBackend.instance, &g_vulkanBackend.swapChain.surface);
    gfx_create_queues();
    gfx_create_command_pool();
}

void gfx_cleanup() {
    gfx_cleanup_command_pool();
    gfx_cleanup_queues();
    gfx_cleanup_surface();
    gfx_cleanup_physical_device();
    gfx_cleanup_debug_callbacks();
    gfx_cleanup_instance();

    free(g_vulkanBackend.supportedExtensions);
    free(g_vulkanBackend.supportedValidationLayers);
}