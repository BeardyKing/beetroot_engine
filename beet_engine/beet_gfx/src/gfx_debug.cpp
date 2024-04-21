#include <beet_gfx/gfx_debug.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_function_pointers.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>

#include <vulkan/vulkan_core.h>

static constexpr VkDebugUtilsMessageSeverityFlagsEXT BEET_VK_DEBUG_UTILS_MESSAGE_SEVERITY =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

static constexpr VkDebugUtilsMessageTypeFlagsEXT BEET_VK_DEBUG_UTILS_MESSAGE_TYPE =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

extern VulkanBackend g_vulkanBackend;
extern PFN_vkCreateDebugUtilsMessengerEXT g_vkCreateDebugUtilsMessengerEXT_Func;
extern PFN_vkDestroyDebugUtilsMessengerEXT g_vkDestroyDebugUtilsMessengerEXT_Func;
extern PFN_vkSetDebugUtilsObjectNameEXT g_vkSetDebugUtilsObjectNameEXT_Func;

static struct VulkanDebug {
    VkDebugUtilsMessengerEXT debugUtilsMessenger = {VK_NULL_HANDLE};
} g_vulkanDebug = {};

static VkBool32 VKAPI_PTR validation_message_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageWarningLevel,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void */*userData*/) {

    switch (messageWarningLevel) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            log_error(MSG_GFX, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            ASSERT_MSG(false, "\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
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
    g_vkDestroyDebugUtilsMessengerEXT_Func(g_vulkanBackend.instance, g_vulkanDebug.debugUtilsMessenger, nullptr);
    g_vulkanDebug.debugUtilsMessenger = VK_NULL_HANDLE;
}

void gfx_create_debug_callbacks() {
    gfx_create_function_pointers_debug_util_messenger();
    const VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = BEET_VK_DEBUG_UTILS_MESSAGE_SEVERITY,
            .messageType = BEET_VK_DEBUG_UTILS_MESSAGE_TYPE,
            .pfnUserCallback = validation_message_callback,
    };
    g_vkCreateDebugUtilsMessengerEXT_Func(g_vulkanBackend.instance, &messengerCreateInfo, nullptr, &g_vulkanDebug.debugUtilsMessenger);
}