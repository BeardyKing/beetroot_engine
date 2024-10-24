#ifndef BEETROOT_GFX_GENERATE_GEOMETRY_H
#define BEETROOT_GFX_GENERATE_GEOMETRY_H

//TODO: replace C++ API a C compatible one, preferably something that takes in an allocator so it can use an arena allocator.

#include <cstdint>
#include <beet_gfx/gfx_types.h>

//===API================================================================================================================

std::vector<LinePoint3D> gfx_generate_geometry_cone(const vec3f &baseCenter, float radius, float height, uint32_t color, uint32_t segments);
std::vector<LinePoint3D> gfx_generate_geometry_thick_polyline(const std::vector<vec2f> &points, float lineWidth, uint32_t color, bool closedLoop = false);

//======================================================================================================================

#endif //BEETROOT_GFX_GENERATE_GEOMETRY_H
