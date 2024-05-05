#include <beet_gfx/gfx_line.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_descriptors.h>
#include <beet_gfx/gfx_shader.h>
#include <beet_gfx/gfx_pipeline.h>
#include <beet_shared/assert.h>
#include <beet_math/quat.h>

#include <vulkan/vulkan_core.h>

//===INTERNAL_STRUCTS===================================================================================================
static struct VulkanLine {
    VkDescriptorSetLayout descriptorSetLayout = {VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool = {VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout = {VK_NULL_HANDLE};
    VkPipeline pipeline = {VK_NULL_HANDLE};

    VkDescriptorSet descriptorSet;

    GfxBuffer lineUniformBuffer = {VK_NULL_HANDLE};
} s_gfxLine;

#define MAX_LINE_ENTITY_SIZE (1024 * 1)
struct LineEntity {
    uint32_t lineRangeStart = {0};
    uint32_t lineRangeEnd = {0};
    float lineWidth = {1.0f};
};
static LineEntity s_lineEntityPool[MAX_LINE_ENTITY_SIZE] = {};
static uint32_t s_lineEntityCount = 0;

#define MAX_POINT_SIZE (1024 * 4)
LinePoint3D s_pointPool[MAX_POINT_SIZE] = {};
static uint32_t s_pointCount = 0;

extern VulkanBackend g_vulkanBackend;
extern TargetVulkanFormats g_vulkanTargetFormats;
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static uint32_t add_point(const LinePoint3D &point) {
    s_pointPool[s_pointCount] = point;
    s_pointCount++;
    return s_pointCount;
}

static void gfx_update_lines_uniform_buffers() {
    memcpy(s_gfxLine.lineUniformBuffer.mappedData, &s_pointPool, sizeof(LinePoint3D) * s_pointCount);
}

static void gfx_create_lines_uniform_buffers() {
    const VkResult uniformResult = gfx_buffer_create(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            s_gfxLine.lineUniformBuffer,
            (sizeof(LinePoint3D) * MAX_POINT_SIZE),
            nullptr
    );
    ASSERT(uniformResult == VK_SUCCESS);

    const VkResult mapResult = vkMapMemory(g_vulkanBackend.device, s_gfxLine.lineUniformBuffer.memory, 0, VK_WHOLE_SIZE, 0, &s_gfxLine.lineUniformBuffer.mappedData);
    ASSERT(mapResult == VK_SUCCESS);
}

static void gfx_cleanup_lines_uniform_buffers() {
    vkDestroyBuffer(g_vulkanBackend.device, s_gfxLine.lineUniformBuffer.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, s_gfxLine.lineUniformBuffer.memory, nullptr);
}

static void gfx_create_line_descriptor_set_layout() {
    constexpr uint32_t poolSizeCount = 2;
    VkDescriptorPoolSize poolSizes[poolSizeCount] = {
            VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1},
            VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1},
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 2,
            .poolSizeCount = poolSizeCount,
            .pPoolSizes = &poolSizes[0],
    };
    const VkResult createPoolRes = (vkCreateDescriptorPool(g_vulkanBackend.device, &descriptorPoolInfo, nullptr, &s_gfxLine.descriptorPool));
    ASSERT(createPoolRes == VK_SUCCESS);

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
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            }},
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = layoutBindingsCount,
            .pBindings = &layoutBindings[0],
    };
    const VkResult descriptorResult = vkCreateDescriptorSetLayout(g_vulkanBackend.device, &descriptorSetLayoutCreateInfo, nullptr, &s_gfxLine.descriptorSetLayout);
    ASSERT(descriptorResult == VK_SUCCESS);
}

static void gfx_create_line_pipeline_layout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &s_gfxLine.descriptorSetLayout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
    };
    const VkResult pipelineLayoutRes = vkCreatePipelineLayout(g_vulkanBackend.device, &pipelineLayoutCreateInfo, nullptr, &s_gfxLine.pipelineLayout);
    ASSERT(pipelineLayoutRes == VK_SUCCESS);
}

static bool gfx_create_line_pipelines(VkPipeline &outLinePipeline) {
    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = gfx_pipeline_input_assembly_create(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 0, VK_FALSE);
    const VkPipelineRasterizationStateCreateInfo rasterizationState = gfx_pipeline_rasterization_create(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    const VkPipelineColorBlendAttachmentState blendAttachmentState = gfx_pipeline_color_blend_attachment_state(0xf, VK_FALSE);
    const VkPipelineColorBlendStateCreateInfo colorBlendState = gfx_pipeline_color_blend_state_create(1, &blendAttachmentState);
    // TODO: this means we don't sort the gizmo based on depth (i.e. order of rendering matters) will consider rendering this to a different buffer in the future
    const VkPipelineDepthStencilStateCreateInfo depthStencilState = gfx_pipeline_depth_stencil_state_create(VK_TRUE, VK_TRUE, VK_COMPARE_OP_ALWAYS);
    const VkPipelineViewportStateCreateInfo viewportState = gfx_pipeline_viewport_state_create(1, 1, 0);
    const VkPipelineMultisampleStateCreateInfo multisampleState = gfx_pipeline_multisample_state_create(VK_SAMPLE_COUNT_1_BIT, 0);

    constexpr uint32_t dynamicStateCount = 3;
    const VkDynamicState dynamicStateEnables[dynamicStateCount] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH};
    VkPipelineDynamicStateCreateInfo dynamicState = gfx_pipeline_dynamic_state_create(dynamicStateEnables, dynamicStateCount, 0);

    constexpr uint32_t shaderStagesCount = 2;
    VkPipelineShaderStageCreateInfo shaderStages[shaderStagesCount] = {};

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gfx_graphics_pipeline_create(); // using VK_KHR_dynamic_rendering we can skip passing a render pass
    pipelineCreateInfo.layout = s_gfxLine.pipelineLayout;
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
extern PFN_vkCmdSetLineRasterizationModeEXT g_vkCmdSetLineRasterizationModeEXT_Func;
//===API================================================================================================================
void gfx_line_draw(VkCommandBuffer &cmdBuffer) {
    gfx_update_lines_uniform_buffers();
    // There is only 1 descriptor for lines, so we cache it here instead of adding it to the db_pool
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_gfxLine.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_gfxLine.pipelineLayout, 0, 1, &s_gfxLine.descriptorSet, 0, nullptr);
//    g_vkCmdSetLineRasterizationModeEXT_Func(cmdBuffer, static_cast<VkLineRasterizationModeEXT>(3)); //FIXME: This crashes
    for (uint32_t i = 0; i < s_lineEntityCount; ++i) {
        const LineEntity &lineEntity = s_lineEntityPool[i];
        const uint32_t numberOfPoints = (lineEntity.lineRangeEnd - lineEntity.lineRangeStart);
        vkCmdSetLineWidth(cmdBuffer, lineEntity.lineWidth);
        vkCmdDraw(cmdBuffer, numberOfPoints, 1, lineEntity.lineRangeStart, 0);
    }

    //clear existing counts, we only want to copy the data that was added this frame.
    s_pointCount = {0};
    s_lineEntityCount = {0};
}

void gfx_line_update_material_descriptor(VkDescriptorSet &outDescriptorSet) {
    VkDescriptorSetAllocateInfo allocInfo = gfx_descriptor_set_alloc_info(s_gfxLine.descriptorPool, &s_gfxLine.descriptorSetLayout, 1);
    const VkResult allocDescRes = vkAllocateDescriptorSets(g_vulkanBackend.device, &allocInfo, &outDescriptorSet);
    ASSERT(allocDescRes == VK_SUCCESS);
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            gfx_descriptor_set_write(outDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &g_vulkanBackend.uniformBuffer.descriptor, 1),
            gfx_descriptor_set_write(outDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &s_gfxLine.lineUniformBuffer.descriptor, 1),
    };
    vkUpdateDescriptorSets(g_vulkanBackend.device, uint32_t(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

bool gfx_rebuild_line_pipeline() {
    VkPipeline newPipeline = {};
    if (gfx_create_line_pipelines(newPipeline)) {
        vkDeviceWaitIdle(g_vulkanBackend.device);
        vkDestroyPipeline(g_vulkanBackend.device, s_gfxLine.pipeline, nullptr);
        s_gfxLine.pipeline = newPipeline;
        return true;
    }
    return false;
}

void gfx_line_add_segment_immediate(const LinePoint3D &start, const LinePoint3D &end, const float lineWidth) {
    s_lineEntityPool[s_lineEntityCount] = {
            .lineRangeStart = s_pointCount,
            .lineRangeEnd = s_pointCount + 2,
            .lineWidth = lineWidth
    };
    add_point(start);
    add_point(end);
    s_lineEntityCount += 1;
}

//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_line() {
    gfx_create_lines_uniform_buffers();
    gfx_create_line_descriptor_set_layout();
    gfx_create_line_pipeline_layout();
    const bool pipelineResult = gfx_create_line_pipelines(s_gfxLine.pipeline);
    ASSERT(pipelineResult);
    gfx_line_update_material_descriptor(s_gfxLine.descriptorSet);
    gfx_line_update_material_descriptor(s_gfxLine.descriptorSet);
}

void gfx_cleanup_line() {
    gfx_cleanup_lines_uniform_buffers();
    vkDestroyDescriptorSetLayout(g_vulkanBackend.device, s_gfxLine.descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(g_vulkanBackend.device, s_gfxLine.descriptorPool, nullptr);
    vkDestroyPipeline(g_vulkanBackend.device, s_gfxLine.pipeline, nullptr);
    vkDestroyPipelineLayout(g_vulkanBackend.device, s_gfxLine.pipelineLayout, nullptr);
}
//======================================================================================================================