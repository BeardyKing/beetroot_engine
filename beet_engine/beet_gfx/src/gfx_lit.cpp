#include <beet_gfx/gfx_lit.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_descriptors.h>
#include <beet_gfx/gfx_shader.h>
#include <beet_gfx/gfx_pipeline.h>
#include <beet_gfx/db_asset.h>

#include <beet_shared/assert.h>
#include <beet_shared/beet_types.h>

#include <beet_math/quat.h>

#include <vulkan/vulkan_core.h>

//===INTERNAL_STRUCTS===================================================================================================
struct LitPushConstantBuffer {
    mat4 model;
};

static struct VulkanLit {
    VkDescriptorSetLayout descriptorSetLayout = {VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool = {VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout = {VK_NULL_HANDLE};
    VkPipeline pipeline = {VK_NULL_HANDLE};
} g_gfxLit;

extern VulkanBackend g_vulkanBackend;
extern TargetVulkanFormats g_vulkanTargetFormats;
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static void gfx_create_lit_descriptor_set_layout() {
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
    const VkResult createPoolRes = (vkCreateDescriptorPool(g_vulkanBackend.device, &descriptorPoolInfo, nullptr, &g_gfxLit.descriptorPool));
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
    const VkResult descriptorResult = vkCreateDescriptorSetLayout(g_vulkanBackend.device, &descriptorSetLayoutCreateInfo, nullptr, &g_gfxLit.descriptorSetLayout);
    ASSERT(descriptorResult == VK_SUCCESS);
}

static void gfx_create_lit_pipeline_layout() {
    constexpr static uint32_t pushConstantRangeCount = 1;
    VkPushConstantRange pushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(LitPushConstantBuffer),
    };

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &g_gfxLit.descriptorSetLayout,
            .pushConstantRangeCount = pushConstantRangeCount,
            .pPushConstantRanges = &pushConstantRange,
    };
    const VkResult pipelineLayoutRes = vkCreatePipelineLayout(g_vulkanBackend.device, &pipelineLayoutCreateInfo, nullptr, &g_gfxLit.pipelineLayout);
    ASSERT(pipelineLayoutRes == VK_SUCCESS);
}

static bool gfx_create_lit_pipelines(VkPipeline &outLitPipeline) {
    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = gfx_pipeline_input_assembly_create(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    const VkPipelineRasterizationStateCreateInfo rasterizationState = gfx_pipeline_rasterization_create(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
                                                                                                        VK_FRONT_FACE_COUNTER_CLOCKWISE);
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

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gfx_graphics_pipeline_create(); // using VK_KHR_dynamic_rendering we can skip passing a render pass
    pipelineCreateInfo.layout = g_gfxLit.pipelineLayout;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStagesCount;
    pipelineCreateInfo.pStages = &shaderStages[0];

    // setup dynamic rendering
    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .pNext = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &g_vulkanTargetFormats.surfaceFormat.format,
            .depthAttachmentFormat = g_vulkanTargetFormats.depthFormat,
            .stencilAttachmentFormat = g_vulkanTargetFormats.depthFormat,
    };
    pipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;

    const uint32_t bindingDescriptionsSize = 1;
    VkVertexInputBindingDescription bindingDescriptions[bindingDescriptionsSize] = {
            gfx_vertex_input_binding_desc(0, sizeof(GfxVertex), VK_VERTEX_INPUT_RATE_VERTEX),
    };

    constexpr uint32_t attributeDescriptionsSize = 4;
    VkVertexInputAttributeDescription attributeDescriptions[attributeDescriptionsSize] = {
            gfx_vertex_input_attribute_desc(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxVertex, pos)),    // 0: Position
            gfx_vertex_input_attribute_desc(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxVertex, normal)), // 1: Normal
            gfx_vertex_input_attribute_desc(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(GfxVertex, uv)),        // 2: Texture coordinates
            gfx_vertex_input_attribute_desc(0, 3, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxVertex, color)),  // 3: Color
    };

    VkPipelineVertexInputStateCreateInfo inputState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = bindingDescriptionsSize,
            .pVertexBindingDescriptions = &bindingDescriptions[0],

            .vertexAttributeDescriptionCount = attributeDescriptionsSize,
            .pVertexAttributeDescriptions = &attributeDescriptions[0],
    };
    pipelineCreateInfo.pVertexInputState = &inputState;

    shaderStages[0] = gfx_load_shader("assets/shaders/lit/lit.vert", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gfx_load_shader("assets/shaders/lit/lit.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
    const VkResult pipelineRes = (vkCreateGraphicsPipelines(g_vulkanBackend.device, g_vulkanBackend.pipelineCache, 1, &pipelineCreateInfo, nullptr, &outLitPipeline));
    ASSERT_MSG(pipelineRes == VK_SUCCESS, "Err: failed to create graphics pipeline");
    vkDestroyShaderModule(g_vulkanBackend.device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(g_vulkanBackend.device, shaderStages[1].module, nullptr);
    return (pipelineRes == VK_SUCCESS);
}
//======================================================================================================================

//===API================================================================================================================
void gfx_lit_draw(VkCommandBuffer &cmdBuffer) {
    const uint32_t litEntityCount = db_get_lit_entity_count();
    for (uint32_t i = 0; i < litEntityCount; ++i) {
        const LitEntity &entity = *db_get_lit_entity(i);
        const LitMaterial &material = *db_get_lit_material(entity.materialIndex);
        const VkDescriptorSet &descriptorSet = *db_get_descriptor_set(material.descriptorSetIndex);
        const Transform &transform = *db_get_transform(entity.transformIndex);
        const GfxMesh &mesh = *db_get_mesh(entity.meshIndex);

        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_gfxLit.pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_gfxLit.pipeline);

        const mat4 model = translate(mat4(1.0f), transform.position) * toMat4(quat(transform.rotation)) * scale(mat4(1.0f), transform.scale);
        const LitPushConstantBuffer pushConstantBuffer = {model};
        vkCmdPushConstants(cmdBuffer, g_gfxLit.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LitPushConstantBuffer), &pushConstantBuffer);

        const VkBuffer vertexBuffers[] = {mesh.vertBuffer};
        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cmdBuffer, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuffer, mesh.indexCount, 1, 0, 0, 0);
    }
}

void gfx_lit_update_material_descriptor(VkDescriptorSet &outDescriptorSet, const GfxTexture &albedoTexture) {
    VkDescriptorSetAllocateInfo allocInfo = gfx_descriptor_set_alloc_info(g_gfxLit.descriptorPool, &g_gfxLit.descriptorSetLayout, 1);
    const VkResult allocDescRes = vkAllocateDescriptorSets(g_vulkanBackend.device, &allocInfo, &outDescriptorSet);
    ASSERT(allocDescRes == VK_SUCCESS);
    // TODO:    Update this to a buffer of textures so we can modify the contents without needing to rebuild descriptors
    //          runtime packages will not need this as the content won't change.
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            // Binding 0: Vertex shader uniform buffer
            // Binding 1: albedoTexture // TODO: Add a per package loaded texture array
            gfx_descriptor_set_write(outDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &g_vulkanBackend.uniformBuffer.descriptor, 1),
            gfx_descriptor_set_write(outDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &albedoTexture.descriptor, 1)
    };
    vkUpdateDescriptorSets(g_vulkanBackend.device, uint32_t(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

bool gfx_rebuild_lit_pipeline() {
    VkPipeline newPipeline = {};
    if (gfx_create_lit_pipelines(newPipeline)) {
        vkDeviceWaitIdle(g_vulkanBackend.device);
        vkDestroyPipeline(g_vulkanBackend.device, g_gfxLit.pipeline, nullptr);
        g_gfxLit.pipeline = newPipeline;
        return true;
    }
    return false;
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_lit() {
    gfx_create_lit_descriptor_set_layout();
    gfx_create_lit_pipeline_layout();
    const bool pipelineResult = gfx_create_lit_pipelines(g_gfxLit.pipeline);
    ASSERT(pipelineResult);
}

void gfx_cleanup_lit() {
    vkDestroyDescriptorSetLayout(g_vulkanBackend.device, g_gfxLit.descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(g_vulkanBackend.device, g_gfxLit.descriptorPool, nullptr);
    vkDestroyPipeline(g_vulkanBackend.device, g_gfxLit.pipeline, nullptr);
    vkDestroyPipelineLayout(g_vulkanBackend.device, g_gfxLit.pipelineLayout, nullptr);
}
//======================================================================================================================