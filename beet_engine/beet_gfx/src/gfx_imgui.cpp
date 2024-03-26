#include <beet_gfx/gfx_imgui.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_utils.h>

#include <beet_shared/assert.h>

#include <vulkan/vulkan_core.h>

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_win32.h>

extern VulkanBackend g_vulkanBackend;

VkDescriptorPool g_imguiPool = {VK_NULL_HANDLE};
bool g_imguiFinishedRendering = {true};

void gfx_create_imgui(void *windowHandle) {
    constexpr uint32_t poolSize = 11;
    const VkDescriptorPoolSize descriptorPoolSizes[poolSize] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
    };

    const VkDescriptorPoolCreateInfo descriptorPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1000,
            .poolSizeCount = poolSize,
            .pPoolSizes = descriptorPoolSizes,
    };

    const VkResult poolResult = vkCreateDescriptorPool(g_vulkanBackend.device, &descriptorPoolInfo, nullptr, &g_imguiPool);
    ASSERT(poolResult == VK_SUCCESS);

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(windowHandle);
    ImGuiIO &io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    const VkFormat surfaceFormat = gfx_utils_select_surface_format().format;
    const VkFormat depthFormat = gfx_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);

    ImGui_ImplVulkan_InitInfo imguiInitInfo = {
            .Instance = g_vulkanBackend.instance,
            .PhysicalDevice = g_vulkanBackend.physicalDevice,
            .Device = g_vulkanBackend.device,
            .Queue = g_vulkanBackend.queue,
            .DescriptorPool = g_imguiPool,
            .MinImageCount = 3,
            .ImageCount = 3,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .UseDynamicRendering = true,
            .PipelineRenderingCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                    .colorAttachmentCount = 1,
                    .pColorAttachmentFormats = &surfaceFormat,
                    .depthAttachmentFormat = depthFormat,
                    .stencilAttachmentFormat = depthFormat,
            },
    };

    ImGui_ImplVulkan_Init(&imguiInitInfo);
    ImGui_ImplVulkan_CreateFontsTexture();
}

void gfx_cleanup_imgui() {
    ImGui_ImplVulkan_DestroyFontsTexture();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext(nullptr);
    vkDestroyDescriptorPool(g_vulkanBackend.device, g_imguiPool, nullptr);
}

void gfx_imgui_begin() {
    if (g_imguiFinishedRendering) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        g_imguiFinishedRendering = false;
    }
}

void gfx_imgui_end() {
    ImGui::EndFrame();
    g_imguiFinishedRendering = true;
}

void gfx_imgui_demo_window() {
    ImGui::ShowDemoWindow();
}

void gfx_imgui_draw(VkCommandBuffer &cmdBuffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer, nullptr);
    g_imguiFinishedRendering = true;
}
