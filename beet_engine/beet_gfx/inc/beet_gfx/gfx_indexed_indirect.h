#ifndef BEETROOT_GFX_INDEXED_INDIRECT_H
#define BEETROOT_GFX_INDEXED_INDIRECT_H

#include <vulkan/vulkan_core.h>

void gfx_build_indexed_indirect_commands();
void gfx_build_indexed_indirect_instance_data();
void gfx_build_indexed_indirect_descriptor_set_layout();
void gfx_build_indexed_indirect_pipelines();

void gfx_indexed_indirect_render(VkCommandBuffer &cmdBuffer);

void gfx_cleanup_indexed_indirect_pipelines();
void gfx_cleanup_indexed_indirect_descriptor_set_layout();
void gfx_cleanup_indexed_indirect_commands();

#endif //BEETROOT_GFX_INDEXED_INDIRECT_H
