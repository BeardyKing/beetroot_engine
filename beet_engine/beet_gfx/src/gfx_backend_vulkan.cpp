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
#include <beet_gfx/gfx_pipeline.h>
#include <beet_gfx/gfx_descriptors.h>

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

constexpr uint32_t BEET_INSTANCE_BUFFER_BIND_ID = 1;
constexpr VkSurfaceFormatKHR BEET_TARGET_SWAPCHAIN_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM,
                                                             VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
static struct UserArguments {
    uint32_t selectedPhysicalDeviceIndex = {};
    bool vsync = {true};
} g_userArguments = {};

static struct VulkanDebug {
    VkDebugUtilsMessengerEXT debugUtilsMessenger = {VK_NULL_HANDLE};
} g_vulkanDebug = {};

struct TargetVulkanFormats {
    // TODO: this struct should only last a single "frame" would be a good candidate for the inevitable gfx backend arena allocator
    VkSurfaceFormatKHR surfaceFormat = {};
    VkFormat depthFormat = {};
} g_vulkanTargetFormats = {};


static struct textures {
    GfxTexture uvGrid;
} g_textures = {};

static struct meshes {
    GfxMesh cube;
} g_meshes = {};

//TODO: replace with db arr of structs with id lookups
static struct Objects {
    struct CamObj {
        Camera cam;
        Transform transform;
    } camObj;
} g_dbEntities = {};

struct VulkanBackend g_vulkanBackend = {};

void gfx_cleanup_indirect_commands();

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
        log_verbose(MSG_GFX, "Extension: %s \n", g_vulkanBackend.supportedExtensions[i].extensionName);
    }

    for (uint8_t i = 0; i < BEET_VK_EXTENSION_COUNT; i++) {
        const bool result = gfx_find_supported_extension(BEET_VK_EXTENSIONS[i]);
        ASSERT_MSG(result, "Err: failed find support for extension [%s]", BEET_VK_EXTENSIONS[i]);
    }

    vkEnumerateInstanceLayerProperties(&g_vulkanBackend.validationLayersCount, nullptr);
    ASSERT(g_vulkanBackend.supportedValidationLayers == nullptr);
    g_vulkanBackend.supportedValidationLayers = (VkLayerProperties *) mem_zalloc(
            sizeof(VkLayerProperties) * g_vulkanBackend.validationLayersCount);
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

static VkBool32 VKAPI_PTR validation_message_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageWarningLevel,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void */*userData*/) {

    switch (messageWarningLevel) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            log_error(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName,
                      callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            log_warning(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName,
                        callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            log_info(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            log_verbose(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName,
                        callbackData->pMessage);
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

VkSurfaceFormatKHR select_surface_format() {
    uint32_t surfaceFormatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &surfaceFormatsCount, nullptr);

    VkSurfaceFormatKHR *surfaceFormats = (VkSurfaceFormatKHR *) mem_zalloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatsCount);
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
    mem_free(surfaceFormats);
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

    ASSERT(g_vulkanBackend.swapChain.images == nullptr);
    g_vulkanBackend.swapChain.images = (VkImage *) mem_zalloc(sizeof(VkImage) * g_vulkanBackend.swapChain.imageCount);
    VkResult swapChainImageResult = vkGetSwapchainImagesKHR(g_vulkanBackend.device, g_vulkanBackend.swapChain.swapChain,
                                                            &g_vulkanBackend.swapChain.imageCount,
                                                            &g_vulkanBackend.swapChain.images[0]);
    ASSERT_MSG(swapChainImageResult == VK_SUCCESS, "Err: failed to re-create swap chain images")

    ASSERT(g_vulkanBackend.swapChain.buffers == nullptr);
    g_vulkanBackend.swapChain.buffers = (SwapChainBuffers *) mem_zalloc(
            sizeof(SwapChainBuffers) * g_vulkanBackend.swapChain.imageCount);
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

    // GRAPHICS
    ASSERT(g_vulkanBackend.graphicsCommandBuffers == nullptr);
    g_vulkanBackend.graphicsCommandBuffers = (VkCommandBuffer *) mem_zalloc(sizeof(VkCommandBuffer) * g_vulkanBackend.swapChain.imageCount);

    VkCommandBufferAllocateInfo commandBufferInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferInfo.commandPool = g_vulkanBackend.graphicsCommandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = g_vulkanBackend.swapChain.imageCount;

    VkResult cmdBuffResult = vkAllocateCommandBuffers(g_vulkanBackend.device, &commandBufferInfo, g_vulkanBackend.graphicsCommandBuffers);
    ASSERT_MSG(cmdBuffResult == VK_SUCCESS, "Err: failed to create graphics command buffer");


    // IMMEDIATE
    ASSERT(g_vulkanBackend.immediateCommandBuffer == VK_NULL_HANDLE);
    commandBufferInfo.commandBufferCount = 1;
    VkResult cmdImmediateBufferResult = vkAllocateCommandBuffers(g_vulkanBackend.device, &commandBufferInfo, &g_vulkanBackend.immediateCommandBuffer);
    ASSERT_MSG(cmdImmediateBufferResult == VK_SUCCESS, "Err: failed to create immediate command buffer");
}

void gfx_cleanup_command_buffers() {
    // GRAPHICS
    vkFreeCommandBuffers(g_vulkanBackend.device, g_vulkanBackend.graphicsCommandPool, g_vulkanBackend.swapChain.imageCount, g_vulkanBackend.graphicsCommandBuffers);
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; ++i) {
        g_vulkanBackend.graphicsCommandBuffers[i] = VK_NULL_HANDLE;
    }

    mem_free(g_vulkanBackend.graphicsCommandBuffers);
    g_vulkanBackend.graphicsCommandBuffers = nullptr;

    // IMMEDIATE
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

void gfx_create_frame_buffer() {
    ASSERT(g_vulkanBackend.frameBuffers == nullptr);
    g_vulkanBackend.frameBuffers = (VkFramebuffer *) mem_zalloc(sizeof(VkFramebuffer) * g_vulkanBackend.swapChain.imageCount);

    constexpr uint32_t ATTACHMENT_COUNT = 2;
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; ++i) {
        const VkImageView attachments[ATTACHMENT_COUNT] = {g_vulkanBackend.swapChain.buffers[i].view, g_vulkanBackend.depthStencil.view};

        VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.pNext = nullptr;
        framebufferInfo.renderPass = g_vulkanBackend.renderPass;
        framebufferInfo.attachmentCount = ATTACHMENT_COUNT;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = g_vulkanBackend.swapChain.width;
        framebufferInfo.height = g_vulkanBackend.swapChain.height;
        framebufferInfo.layers = 1;

        const VkResult frameBuffRes = vkCreateFramebuffer(g_vulkanBackend.device, &framebufferInfo, nullptr, &g_vulkanBackend.frameBuffers[i]);
        ASSERT_MSG(frameBuffRes == VK_SUCCESS, "Err: failed to create frame buffer [%u]", i);
    }
}

void gfx_cleanup_frame_buffer() {
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; i++) {
        vkDestroyFramebuffer(g_vulkanBackend.device, g_vulkanBackend.frameBuffers[i], nullptr);
    }
    mem_free(g_vulkanBackend.frameBuffers);
    g_vulkanBackend.frameBuffers = nullptr;
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

void gfx_render_pass(VkCommandBuffer &cmdBuffer) {
    constexpr uint32_t clearValueCount = 2;
    VkClearValue clearValues[2];
    clearValues[0].color = {{0.5f, 0.092f, 0.167f, 1.0f}};
    clearValues[1].depthStencil.depth = 1.0f;

    VkRenderPassBeginInfo renderPassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassBeginInfo.renderPass = g_vulkanBackend.renderPass;
    renderPassBeginInfo.framebuffer = g_vulkanBackend.frameBuffers[g_vulkanBackend.currentBufferIndex];
    renderPassBeginInfo.renderArea.extent.width = g_vulkanBackend.swapChain.width;
    renderPassBeginInfo.renderArea.extent.height = g_vulkanBackend.swapChain.height;
    renderPassBeginInfo.clearValueCount = clearValueCount;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    {
        VkViewport viewport = {0, 0, float(g_vulkanBackend.swapChain.width), float(g_vulkanBackend.swapChain.height), 0.0f, 1.0f};
        vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

        VkRect2D scissor = {0, 0, g_vulkanBackend.swapChain.width, g_vulkanBackend.swapChain.height};
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

        VkDeviceSize offsets[1] = {0};
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanBackend.pipelineLayout, 0, 1, &g_vulkanBackend.descriptorSet, 0, NULL);

        //cubePipeline
        // [POI] Instanced multi draw rendering of the cubes
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanBackend.cubePipeline);
        // Binding point 0 : Mesh vertex buffer
        // Binding point 1 : Instance data buffer
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &g_meshes.cube.vertBuffer, offsets);
        vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &g_vulkanBackend.instanceBuffer.buffer, offsets);

        vkCmdBindIndexBuffer(cmdBuffer, g_meshes.cube.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexedIndirect(cmdBuffer, g_vulkanBackend.indirectCommandsBuffer.buffer, 0, g_vulkanBackend.indirectDrawCount, sizeof(VkDrawIndexedIndirectCommand));

    }
    vkCmdEndRenderPass(cmdBuffer);
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

    gfx_cleanup_frame_buffer();
    gfx_cleanup_render_pass();
    gfx_cleanup_depth_stencil_buffer();
    gfx_cleanup_swap_chain();

    gfx_create_swap_chain();
    gfx_create_depth_stencil_buffer();
    gfx_create_render_pass();
    gfx_create_frame_buffer();
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
//    // copy to ubo
//    g_vulkanBackend.uniformData.projection = camera.matrices.perspective;
//    uniformData.view = camera.matrices.view;
    memcpy(g_vulkanBackend.uniformBuffer.mappedData, &uniformBuffData, sizeof(uniformBuffData));
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
        gfx_render_pass(cmdBuffer);
    }
    end_command_recording(cmdBuffer);

    //TODO: build draw commands
    gfx_render_frame();
    gfx_present();
    gfx_flush();
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

void gfx_command_begin_immediate_recording() {
    VkCommandBufferBeginInfo cmdBufBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(g_vulkanBackend.immediateCommandBuffer, &cmdBufBeginInfo);
}

void gfx_command_end_immediate_recording() {
    vkEndCommandBuffer(g_vulkanBackend.immediateCommandBuffer);

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_vulkanBackend.immediateCommandBuffer;

    //TODO:GFX replace immediate submit with transfer immediate submit.
    vkQueueSubmit(g_vulkanBackend.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_vulkanBackend.queue);
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
    memoryAllocateInfo.memoryTypeIndex = gfx_get_memory_type(memoryRequirements.memoryTypeBits,
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
    memoryAllocateInfo.memoryTypeIndex = gfx_get_memory_type(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
    view.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
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

VkResult gfx_buffer_create(const VkBufferUsageFlags &usageFlags, const VkMemoryPropertyFlags &memoryPropertyFlags, GfxBuffer &outBuffer, const VkDeviceSize size, void *inData) {
    VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;
    ASSERT(bufferCreateInfo.size > 0);

    const VkResult createResult = vkCreateBuffer(g_vulkanBackend.device, &bufferCreateInfo, nullptr, &outBuffer.buffer);
    ASSERT(createResult == VK_SUCCESS);

    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    vkGetBufferMemoryRequirements(g_vulkanBackend.device, outBuffer.buffer, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = gfx_get_memory_type(memReqs.memoryTypeBits, memoryPropertyFlags);

    // when outBuffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAllocInfo.pNext = &allocFlagsInfo;
    }
    const VkResult allocResult = vkAllocateMemory(g_vulkanBackend.device, &memAllocInfo, nullptr, &outBuffer.memory);
    ASSERT(allocResult == VK_SUCCESS);

    outBuffer.alignment = memReqs.alignment;
    outBuffer.size = size;
    outBuffer.usageFlags = usageFlags;
    outBuffer.memoryPropertyFlags = memoryPropertyFlags;

    if (inData != nullptr) {
        void *mapped;
        const VkResult VkMapRes = vkMapMemory(g_vulkanBackend.device, outBuffer.memory, 0, size, 0, &mapped);
        ASSERT(VkMapRes == VK_SUCCESS);

        memcpy(mapped, inData, size);
        // when host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mappedRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
            mappedRange.memory = outBuffer.memory;
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(g_vulkanBackend.device, 1, &mappedRange);
        }
        vkUnmapMemory(g_vulkanBackend.device, outBuffer.memory);
    }

    // Initialize a default descriptor that covers the whole buffer size
    outBuffer.descriptor.offset = 0;
    outBuffer.descriptor.buffer = outBuffer.buffer;
    outBuffer.descriptor.range = VK_WHOLE_SIZE;

    return vkBindBufferMemory(g_vulkanBackend.device, outBuffer.buffer, outBuffer.memory, 0);
}

VkResult
gfx_buffer_create(const VkBufferUsageFlags &usageFlags, const VkMemoryPropertyFlags &memoryPropertyFlags, const VkDeviceSize &size, VkBuffer &outBuffer, VkDeviceMemory &memory,
                  void *inData) {
    VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;
    ASSERT(bufferCreateInfo.size > 0);

    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    const VkResult createBuffer = vkCreateBuffer(g_vulkanBackend.device, &bufferCreateInfo, nullptr, &outBuffer);
    ASSERT(createBuffer == VK_SUCCESS);

    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

    vkGetBufferMemoryRequirements(g_vulkanBackend.device, outBuffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = gfx_get_memory_type(memReqs.memoryTypeBits, memoryPropertyFlags);

    // when outBuffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    const VkResult allocRes = vkAllocateMemory(g_vulkanBackend.device, &memAlloc, nullptr, &memory);
    ASSERT(allocRes == VK_SUCCESS);

    if (inData != nullptr) {
        void *mapped;
        const VkResult VkMapRes = vkMapMemory(g_vulkanBackend.device, memory, 0, size, 0, &mapped);
        ASSERT(VkMapRes == VK_SUCCESS);

        memcpy(mapped, inData, size);
        // when host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mappedRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
            mappedRange.memory = memory;
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(g_vulkanBackend.device, 1, &mappedRange);
        }
        vkUnmapMemory(g_vulkanBackend.device, memory);
    }

    const VkResult bindRes = vkBindBufferMemory(g_vulkanBackend.device, outBuffer, memory, 0);
    return bindRes;
}

void gfx_mesh_create_immediate(const RawMesh &rawMesh, GfxMesh &outMesh) {
    ASSERT((rawMesh.vertexCount > 0) && (rawMesh.indexCount > 0));

    struct StagingBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };
    StagingBuffer vertexStaging = {};
    StagingBuffer indexStaging = {};

    const size_t vertexBufferSize = sizeof(GfxVertex) * rawMesh.vertexCount;
    const size_t indexBufferSize = sizeof(uint32_t) * rawMesh.indexCount;

    outMesh.indexCount = rawMesh.indexCount;
    outMesh.vertCount = rawMesh.vertexCount;

    // Create staging buffers
    VkResult vertexCreateStageRes = gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBufferSize,
            vertexStaging.buffer,
            vertexStaging.memory,
            rawMesh.vertexData
    );
    ASSERT(vertexCreateStageRes == VK_SUCCESS);

    VkResult indexCreateStageRes = gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indexBufferSize,
            indexStaging.buffer,
            indexStaging.memory,
            rawMesh.indexData
    );
    ASSERT(indexCreateStageRes == VK_SUCCESS);

    // Create device local buffers
    const VkResult vertexCreateDeviceLocalRes = gfx_buffer_create(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBufferSize,
            outMesh.vertBuffer,
            outMesh.vertMemory,
            nullptr
    );
    ASSERT(vertexCreateDeviceLocalRes == VK_SUCCESS);
    // Index buffer
    const VkResult indexCreateDeviceLocalRes = gfx_buffer_create(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBufferSize,
            outMesh.indexBuffer,
            outMesh.indexMemory,
            nullptr
    );
    ASSERT(indexCreateDeviceLocalRes == VK_SUCCESS);

    gfx_command_begin_immediate_recording();
    {
        VkBufferCopy copyRegion = {};
        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(g_vulkanBackend.immediateCommandBuffer, vertexStaging.buffer, outMesh.vertBuffer, 1, &copyRegion);

        copyRegion.size = indexBufferSize;
        vkCmdCopyBuffer(g_vulkanBackend.immediateCommandBuffer, indexStaging.buffer, outMesh.indexBuffer, 1, &copyRegion);
    }
    gfx_command_end_immediate_recording();

    vkDestroyBuffer(g_vulkanBackend.device, vertexStaging.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, vertexStaging.memory, nullptr);
    vkDestroyBuffer(g_vulkanBackend.device, indexStaging.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, indexStaging.memory, nullptr);
}

void gfx_cube_create_immediate(GfxMesh &outMesh) {
    const uint32_t vertexCount = 24;
    static GfxVertex vertexData[vertexCount] = {
            //===POS================//===COLOUR=========//===UV======
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    };

    const uint32_t indexCount = 36;
    static uint32_t indexData[indexCount] = {
            0, 3, 2,
            2, 1, 0,
            4, 5, 6,
            6, 7, 4,
            11, 8, 9,
            9, 10, 11,
            12, 13, 14,
            14, 15, 12,
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
    };

    RawMesh rawMesh = {
            vertexData,
            indexData,
            vertexCount,
            indexCount,
    };

    gfx_mesh_create_immediate(rawMesh, outMesh);
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

#define OBJECT_INSTANCE_COUNT 1

void gfx_build_indirect_commands() {
    auto &indirectCommands = g_vulkanBackend.indirectCommands;
    auto &indirectCommandsBuffer = g_vulkanBackend.indirectCommandsBuffer;
    auto &indirectDrawCount = g_vulkanBackend.indirectDrawCount;
    auto &objectCount = g_vulkanBackend.objectCount;
    indirectCommands.clear();

    // TODO: Create on indirect command for each element in a package
    {
        uint32_t idx = 0;
        VkDrawIndexedIndirectCommand indirectCmd{};
        indirectCmd.instanceCount = OBJECT_INSTANCE_COUNT;
        indirectCmd.firstInstance = idx * OBJECT_INSTANCE_COUNT;
        indirectCmd.firstIndex = 0; // TODO: figure out what this should be when there are multiple instances
        indirectCmd.indexCount = g_meshes.cube.indexCount;

        indirectCommands.push_back(indirectCmd);

        idx++;
    }

    indirectDrawCount = uint32_t(indirectCommands.size());

    objectCount = 0;
    for (auto indirectCmd: indirectCommands) {
        objectCount += indirectCmd.instanceCount;
    }

    GfxBuffer stagingBuffer;
    const VkResult stagingResult = gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            indirectCommands.size() * sizeof(VkDrawIndexedIndirectCommand),
            indirectCommands.data()
    );
    ASSERT(stagingResult == VK_SUCCESS);

    const VkResult indirectCreateResult = gfx_buffer_create(
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indirectCommandsBuffer,
            stagingBuffer.size,
            nullptr
    );
    ASSERT(indirectCreateResult == VK_SUCCESS);

    gfx_command_begin_immediate_recording();
    {
        VkBufferCopy copyRegion = {};
        copyRegion.size = stagingBuffer.size;
        vkCmdCopyBuffer(g_vulkanBackend.immediateCommandBuffer, stagingBuffer.buffer, g_vulkanBackend.indirectCommandsBuffer.buffer, 1, &copyRegion);
    }
    gfx_command_end_immediate_recording();

    vkDestroyBuffer(g_vulkanBackend.device, stagingBuffer.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, stagingBuffer.memory, nullptr);
}

void gfx_cleanup_indirect_commands() {
    vkDestroyBuffer(g_vulkanBackend.device, g_vulkanBackend.indirectCommandsBuffer.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, g_vulkanBackend.indirectCommandsBuffer.memory, nullptr);
}

void gfx_buffer_copy_immediate(GfxBuffer &src, GfxBuffer &dst, VkQueue queue, VkBufferCopy *copyRegion) {
    ASSERT(dst.size <= src.size);
    ASSERT(src.buffer);
    gfx_command_begin_immediate_recording();
    {
        VkBufferCopy bufferCopy{};
        if (copyRegion == nullptr) {
            bufferCopy.size = src.size;
        } else {
            bufferCopy = *copyRegion;
        }
        vkCmdCopyBuffer(g_vulkanBackend.immediateCommandBuffer, src.buffer, dst.buffer, 1, &bufferCopy);
    }
    gfx_command_end_immediate_recording();
}

void gfx_build_instance_data() {
    constexpr uint32_t objectCount = 1;
    std::vector<GfxInstanceData> instanceData;
    instanceData.resize(objectCount);

//    std::default_random_engine rndEngine(benchmark.active ? 0 : (unsigned)time(nullptr));
//    std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);
//TODO: add to obj struct
    for (uint32_t i = 0; i < objectCount; i++) {
        instanceData[i].rot = vec3f(0);
        instanceData[i].pos = vec3f(-2, 1, -12);
        instanceData[i].scale = 1.0f;
        instanceData[i].texIndex = i / OBJECT_INSTANCE_COUNT;
    }

    GfxBuffer stagingBuffer;
    gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            instanceData.size() * sizeof(GfxInstanceData),
            instanceData.data()
    );

    gfx_buffer_create(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            g_vulkanBackend.instanceBuffer,
            stagingBuffer.size,
            nullptr
    );

    gfx_buffer_copy_immediate(stagingBuffer, g_vulkanBackend.instanceBuffer, g_vulkanBackend.queue, nullptr);

    vkDestroyBuffer(g_vulkanBackend.device, stagingBuffer.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, stagingBuffer.memory, nullptr);
}

void gfx_build_uniform_buffers() {
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


void gfx_build_descriptor_set_layout() {
    //=== POOL =====//
    constexpr uint32_t poolSizeCount = 2;
    VkDescriptorPoolSize poolSizes[poolSizeCount] = {
            VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1},
            VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1},
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 2,
            .poolSizeCount = poolSizeCount,
            .pPoolSizes = &poolSizes[0],
    };
    const VkResult createPoolRes = (vkCreateDescriptorPool(g_vulkanBackend.device, &descriptorPoolInfo, nullptr, &g_vulkanBackend.descriptorPool));
    ASSERT(createPoolRes == VK_SUCCESS);

    //=== LAYOUT ===//
    constexpr uint32_t layoutBindingsCount = 2;
    VkDescriptorSetLayoutBinding layoutBindings[layoutBindingsCount] = {
            {VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            }},
            {VkDescriptorSetLayoutBinding{
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            }},
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = layoutBindingsCount,
            .pBindings = &layoutBindings[0],
    };
    const VkResult descriptorResult = vkCreateDescriptorSetLayout(g_vulkanBackend.device, &descriptorSetLayoutCreateInfo, nullptr, &g_vulkanBackend.descriptorSetLayout);
    ASSERT(descriptorResult == VK_SUCCESS);

    //=== SET ======//
    VkDescriptorSetAllocateInfo allocInfo = gfx_descriptor_set_alloc_info(g_vulkanBackend.descriptorPool, &g_vulkanBackend.descriptorSetLayout, 1);
    const VkResult allocDescRes = vkAllocateDescriptorSets(g_vulkanBackend.device, &allocInfo, &g_vulkanBackend.descriptorSet);
    ASSERT(allocDescRes == VK_SUCCESS);
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            // Binding 0: Vertex shader uniform buffer
            // Binding 1: uv grid image // TODO: Add a per package loaded texture array
            gfx_descriptor_set_write(g_vulkanBackend.descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &g_vulkanBackend.uniformBuffer.descriptor, 1),
            gfx_descriptor_set_write(g_vulkanBackend.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &g_textures.uvGrid.descriptor, 1)
    };
    vkUpdateDescriptorSets(g_vulkanBackend.device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void gfx_cleanup_descriptor_set_layout() {
    vkDestroyPipelineLayout(g_vulkanBackend.device, g_vulkanBackend.pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(g_vulkanBackend.device, g_vulkanBackend.descriptorSetLayout, nullptr);
}

VkShaderModule gfx_load_shader_binary(const char *path) {
    //TODO Refactor this to using a new binary FS api that uses fstat as we shouldn't use tellg on a binary.
    std::ifstream is(path, std::ios::binary | std::ios::in | std::ios::ate);
    ASSERT_MSG(is.is_open(), "Err: Failed to open shader %s", path);
    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);
    char *shaderCode = (char *) malloc(sizeof(char) * size);
    is.read(shaderCode, size);
    is.close();
    assert(size > 0);

    VkShaderModuleCreateInfo moduleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = size,
            .pCode = (uint32_t *) shaderCode,
    };
    VkShaderModule shaderModule = {};
    const VkResult moduleResult = vkCreateShaderModule(g_vulkanBackend.device, &moduleCreateInfo, nullptr, &shaderModule);
    ASSERT(moduleResult == VK_SUCCESS);

    free(shaderCode);
    return shaderModule;
}

VkPipelineShaderStageCreateInfo gfx_load_shader(const char *path, VkShaderStageFlagBits stage) {
    VkPipelineShaderStageCreateInfo shaderStage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage,
            .module = gfx_load_shader_binary(path),
            .pName = "main",
    };
    assert(shaderStage.module != VK_NULL_HANDLE);
    return shaderStage;
}

void gfx_build_pipelines() {

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &g_vulkanBackend.descriptorSetLayout
    };
    const VkResult pipelineLayoutRes = vkCreatePipelineLayout(g_vulkanBackend.device, &pipelineLayoutCreateInfo, nullptr, &g_vulkanBackend.pipelineLayout);
    ASSERT(pipelineLayoutRes == VK_SUCCESS);

    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = gfx_pipeline_input_assembly_create(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    const VkPipelineRasterizationStateCreateInfo rasterizationState = gfx_pipeline_rasterization_create(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    const VkPipelineColorBlendAttachmentState blendAttachmentState = gfx_pipeline_color_blend_attachment_state(0xf, VK_FALSE);
    const VkPipelineColorBlendStateCreateInfo colorBlendState = gfx_pipeline_color_blend_state_create(1, &blendAttachmentState);
    const VkPipelineDepthStencilStateCreateInfo depthStencilState = gfx_pipeline_depth_stencil_state_create(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    const VkPipelineViewportStateCreateInfo viewportState = gfx_pipeline_viewport_state_create(1, 1, 0);
    const VkPipelineMultisampleStateCreateInfo multisampleState = gfx_pipeline_multisample_state_create(VK_SAMPLE_COUNT_1_BIT, 0);

    constexpr uint32_t dynamicStateCount = 2;
    const VkDynamicState dynamicStateEnables[dynamicStateCount] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState = gfx_pipeline_dynamic_state_create(dynamicStateEnables, dynamicStateCount, 0);

    constexpr uint32_t shaderStagesCount = 2;
    VkPipelineShaderStageCreateInfo shaderStages[shaderStagesCount] = {};

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gfx_graphics_pipeline_create(g_vulkanBackend.pipelineLayout, g_vulkanBackend.renderPass, 0);
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStagesCount;
    pipelineCreateInfo.pStages = &shaderStages[0];

    const uint32_t bindingDescriptionsSize = 2;
    VkVertexInputBindingDescription bindingDescriptions[bindingDescriptionsSize] = {
            gfx_vertex_input_binding_desc(0, sizeof(GfxVertex), VK_VERTEX_INPUT_RATE_VERTEX),
            gfx_vertex_input_binding_desc(BEET_INSTANCE_BUFFER_BIND_ID, sizeof(GfxInstanceData), VK_VERTEX_INPUT_RATE_VERTEX),
    };

    constexpr uint32_t attributeDescriptionsSize = 8;
    VkVertexInputAttributeDescription attributeDescriptions[attributeDescriptionsSize] = {
            gfx_vertex_input_attribute_desc(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxVertex, pos)),                                  // 0: Position
            gfx_vertex_input_attribute_desc(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxVertex, normal)),                               // 1: Normal
            gfx_vertex_input_attribute_desc(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(GfxVertex, uv)),                                      // 2: Texture coordinates
            gfx_vertex_input_attribute_desc(0, 3, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxVertex, normal)),                               // 3: Color

            gfx_vertex_input_attribute_desc(BEET_INSTANCE_BUFFER_BIND_ID, 4, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxInstanceData, pos)), // 4: Position
            gfx_vertex_input_attribute_desc(BEET_INSTANCE_BUFFER_BIND_ID, 5, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxInstanceData, rot)), // 5: Rotation
            gfx_vertex_input_attribute_desc(BEET_INSTANCE_BUFFER_BIND_ID, 6, VK_FORMAT_R32_SFLOAT, offsetof(GfxInstanceData, scale)),     // 6: Scale
            gfx_vertex_input_attribute_desc(BEET_INSTANCE_BUFFER_BIND_ID, 7, VK_FORMAT_R32_SINT, offsetof(GfxInstanceData, texIndex)),    // 7: Texture array layer index
    };

    VkPipelineVertexInputStateCreateInfo inputState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = bindingDescriptionsSize,
            .pVertexBindingDescriptions = &bindingDescriptions[0],

            .vertexAttributeDescriptionCount = attributeDescriptionsSize,
            .pVertexAttributeDescriptions = &attributeDescriptions[0],
    };
    pipelineCreateInfo.pVertexInputState = &inputState;

    shaderStages[0] = gfx_load_shader("../assets/shaders/indirectdraw.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gfx_load_shader("../assets/shaders/indirectdraw.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    const VkResult pipelineRes = (vkCreateGraphicsPipelines(g_vulkanBackend.device, g_vulkanBackend.pipelineCache, 1, &pipelineCreateInfo, nullptr, &g_vulkanBackend.cubePipeline));
    ASSERT_MSG(pipelineRes == VK_SUCCESS, "Err: failed to create graphics pipeline");
    vkDestroyShaderModule(g_vulkanBackend.device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(g_vulkanBackend.device, shaderStages[1].module, nullptr);
}

void gfx_cleanup_pipelines() {
    vkDestroyPipeline(g_vulkanBackend.device, g_vulkanBackend.cubePipeline, nullptr);
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
    gfx_create_render_pass();
    gfx_create_pipeline_cache();
    gfx_create_frame_buffer();
    gfx_create_samplers();

    gfx_load_packages();
    gfx_build_indirect_commands();
    gfx_build_instance_data();
    gfx_build_uniform_buffers();
    gfx_build_descriptor_set_layout();
    gfx_build_pipelines();

    //TODO: 1) DONE - MANUAL load scene/package into memory
    //TODO: 2) DONE - build indirect draw commands
    //TODO: 3) DONE - build UBO
    //TODO: 4) DONE - descriptor set layout
    //TODO: 5) DONE - create pipelines
    //TODO: 6) create descriptors/pools
    //TODO: 7) build command buffers
}

void gfx_cleanup() {
    gfx_cleanup_pipelines();
    gfx_cleanup_descriptor_set_layout();
    gfx_cleanup_uniform_buffers();
    gfx_cleanup_indirect_commands();
    gfx_unload_packages();

    gfx_cleanup_samplers();
    gfx_cleanup_frame_buffer();
    gfx_cleanup_pipeline_cache();
    gfx_cleanup_render_pass();
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
}