#include <beet_gfx/gfx_triangle_strip.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_descriptors.h>
#include <beet_gfx/gfx_shader.h>
#include <beet_gfx/gfx_pipeline.h>
#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/gfx_samplers.h>

#include <beet_shared/assert.h>

#include <beet_math/quat.h>

//===INTERNAL_STRUCTS===================================================================================================
static struct TriangleStrip {
    VkDescriptorSetLayout descriptorSetLayout = {VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout = {VK_NULL_HANDLE};
    VkPipeline pipeline = {VK_NULL_HANDLE};

    VkDescriptorPool descriptorPools[BEET_BUFFER_COUNT] = {VK_NULL_HANDLE};
    VkDescriptorSet descriptorSets[BEET_BUFFER_COUNT] = {VK_NULL_HANDLE};
    GfxBuffer triangleStripUniformBuffers[BEET_BUFFER_COUNT] = {VK_NULL_HANDLE};
} s_triangleStrip;

#define MAX_TRIANGLE_STRIP_ENTITY_SIZE (1024 * 1)
struct TriangleStripEntity {
    uint32_t triangleStripRangeStart = {0};
    uint32_t triangleStripRangeEnd = {0};
};
static TriangleStripEntity s_triangleStripEntityPool[MAX_TRIANGLE_STRIP_ENTITY_SIZE] = {};
static uint32_t s_triangleStripEntityCount = 0;

#define MAX_POINT_SIZE (1024 * 4)
static uint32_t s_pointCount = 0;

extern VulkanBackend g_vulkanBackend;
extern TargetVulkanFormats g_vulkanTargetFormats;
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static uint32_t add_point(const LinePoint3D &point) {
    //write directly to the mappedData i.e. avoid memcpy from pool to mappedData
    ((LinePoint3D *) s_triangleStrip.triangleStripUniformBuffers[gfx_buffer_index()].mappedData)[s_pointCount] = point;
    s_pointCount++;
    ASSERT(s_pointCount < MAX_POINT_SIZE);
    return s_pointCount;
}

static void gfx_create_triangle_strip_material_descriptor() {
    for (int i = 0; i < BEET_BUFFER_COUNT; ++i) {
        const VkDescriptorSetAllocateInfo allocInfo = gfx_descriptor_set_alloc_info(s_triangleStrip.descriptorPools[i], &s_triangleStrip.descriptorSetLayout, 1);
        const VkResult allocDescRes = vkAllocateDescriptorSets(g_vulkanBackend.device, &allocInfo, &s_triangleStrip.descriptorSets[i]);
        ASSERT(allocDescRes == VK_SUCCESS);
    }
}

static void gfx_create_triangle_strip_uniform_buffers() {
    for (uint32_t i = 0; i < BEET_BUFFER_COUNT; ++i) {
        const VkResult uniformResult = gfx_buffer_create(
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                s_triangleStrip.triangleStripUniformBuffers[i],
                (sizeof(LinePoint3D) * MAX_POINT_SIZE),
                nullptr);
        ASSERT(uniformResult == VK_SUCCESS);

        const VkResult mapResult = vkMapMemory(g_vulkanBackend.device, s_triangleStrip.triangleStripUniformBuffers[i].memory, 0, VK_WHOLE_SIZE, 0,
                                               &s_triangleStrip.triangleStripUniformBuffers[i].mappedData);
        ASSERT(mapResult == VK_SUCCESS);
    }
}

static void gfx_cleanup_triangle_strip_uniform_buffers() {
    for (uint32_t i = 0; i < BEET_BUFFER_COUNT; ++i) {
        vkDestroyBuffer(g_vulkanBackend.device, s_triangleStrip.triangleStripUniformBuffers[i].buffer, nullptr);
        vkFreeMemory(g_vulkanBackend.device, s_triangleStrip.triangleStripUniformBuffers[i].memory, nullptr);
    }
}

static void gfx_create_triangle_strip_descriptor_set_layout() {
    constexpr uint32_t poolSizeCount = 3;
    VkDescriptorPoolSize poolSizes[poolSizeCount] = {
            VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1},
            VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1},
            VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1},
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 2,
            .poolSizeCount = poolSizeCount,
            .pPoolSizes = &poolSizes[0],
    };
    for (uint32_t i = 0; i < BEET_BUFFER_COUNT; ++i) {
        const VkResult createPoolRes = vkCreateDescriptorPool(g_vulkanBackend.device, &descriptorPoolInfo, nullptr, &s_triangleStrip.descriptorPools[i]);
        ASSERT(createPoolRes == VK_SUCCESS);
    }

    constexpr uint32_t layoutBindingsCount = 3;
    VkDescriptorSetLayoutBinding layoutBindings[layoutBindingsCount] = {
            {VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            }},
            {VkDescriptorSetLayoutBinding{
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            }},
            {VkDescriptorSetLayoutBinding{
                    .binding = 2,
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
    const VkResult descriptorResult = vkCreateDescriptorSetLayout(g_vulkanBackend.device, &descriptorSetLayoutCreateInfo, nullptr, &s_triangleStrip.descriptorSetLayout);
    ASSERT(descriptorResult == VK_SUCCESS);
}

static void gfx_create_triangle_strip_pipeline_layout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &s_triangleStrip.descriptorSetLayout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
    };
    const VkResult pipelineLayoutRes = vkCreatePipelineLayout(g_vulkanBackend.device, &pipelineLayoutCreateInfo, nullptr, &s_triangleStrip.pipelineLayout);
    ASSERT(pipelineLayoutRes == VK_SUCCESS);
}

static bool gfx_create_triangle_strip_pipelines(VkPipeline &outLinePipeline) {
    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = gfx_pipeline_input_assembly_create(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE);
    const VkPipelineRasterizationStateCreateInfo rasterizationState = gfx_pipeline_rasterization_create(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    VkPipelineColorBlendAttachmentState blendAttachmentState = gfx_pipeline_color_blend_attachment_state(0xf, VK_FALSE);
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    const VkPipelineColorBlendStateCreateInfo colorBlendState = gfx_pipeline_color_blend_state_create(1, &blendAttachmentState);
    // TODO: this means we don't sort the gizmo based on depth (i.e. order of rendering matters) will consider rendering this to a different buffer in the future
    const VkPipelineDepthStencilStateCreateInfo depthStencilState = gfx_pipeline_depth_stencil_state_create(VK_TRUE, VK_TRUE, VK_COMPARE_OP_ALWAYS);
    const VkPipelineViewportStateCreateInfo viewportState = gfx_pipeline_viewport_state_create(1, 1, 0);
    const VkPipelineMultisampleStateCreateInfo multisampleState = gfx_pipeline_multisample_state_create(g_vulkanBackend.sampleCount, 0);

    constexpr uint32_t dynamicStateCount = 3;
    const VkDynamicState dynamicStateEnables[dynamicStateCount] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = gfx_pipeline_dynamic_state_create(dynamicStateEnables, dynamicStateCount, 0);
    constexpr uint32_t shaderStagesCount = 2;
    VkPipelineShaderStageCreateInfo shaderStages[shaderStagesCount] = {};

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gfx_graphics_pipeline_create(); // using VK_KHR_dynamic_rendering we can skip passing a render pass
    pipelineCreateInfo.layout = s_triangleStrip.pipelineLayout;
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

    VkPipelineVertexInputStateCreateInfo inputState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };
    pipelineCreateInfo.pVertexInputState = &inputState;

    shaderStages[0] = gfx_load_shader("assets/shaders/line/line.vert", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gfx_load_shader("assets/shaders/line/line.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
    const VkResult pipelineRes = (vkCreateGraphicsPipelines(g_vulkanBackend.device, g_vulkanBackend.pipelineCache, 1, &pipelineCreateInfo, nullptr, &outLinePipeline));
    ASSERT_MSG(pipelineRes == VK_SUCCESS, "Err: failed to create graphics pipeline");
    vkDestroyShaderModule(g_vulkanBackend.device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(g_vulkanBackend.device, shaderStages[1].module, nullptr);
    return (pipelineRes == VK_SUCCESS);
}
//======================================================================================================================

//===API================================================================================================================
void gfx_triangle_strip_draw(VkCommandBuffer &cmdBuffer) {
    gfx_triangle_strip_update_material_descriptor(s_triangleStrip.descriptorSets[gfx_buffer_index()]);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_triangleStrip.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_triangleStrip.pipelineLayout, 0, 1, &s_triangleStrip.descriptorSets[gfx_buffer_index()], 0, nullptr);
    for (uint32_t i = 0; i < s_triangleStripEntityCount; ++i) {
        const TriangleStripEntity &lineEntity = s_triangleStripEntityPool[i];
        const uint32_t numberOfPoints = (lineEntity.triangleStripRangeEnd - lineEntity.triangleStripRangeStart);
        vkCmdDraw(cmdBuffer, numberOfPoints, 1, lineEntity.triangleStripRangeStart, 0);
    }
    s_pointCount = {0};
    s_triangleStripEntityCount = {0};
}

void gfx_triangle_strip_update_material_descriptor(VkDescriptorSet &outDescriptorSet) {

    const VkDescriptorImageInfo depthImageInfo = {
            .sampler = gfx_samplers()->samplers[TextureSamplerType::DepthStencil],
            .imageView = g_vulkanBackend.resolvedDepthBuffer.view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
    };

    constexpr uint32_t descriptorSetSize = 3;
    const VkWriteDescriptorSet writeDescriptorSets[descriptorSetSize] = {
            gfx_descriptor_set_write(outDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &g_vulkanBackend.uniformBuffer.descriptor, 1),
            gfx_descriptor_set_write(outDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &s_triangleStrip.triangleStripUniformBuffers[gfx_buffer_index()].descriptor, 1),
            gfx_descriptor_set_write(outDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &depthImageInfo, 1),
    };
    vkUpdateDescriptorSets(g_vulkanBackend.device, descriptorSetSize, &writeDescriptorSets[0], 0, nullptr);
}

bool gfx_rebuild_triangle_strip_pipeline() {
    VkPipeline newPipeline = {};
    if (gfx_create_triangle_strip_pipelines(newPipeline)) {
        vkDeviceWaitIdle(g_vulkanBackend.device);
        vkDestroyPipeline(g_vulkanBackend.device, s_triangleStrip.pipeline, nullptr);
        s_triangleStrip.pipeline = newPipeline;
        return true;
    }
    return false;
}

void gfx_triangle_strip_add_segment_immediate(const std::vector<LinePoint3D> &points) {
    // Ensure there is enough space in the entity pool
    assert(s_triangleStripEntityCount < MAX_TRIANGLE_STRIP_ENTITY_SIZE);

    // Ensure there is enough space for the points
    assert(s_pointCount + points.size() < MAX_POINT_SIZE);

    // Add the triangle strip segment to the entity pool
    s_triangleStripEntityPool[s_triangleStripEntityCount] = {
            .triangleStripRangeStart = s_pointCount,
            .triangleStripRangeEnd = s_pointCount + static_cast<uint32_t>(points.size()),
    };

    // Add each point to the buffer
    for (const auto &point: points) {
        add_point(point);
    }

    s_triangleStripEntityCount += 1;
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_triangle_strip() {
    gfx_create_triangle_strip_uniform_buffers();
    gfx_create_triangle_strip_descriptor_set_layout();
    gfx_create_triangle_strip_pipeline_layout();
    const bool pipelineResult = gfx_create_triangle_strip_pipelines(s_triangleStrip.pipeline);
    ASSERT(pipelineResult);
    gfx_create_triangle_strip_material_descriptor();
    for (int i = 0; i < BEET_BUFFER_COUNT; ++i) {
        gfx_triangle_strip_update_material_descriptor(s_triangleStrip.descriptorSets[i]);
    }
}

void gfx_cleanup_triangle_strip() {
    gfx_cleanup_triangle_strip_uniform_buffers();
    vkDestroyDescriptorSetLayout(g_vulkanBackend.device, s_triangleStrip.descriptorSetLayout, nullptr);
    for (uint32_t i = 0; i < BEET_BUFFER_COUNT; ++i) {
        vkDestroyDescriptorPool(g_vulkanBackend.device, s_triangleStrip.descriptorPools[i], nullptr);
    }
    vkDestroyPipeline(g_vulkanBackend.device, s_triangleStrip.pipeline, nullptr);
    vkDestroyPipelineLayout(g_vulkanBackend.device, s_triangleStrip.pipelineLayout, nullptr);
}
//======================================================================================================================