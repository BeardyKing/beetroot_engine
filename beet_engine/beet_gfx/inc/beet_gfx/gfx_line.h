#ifndef BEETROOT_GFX_LINE_H
#define BEETROOT_GFX_LINE_H

#include <beet_gfx/gfx_types.h>
#include <vulkan/vulkan_core.h>

//===MUST_MIRROR_line/line.vert=========================================================================================
struct LinePoint3D {
    vec3f position;
    uint32_t color;
};
//======================================================================================================================

//===API================================================================================================================
void gfx_line_add_segment_immediate(const LinePoint3D &start, const LinePoint3D &end, const float lineWidth = 1.0f);

bool gfx_rebuild_line_pipeline();
void gfx_line_draw(VkCommandBuffer &cmdBuffer);
void gfx_line_update_material_descriptor(VkDescriptorSet &outDescriptorSet);
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_line();
void gfx_cleanup_line();
//======================================================================================================================

#endif //BEETROOT_GFX_LINE_H
