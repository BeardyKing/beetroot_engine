#include <beet_gfx/gfx_texture.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_samplers.h>
#include <beet_gfx/gfx_utils.h>
#include <beet_gfx/gfx_command.h>

#include <beet_shared/texture_formats.h>
#include <beet_shared/dds_loader.h>
#include <beet_shared/assert.h>
#include <beet_shared/memory.h>

#include <vulkan/vulkan_core.h>

extern VulkanBackend g_vulkanBackend;

void set_image_layout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask) {
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // Image layout is undefined (or does not matter)
            // Only valid as initial layout
            // No flags required, listed only for completeness
            imageMemoryBarrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            // Image is preinitialized
            // Only valid as initial layout for linear images, preserves memory contents
            // Make sure host writes have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image is a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image is a depth/stencil attachment
            // Make sure any writes to the depth/stencil buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image is a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image is a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image is read by a shader
            // Make sure any shader reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used as a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used as a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image layout will be used as a depth/stencil attachment
            // Make sure any writes to depth/stencil buffer have been finished
            imageMemoryBarrier.dstAccessMask =
                    imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read in a shader (sampler, input attachment)
            // Make sure any writes to the image have been finished
            if (imageMemoryBarrier.srcAccessMask == 0) {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
            cmdbuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
}

void gfx_texture_create_immediate_dds(const char *path, GfxTexture &inOutTexture) {
    if(inOutTexture.imageSamplerType == TextureSamplerType::Invalid){
        inOutTexture.imageSamplerType = TextureSamplerType::LinearRepeat;
    }

    RawImage myImage{};
    load_dds_image(path, &myImage);
    auto rawImageData = (unsigned char *) myImage.data;

#if BEET_DEBUG
    sprintf(inOutTexture.debug_name, "%s", path);
    inOutTexture.debug_textureFormat = myImage.textureFormat;
    inOutTexture.debug_mipMapCount = myImage.mipMapCount;
    inOutTexture.debug_width = myImage.width;
    inOutTexture.debug_height = myImage.height;
    inOutTexture.debug_depth = myImage.depth;
#endif

    const uint32_t sizeX = myImage.width;
    const uint32_t sizeY = myImage.height;
    const uint32 mipMapCount = myImage.mipMapCount;
    const VkDeviceSize imageSize = myImage.dataSize;
    const VkFormat format = gfx_utils_beet_image_format_to_vk(myImage.textureFormat);

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(g_vulkanBackend.physicalDevice, format, &formatProperties);

    VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    VkMemoryRequirements memoryRequirements;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkBufferCreateInfo stagingBufInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    stagingBufInfo.size = imageSize;
    stagingBufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    const VkResult createStageBuffRes = vkCreateBuffer(g_vulkanBackend.device, &stagingBufInfo, nullptr, &stagingBuffer);
    ASSERT(createStageBuffRes == VK_SUCCESS);

    vkGetBufferMemoryRequirements(g_vulkanBackend.device, stagingBuffer, &memoryRequirements);
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = gfx_utils_get_memory_type(memoryRequirements.memoryTypeBits,
                                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    const VkResult memAllocResult = vkAllocateMemory(g_vulkanBackend.device, &memoryAllocateInfo, nullptr, &stagingMemory);
    ASSERT(memAllocResult == VK_SUCCESS);

    const VkResult memBindResult = vkBindBufferMemory(g_vulkanBackend.device, stagingBuffer, stagingMemory, 0);
    ASSERT(memBindResult == VK_SUCCESS);

    // map raw image data to staging buffer
    uint8_t *data;
    const VkResult mapResult(vkMapMemory(g_vulkanBackend.device, stagingMemory, 0, memoryRequirements.size, 0, (void **) &data));
    ASSERT(mapResult == VK_SUCCESS)

    memcpy(data, rawImageData, imageSize);
    vkUnmapMemory(g_vulkanBackend.device, stagingMemory);

    VkBufferImageCopy *bufferCopyRegions = (VkBufferImageCopy *) mem_zalloc(mipMapCount * sizeof(VkBufferImageCopy));
    uint32_t offset = 0;
    for (uint32_t i = 0; i < mipMapCount; i++) {
        // set up a buffer image copy structure for the current mip level
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = i;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = myImage.width >> i;
        bufferCopyRegion.imageExtent.height = myImage.height >> i;
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.bufferOffset = offset;
        bufferCopyRegions[i] = bufferCopyRegion;
        offset += myImage.mipDataSizes[i];
    }

    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = sizeX;
    imageCreateInfo.extent.height = sizeY;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = mipMapCount;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.flags = 0;

    const VkResult createImageRes = vkCreateImage(g_vulkanBackend.device, &imageCreateInfo, nullptr, &inOutTexture.image);
    ASSERT(createImageRes == VK_SUCCESS);

    vkGetImageMemoryRequirements(g_vulkanBackend.device, inOutTexture.image, &memoryRequirements);

    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = gfx_utils_get_memory_type(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    const VkResult memAllocRes = vkAllocateMemory(g_vulkanBackend.device, &memoryAllocateInfo, nullptr, &inOutTexture.deviceMemory);
    ASSERT(memAllocRes == VK_SUCCESS);

    const VkResult bindRes = vkBindImageMemory(g_vulkanBackend.device, inOutTexture.image, inOutTexture.deviceMemory, 0);
    ASSERT(bindRes == VK_SUCCESS);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipMapCount;
    subresourceRange.layerCount = 1;

    gfx_command_begin_immediate_recording();

    set_image_layout(
            g_vulkanBackend.immediateCommandBuffer,
            inOutTexture.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            subresourceRange,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    );

    // Copy mips from staging buffer
    vkCmdCopyBufferToImage(
            g_vulkanBackend.immediateCommandBuffer,
            stagingBuffer,
            inOutTexture.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(mipMapCount),
            bufferCopyRegions
    );

    set_image_layout(
            g_vulkanBackend.immediateCommandBuffer,
            inOutTexture.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            subresourceRange,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    );

    inOutTexture.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    gfx_command_end_immediate_recording();

    vkDestroyBuffer(g_vulkanBackend.device, stagingBuffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, stagingMemory, nullptr);

    VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = gfx_utils_beet_image_format_to_vk(myImage.textureFormat);;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    view.subresourceRange.levelCount = mipMapCount;
    view.image = inOutTexture.image;

    const VkResult imageViewRes = vkCreateImageView(g_vulkanBackend.device, &view, nullptr, &inOutTexture.view);
    ASSERT(imageViewRes == VK_SUCCESS);

    inOutTexture.descriptor.imageView = inOutTexture.view;
    inOutTexture.descriptor.sampler = gfx_samplers()->samplers[inOutTexture.imageSamplerType];
    inOutTexture.descriptor.imageLayout = inOutTexture.layout;
}

void gfx_texture_cleanup(GfxTexture &gfxTexture) {
    vkDestroyImageView(g_vulkanBackend.device, gfxTexture.view, nullptr);
    vkDestroyImage(g_vulkanBackend.device, gfxTexture.image, nullptr);
    vkFreeMemory(g_vulkanBackend.device, gfxTexture.deviceMemory, nullptr);
    gfxTexture = {};

    //TODO:GFX We don't re-add this as a free slot in the texture pool i.e.
    //we could address this pretty simply in a few ways.
    //create free-list for each pool and next time we try and create a new texture to check if any free list spaces are free
    //we move the last image loaded into the newly free position and fix up and dependency, this will break any cached references.
}