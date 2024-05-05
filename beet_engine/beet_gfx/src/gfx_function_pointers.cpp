#include <beet_gfx/gfx_function_pointers.h>
#include <beet_gfx/gfx_types.h>

#include <beet_shared/assert.h>

//===INTERNAL_STRUCTS===================================================================================================
extern VulkanBackend g_vulkanBackend;

PFN_vkCmdBeginRenderingKHR g_vkCmdBeginRenderingKHR_Func = {VK_NULL_HANDLE};
PFN_vkCmdEndRenderingKHR g_vkCmdEndRenderingKHR_Func = {VK_NULL_HANDLE};
static constexpr char BEET_VK_CMD_BEGIN_RENDERING_KHR[] = "vkCmdBeginRenderingKHR";
static constexpr char BEET_VK_CMD_END_RENDERING_KHR[] = "vkCmdEndRenderingKHR";

PFN_vkCreateDebugUtilsMessengerEXT g_vkCreateDebugUtilsMessengerEXT_Func = {VK_NULL_HANDLE};
PFN_vkDestroyDebugUtilsMessengerEXT g_vkDestroyDebugUtilsMessengerEXT_Func = {VK_NULL_HANDLE};
PFN_vkSetDebugUtilsObjectNameEXT g_vkSetDebugUtilsObjectNameEXT_Func = {VK_NULL_HANDLE};
static constexpr char BEET_VK_CREATE_DEBUG_UTIL_EXT[] = "vkCreateDebugUtilsMessengerEXT";
static constexpr char BEET_VK_DESTROY_DEBUG_UTIL_EXT[] = "vkDestroyDebugUtilsMessengerEXT";
static constexpr char BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT[] = "vkSetDebugUtilsObjectNameEXT";
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static void gfx_cleanup_function_pointers_dynamic_rendering() {
    ASSERT_MSG(g_vkCmdBeginRenderingKHR_Func != VK_NULL_HANDLE, "vulkan function pointer has already been invalidated");
    ASSERT_MSG(g_vkCmdEndRenderingKHR_Func != VK_NULL_HANDLE, "vulkan function pointer has already been invalidated");
    g_vkCmdBeginRenderingKHR_Func = {VK_NULL_HANDLE};
    g_vkCmdEndRenderingKHR_Func = {VK_NULL_HANDLE};
}

static void gfx_cleanup_function_pointers_debug_util_messenger() {
    ASSERT_MSG(g_vkCreateDebugUtilsMessengerEXT_Func != VK_NULL_HANDLE, "vulkan function pointer has already been invalidated");
    ASSERT_MSG(g_vkDestroyDebugUtilsMessengerEXT_Func != VK_NULL_HANDLE, "vulkan function pointer has already been invalidated");
    ASSERT_MSG(g_vkSetDebugUtilsObjectNameEXT_Func != VK_NULL_HANDLE, "vulkan function pointer has already been invalidated");
    g_vkCreateDebugUtilsMessengerEXT_Func = {VK_NULL_HANDLE};
    g_vkDestroyDebugUtilsMessengerEXT_Func = {VK_NULL_HANDLE};
    g_vkSetDebugUtilsObjectNameEXT_Func = {VK_NULL_HANDLE};
}
//======================================================================================================================

//===API================================================================================================================
void gfx_create_function_pointers_dynamic_rendering() {
    g_vkCmdBeginRenderingKHR_Func = PFN_vkCmdBeginRenderingKHR(vkGetDeviceProcAddr(g_vulkanBackend.device, BEET_VK_CMD_BEGIN_RENDERING_KHR));
    g_vkCmdEndRenderingKHR_Func = PFN_vkCmdEndRenderingKHR(vkGetDeviceProcAddr(g_vulkanBackend.device, BEET_VK_CMD_END_RENDERING_KHR));
    ASSERT(g_vkCmdBeginRenderingKHR_Func != VK_NULL_HANDLE);
    ASSERT(g_vkCmdEndRenderingKHR_Func != VK_NULL_HANDLE);
}

void gfx_create_function_pointers_debug_util_messenger() {
    VkInstance &instance = g_vulkanBackend.instance;
    g_vkCreateDebugUtilsMessengerEXT_Func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, BEET_VK_CREATE_DEBUG_UTIL_EXT);
    ASSERT_MSG(g_vkCreateDebugUtilsMessengerEXT_Func, "Err: failed to setup debug callback %s", BEET_VK_CREATE_DEBUG_UTIL_EXT);

    g_vkDestroyDebugUtilsMessengerEXT_Func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, BEET_VK_DESTROY_DEBUG_UTIL_EXT);
    ASSERT_MSG(g_vkDestroyDebugUtilsMessengerEXT_Func, "Err: failed to setup debug callback %s", BEET_VK_DESTROY_DEBUG_UTIL_EXT);

    g_vkSetDebugUtilsObjectNameEXT_Func = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetInstanceProcAddr(instance, BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT);
    ASSERT_MSG(g_vkSetDebugUtilsObjectNameEXT_Func, "Err: failed to setup debug callback %s", BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT);
}

static constexpr char BEET_VK_CREATE_LINE_RASTERIZATION_MODE_EXT[] = "vkCmdSetLineRasterizationModeEXT";
PFN_vkCmdSetLineRasterizationModeEXT g_vkCmdSetLineRasterizationModeEXT_Func = {VK_NULL_HANDLE};

static void gfx_create_function_pointers_line_rasterization_mode(){
    g_vkCmdSetLineRasterizationModeEXT_Func = (PFN_vkCmdSetLineRasterizationModeEXT) vkGetInstanceProcAddr(g_vulkanBackend.instance, BEET_VK_CREATE_LINE_RASTERIZATION_MODE_EXT);
    ASSERT(g_vkCmdSetLineRasterizationModeEXT_Func != VK_NULL_HANDLE);
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_function_pointers() {
    gfx_create_function_pointers_dynamic_rendering();
//    gfx_create_function_pointers_line_rasterization_mode();//FIXME: This crashes
};

void gfx_cleanup_function_pointers() {
    gfx_cleanup_function_pointers_dynamic_rendering();
    gfx_cleanup_function_pointers_debug_util_messenger();
};
//======================================================================================================================





