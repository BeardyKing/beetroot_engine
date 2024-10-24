#ifndef BEETROOT_GFX_TRIANGLE_STRIP_H
#define BEETROOT_GFX_TRIANGLE_STRIP_H

#include <beet_gfx/gfx_types.h>
#include <vulkan/vulkan_core.h>

// TODO:    gah, I really need to write a scratch/arena allocator for copies like this I'm using std::vector for now as it's
//          quick to get working but I don't really want std::vector in my codebase, I just want to avoid lots of memory fragmentation
// CONSIDER:
//          - I could expose the mapped GPU pointer and allow the user to write to it directly
//          - Use Arena/Scratch allocator
//          - triangle strip API could have some hard limit on the point count

// I generally think this API should become some sort of draw/build primitive system, but for now it can only draw things as triangle strips
// which limits it a decent amount.

//===API================================================================================================================
void gfx_triangle_strip_add_segment_immediate(const std::vector<LinePoint3D> &points);

bool gfx_rebuild_triangle_strip_pipeline();
void gfx_triangle_strip_draw(VkCommandBuffer &cmdBuffer);
void gfx_triangle_strip_update_material_descriptor(VkDescriptorSet &outDescriptorSet);
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_triangle_strip();
void gfx_cleanup_triangle_strip();
//======================================================================================================================

#endif //BEETROOT_GFX_TRIANGLE_STRIP_H
