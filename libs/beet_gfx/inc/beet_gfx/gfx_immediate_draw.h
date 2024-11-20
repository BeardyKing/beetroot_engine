#ifndef BEETROOT_GFX_IMMEDIATE_DRAW_H
#define BEETROOT_GFX_IMMEDIATE_DRAW_H

#include <beet_math/vec3.h>
#include <beet_math/vec2.h>
#include <beet_math/frustum.h>

struct GfxRect {
    vec3f center;
    vec2f halfExtents;
    vec3f normal;
    vec3f up;
};

void gfx_im_draw_poly_rect(const GfxRect &rect, uint32_t color);
void gfx_im_draw_line_rect(const GfxRect &rect, uint32_t color, float lineThickness = 1.0f);

struct GfxBox{
    vec3f center;
    vec3f halfExtents;
    vec3f normal;
    vec3f up;
};

void gfx_im_draw_poly_box(const GfxBox &box, uint32_t color);
void gfx_im_draw_line_box(const GfxBox &box, uint32_t color, float lineThickness = 1.0f);

struct GfxCircle {
    vec3f center;
    float radius;
    vec3f normal;
    vec3f up;
};

void gfx_im_draw_line_arc(const GfxCircle &arc, uint32_t color,
                          float arcPercent = 1.0f,
                          float startOffsetPercent = 0.0f,
                          uint32_t segments = 32,
                          float lineWidth = 2.0f);


void gfx_im_draw_line_arc_dial_marker(const GfxCircle &arc, uint32_t color,
                                      float arcPercent = 1.0f,
                                      float startOffsetPercent = 0.0f,
                                      float lineWidth = 2.0f,
                                      float sunRayLength = 0.03f,
                                      uint32_t numRays = 8,
                                      float raySpacing = 0.02f);

void gfx_im_draw_line_sun(const GfxCircle &arc, uint32_t color,
                          float arcPercent = 1.0f,
                          float startOffsetPercent = 0.0f,
                          uint32_t segments = 32,
                          float lineWidth = 2.0f,
                          float sunRayLength = 0.03f,
                          uint32_t numRays = 8,
                          float raySpacing = 0.02f);

void gfx_im_draw_line_frustum(const Frustum &frustum, uint32_t color, float lineWidth = 1.0f);
void gfx_im_draw_poly_frustum(const Frustum &frustum, uint32_t color);

void gfx_im_draw_line_view_frustum(const GfxViewFrustum &viewFrustum, uint32_t color, float lineWidth = 1.0f);
#endif //BEETROOT_GFX_IMMEDIATE_DRAW_H
