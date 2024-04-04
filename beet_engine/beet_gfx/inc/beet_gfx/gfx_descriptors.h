#ifndef BEETROOT_GFX_DESCRIPTORS_H
#define BEETROOT_GFX_DESCRIPTORS_H

#include <vulkan/vulkan.h>

VkDescriptorSetAllocateInfo gfx_descriptor_set_alloc_info(VkDescriptorPool descriptorPool,
                                                          const VkDescriptorSetLayout *pSetLayouts,
                                                          uint32_t descriptorSetCount);

VkWriteDescriptorSet gfx_descriptor_set_write(VkDescriptorSet dstSet,
                                              VkDescriptorType type,
                                              uint32_t binding,
                                              VkDescriptorBufferInfo *bufferInfo,
                                              uint32_t descriptorCount);

VkWriteDescriptorSet gfx_descriptor_set_write(VkDescriptorSet dstSet,
                                              VkDescriptorType type,
                                              uint32_t binding,
                                              const VkDescriptorImageInfo *imageInfo,
                                              uint32_t descriptorCount);


#endif //BEETROOT_GFX_DESCRIPTORS_H
