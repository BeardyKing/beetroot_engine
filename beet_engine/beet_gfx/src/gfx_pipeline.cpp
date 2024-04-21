#include <beet_gfx/gfx_pipeline.h>

VkPipelineInputAssemblyStateCreateInfo gfx_pipeline_input_assembly_create(
        const VkPrimitiveTopology topology,
        const VkPipelineInputAssemblyStateCreateFlags flags,
        const VkBool32 primitiveRestartEnable) {

    return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .topology = topology,
            .primitiveRestartEnable = primitiveRestartEnable,
    };
}

VkPipelineRasterizationStateCreateInfo gfx_pipeline_rasterization_create(
        const VkPolygonMode polygonMode,
        const VkCullModeFlags cullMode,
        const VkFrontFace frontFace,
        const VkPipelineRasterizationStateCreateFlags flags) {

    return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .depthClampEnable = VK_FALSE,
            .polygonMode = polygonMode,
            .cullMode = cullMode,
            .frontFace = frontFace,
            .lineWidth = 1.0f
    };
}
VkPipelineColorBlendAttachmentState gfx_pipeline_color_blend_attachment_state(
        const VkColorComponentFlags colorWriteMask,
        const VkBool32 blendEnable) {

    return {
            .blendEnable = blendEnable,
            .colorWriteMask = colorWriteMask,
    };
}

VkPipelineColorBlendStateCreateInfo gfx_pipeline_color_blend_state_create(
        uint32_t attachmentCount,
        const VkPipelineColorBlendAttachmentState *pAttachments) {

    return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .attachmentCount = attachmentCount,
            .pAttachments = pAttachments,
    };
}

VkPipelineDepthStencilStateCreateInfo gfx_pipeline_depth_stencil_state_create(
        VkBool32 depthTestEnable,
        VkBool32 depthWriteEnable,
        VkCompareOp depthCompareOp) {

    return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .depthTestEnable = depthTestEnable,
            .depthWriteEnable = depthWriteEnable,
            .depthCompareOp = depthCompareOp,
            .back = {
                    .compareOp = VK_COMPARE_OP_ALWAYS
            },
    };
}

VkPipelineViewportStateCreateInfo gfx_pipeline_viewport_state_create(
        uint32_t viewportCount,
        uint32_t scissorCount,
        VkPipelineViewportStateCreateFlags flags) {

    return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .viewportCount = viewportCount,
            .scissorCount = scissorCount,
    };
}

VkPipelineMultisampleStateCreateInfo gfx_pipeline_multisample_state_create(
        VkSampleCountFlagBits rasterizationSamples,
        VkPipelineMultisampleStateCreateFlags flags) {

    return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .rasterizationSamples = rasterizationSamples,
    };
}

VkPipelineDynamicStateCreateInfo gfx_pipeline_dynamic_state_create(
        const VkDynamicState *pDynamicStates,
        uint32_t dynamicStateCount,
        VkPipelineDynamicStateCreateFlags flags) {

    return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .dynamicStateCount = dynamicStateCount,
            .pDynamicStates = pDynamicStates,
    };
}

VkGraphicsPipelineCreateInfo gfx_graphics_pipeline_create() {
    return {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
    };
}

VkGraphicsPipelineCreateInfo gfx_graphics_pipeline_create(
        VkPipelineLayout layout,
        VkRenderPass renderPass,
        VkPipelineCreateFlags flags) {

    return {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .layout = layout,
            .renderPass = renderPass,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
    };
}

VkPipelineVertexInputStateCreateInfo gfx_pipeline_vertex_input_state_create() {
    return {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,};
}

VkVertexInputBindingDescription gfx_vertex_input_binding_desc(
        uint32_t binding,
        uint32_t stride,
        VkVertexInputRate inputRate) {
    return {
            .binding = binding,
            .stride = stride,
            .inputRate = inputRate,
    };
}

VkVertexInputAttributeDescription gfx_vertex_input_attribute_desc(
        uint32_t binding,
        uint32_t location,
        VkFormat format,
        uint32_t offset) {
    return {
            .location = location,
            .binding = binding,
            .format = format,
            .offset = offset,
    };
}