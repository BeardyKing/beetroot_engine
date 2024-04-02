#include <beet_gfx/gfx_indexed_indirect.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_pipeline.h>
#include <beet_gfx/gfx_mesh.h>
#include <beet_gfx/gfx_shader.h>
#include <beet_gfx/gfx_descriptors.h>
#include <beet_gfx/gfx_command.h>

#include <beet_shared/assert.h>

constexpr uint32_t BEET_INSTANCE_BUFFER_BIND_ID = 1;

extern VulkanBackend g_vulkanBackend;
extern GlobMeshes g_meshes;
extern GlobTextures g_textures;

struct GfxIndexedIndirect{
    VkDescriptorSetLayout descriptorSetLayout = {VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool = {VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout = {VK_NULL_HANDLE};
    VkPipeline pipeline = {VK_NULL_HANDLE};

    uint32 usedDescriptorCount = 0;
    static constexpr uint32_t MAX_DESCRIPTOR_COUNT = 1;
    VkDescriptorSet descriptorSets[MAX_DESCRIPTOR_COUNT] = {};
} g_gfxIndirect;

void gfx_indexed_indirect_render(VkCommandBuffer &cmdBuffer) {
    VkDeviceSize offsets[1] = {0};
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_gfxIndirect.pipelineLayout, 0, 1, &g_gfxIndirect.descriptorSets[0], 0, NULL);

    //cubePipeline
    // [POI] Instanced multi draw rendering of the cubes
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_gfxIndirect.pipeline);
    // Binding point 0 : Mesh vertex buffer
    // Binding point 1 : Instance data buffer
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &g_meshes.cube.vertBuffer, offsets);
    vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &g_vulkanBackend.instanceBuffer.buffer, offsets);

    vkCmdBindIndexBuffer(cmdBuffer, g_meshes.cube.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexedIndirect(cmdBuffer, g_vulkanBackend.indirectCommandsBuffer.buffer, 0, g_vulkanBackend.indirectDrawCount, sizeof(VkDrawIndexedIndirectCommand));
}

void gfx_build_indexed_indirect_pipelines() {

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &g_gfxIndirect.descriptorSetLayout
    };
    const VkResult pipelineLayoutRes = vkCreatePipelineLayout(g_vulkanBackend.device, &pipelineLayoutCreateInfo, nullptr, &g_gfxIndirect.pipelineLayout);
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

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gfx_graphics_pipeline_create(); // using VK_KHR_dynamic_rendering we can skip passing a render pass
    pipelineCreateInfo.layout = g_gfxIndirect.pipelineLayout;
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
            gfx_vertex_input_attribute_desc(0, 3, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GfxVertex, color)),                               // 3: Color

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
    const VkResult pipelineRes = (vkCreateGraphicsPipelines(g_vulkanBackend.device, g_vulkanBackend.pipelineCache, 1, &pipelineCreateInfo, nullptr, &g_gfxIndirect.pipeline));
    ASSERT_MSG(pipelineRes == VK_SUCCESS, "Err: failed to create graphics pipeline");
    vkDestroyShaderModule(g_vulkanBackend.device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(g_vulkanBackend.device, shaderStages[1].module, nullptr);
}

void gfx_cleanup_indexed_indirect_pipelines() {
    vkDestroyPipeline(g_vulkanBackend.device, g_gfxIndirect.pipeline, nullptr);
}


#define OBJECT_INSTANCE_COUNT 1


void gfx_build_indexed_indirect_commands() {
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

void gfx_cleanup_indexed_indirect_commands() {
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

void gfx_build_indexed_indirect_instance_data() {
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

void gfx_build_indexed_indirect_descriptor_set_layout() {
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
    const VkResult createPoolRes = (vkCreateDescriptorPool(g_vulkanBackend.device, &descriptorPoolInfo, nullptr, &g_gfxIndirect.descriptorPool));
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
    const VkResult descriptorResult = vkCreateDescriptorSetLayout(g_vulkanBackend.device, &descriptorSetLayoutCreateInfo, nullptr, &g_gfxIndirect.descriptorSetLayout);
    ASSERT(descriptorResult == VK_SUCCESS);

    //=== SET ======//
    VkDescriptorSetAllocateInfo allocInfo = gfx_descriptor_set_alloc_info(g_gfxIndirect.descriptorPool, &g_gfxIndirect.descriptorSetLayout, 1);
    const VkResult allocDescRes = vkAllocateDescriptorSets(g_vulkanBackend.device, &allocInfo, &g_gfxIndirect.descriptorSets[0]);
    ASSERT(allocDescRes == VK_SUCCESS);
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            // Binding 0: Vertex shader uniform buffer
            // Binding 1: uv grid image // TODO: Add a per package loaded texture array
            gfx_descriptor_set_write(g_gfxIndirect.descriptorSets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &g_vulkanBackend.uniformBuffer.descriptor, 1),
            gfx_descriptor_set_write(g_gfxIndirect.descriptorSets[0], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &g_textures.uvGrid.descriptor, 1)
    };
    vkUpdateDescriptorSets(g_vulkanBackend.device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void gfx_cleanup_indexed_indirect_descriptor_set_layout() {
    vkDestroyPipelineLayout(g_vulkanBackend.device, g_gfxIndirect.pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(g_vulkanBackend.device, g_gfxIndirect.descriptorSetLayout, nullptr);
}