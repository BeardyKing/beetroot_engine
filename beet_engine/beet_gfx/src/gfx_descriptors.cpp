#include <beet_gfx/gfx_descriptors.h>


VkDescriptorSetAllocateInfo gfx_descriptor_set_alloc_info(VkDescriptorPool descriptorPool,
                                                          const VkDescriptorSetLayout *pSetLayouts,
                                                          uint32_t descriptorSetCount) {
    return VkDescriptorSetAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = descriptorSetCount,
            .pSetLayouts = pSetLayouts,
    };
}

VkWriteDescriptorSet gfx_descriptor_set_write(VkDescriptorSet dstSet,
                                              VkDescriptorType type,
                                              uint32_t binding,
                                              VkDescriptorBufferInfo *bufferInfo,
                                              uint32_t descriptorCount) {
    return VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = dstSet,
            .dstBinding = binding,
            .descriptorCount = descriptorCount,
            .descriptorType = type,
            .pBufferInfo = bufferInfo,

    };
}

VkWriteDescriptorSet gfx_descriptor_set_write(VkDescriptorSet dstSet,
                                              VkDescriptorType type,
                                              uint32_t binding,
                                              VkDescriptorImageInfo *imageInfo,
                                              uint32_t descriptorCount) {
    return VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = dstSet,
            .dstBinding = binding,
            .descriptorCount = descriptorCount,
            .descriptorType = type,
            .pImageInfo = imageInfo,
    };
}