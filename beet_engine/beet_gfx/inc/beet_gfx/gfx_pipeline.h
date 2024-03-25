#ifndef BEETROOT_GFX_PIPELINE_H
#define BEETROOT_GFX_PIPELINE_H

#include <vulkan/vulkan.h>

VkPipelineInputAssemblyStateCreateInfo gfx_pipeline_input_assembly_create(VkPrimitiveTopology topology,
                                                                          VkPipelineInputAssemblyStateCreateFlags flags,
                                                                          VkBool32 primitiveRestartEnable);

VkPipelineRasterizationStateCreateInfo gfx_pipeline_rasterization_create(VkPolygonMode polygonMode,
                                                                         VkCullModeFlags cullMode,
                                                                         VkFrontFace frontFace,
                                                                         VkPipelineRasterizationStateCreateFlags flags = 0);

VkPipelineColorBlendAttachmentState gfx_pipeline_color_blend_attachment_state(VkColorComponentFlags colorWriteMask,
                                                                              VkBool32 blendEnable);

VkPipelineColorBlendStateCreateInfo gfx_pipeline_color_blend_state_create(uint32_t attachmentCount,
                                                                          const VkPipelineColorBlendAttachmentState *pAttachments);

VkPipelineDepthStencilStateCreateInfo gfx_pipeline_depth_stencil_state_create(VkBool32 depthTestEnable,
                                                                              VkBool32 depthWriteEnable,
                                                                              VkCompareOp depthCompareOp);

VkPipelineViewportStateCreateInfo gfx_pipeline_viewport_state_create(uint32_t viewportCount,
                                                                     uint32_t scissorCount,
                                                                     VkPipelineViewportStateCreateFlags);

VkPipelineMultisampleStateCreateInfo gfx_pipeline_multisample_state_create(VkSampleCountFlagBits rasterizationSamples,
                                                                           VkPipelineMultisampleStateCreateFlags flags);

VkPipelineDynamicStateCreateInfo gfx_pipeline_dynamic_state_create(const VkDynamicState *pDynamicStates,
                                                                   uint32_t dynamicStateCount,
                                                                   VkPipelineDynamicStateCreateFlags flags);

VkGraphicsPipelineCreateInfo gfx_graphics_pipeline_create();

VkGraphicsPipelineCreateInfo gfx_graphics_pipeline_create(VkPipelineLayout layout,
                                                          VkRenderPass renderPass,
                                                          VkPipelineCreateFlags flags);

VkPipelineVertexInputStateCreateInfo gfx_pipeline_vertex_input_state_create();

VkVertexInputBindingDescription gfx_vertex_input_binding_desc(uint32_t binding,
                                                              uint32_t stride,
                                                              VkVertexInputRate inputRate);

VkVertexInputAttributeDescription gfx_vertex_input_attribute_desc(uint32_t binding,
                                                                  uint32_t location,
                                                                  VkFormat format,
                                                                  uint32_t offset);

#endif //BEETROOT_GFX_PIPELINE_H
