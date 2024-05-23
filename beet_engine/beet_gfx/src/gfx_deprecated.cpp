#include <beet_gfx/gfx_deprecated.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_utils.h>

#include <beet_shared/assert.h>
#include <beet_shared/memory.h>

#include <vulkan/vulkan_core.h>

extern VulkanBackend g_vulkanBackend;

void gfx_create_deprecated_frame_buffer() {
    ASSERT(g_vulkanBackend.deprecated_frameBuffers == nullptr);
    g_vulkanBackend.deprecated_frameBuffers = (VkFramebuffer *) mem_zalloc(sizeof(VkFramebuffer) * g_vulkanBackend.swapChain.imageCount);

    constexpr uint32_t ATTACHMENT_COUNT = 2;
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; ++i) {
        const VkImageView attachments[ATTACHMENT_COUNT] = {g_vulkanBackend.swapChain.buffers[i].view, g_vulkanBackend.depthStencilBuffer.view};

        VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.pNext = nullptr;
        framebufferInfo.renderPass = g_vulkanBackend.deprecated_renderPass;
        framebufferInfo.attachmentCount = ATTACHMENT_COUNT;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = g_vulkanBackend.swapChain.width;
        framebufferInfo.height = g_vulkanBackend.swapChain.height;
        framebufferInfo.layers = 1;

        const VkResult frameBuffRes = vkCreateFramebuffer(g_vulkanBackend.device, &framebufferInfo, nullptr, &g_vulkanBackend.deprecated_frameBuffers[i]);
        ASSERT_MSG(frameBuffRes == VK_SUCCESS, "Err: failed to create frame buffer [%u]", i);
    }
}

void gfx_cleanup_deprecated_frame_buffer() {
    for (uint32_t i = 0; i < g_vulkanBackend.swapChain.imageCount; i++) {
        vkDestroyFramebuffer(g_vulkanBackend.device, g_vulkanBackend.deprecated_frameBuffers[i], nullptr);
    }
    mem_free(g_vulkanBackend.deprecated_frameBuffers);
    g_vulkanBackend.deprecated_frameBuffers = nullptr;
}

void gfx_create_deprecated_render_pass() {
    constexpr uint32_t ATTACHMENT_SIZE = 2;
    VkAttachmentDescription attachments[2] = {0};
    attachments[0].format = gfx_utils_select_surface_format().format;
    attachments[0].samples = g_vulkanBackend.sampleCount;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[1].format = gfx_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);
    attachments[1].samples = g_vulkanBackend.sampleCount;
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

    const VkResult result = vkCreateRenderPass(g_vulkanBackend.device, &renderPassInfo, nullptr, &g_vulkanBackend.deprecated_renderPass);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create render pass");
}

void gfx_cleanup_deprecated_render_pass() {
    ASSERT_MSG(g_vulkanBackend.deprecated_renderPass != VK_NULL_HANDLE, "Err: render pass has already been destroyed");
    vkDestroyRenderPass(g_vulkanBackend.device, g_vulkanBackend.deprecated_renderPass, nullptr);
    g_vulkanBackend.deprecated_renderPass = VK_NULL_HANDLE;
}

void gfx_deprecated_render_pass(VkCommandBuffer &cmdBuffer) {
    constexpr uint32_t clearValueCount = 2;
    VkClearValue clearValues[2];
    clearValues[0].color = {{0.5f, 0.092f, 0.167f, 1.0f}};
    clearValues[1].depthStencil.depth = 1.0f;

    VkRenderPassBeginInfo renderPassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassBeginInfo.renderPass = g_vulkanBackend.deprecated_renderPass;
    renderPassBeginInfo.framebuffer = g_vulkanBackend.deprecated_frameBuffers[g_vulkanBackend.swapChain.currentImageIndex]; // pretty sure this should use gfx_buffer_index instead.
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

//        gfx_indexed_indirect_render(cmdBuffer); // Disabled for now

    }
    vkCmdEndRenderPass(cmdBuffer);
}