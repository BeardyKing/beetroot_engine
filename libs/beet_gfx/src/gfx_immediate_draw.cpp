#include <beet_gfx/gfx_immediate_draw.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/vulkan/gfx_vk_triangle_strip.h>
#include <beet_gfx/vulkan/gfx_vk_line.h>

#include <beet_shared/assert.h>
#include "beet_math/transform.h"

void gfx_im_draw_poly_rect(const GfxRect &rect, uint32_t color) {
    ASSERT_MSG(rect.normal != rect.up, "Err: normal & rect can't match, if they do right will be NaN i.e. normalize(vec3f(0,0,0) == vec3f(NaN,NaN,NaN)");
    const vec3f right = glm::normalize(glm::cross(rect.normal, rect.up));
    const vec3f up = glm::normalize(glm::cross(right, rect.normal));

    constexpr uint32_t vertCount = 4;
    const LinePoint3D corners[vertCount] = {
            {{rect.center + right * rect.halfExtents.x + up * rect.halfExtents.y}, color},
            {{rect.center - right * rect.halfExtents.x + up * rect.halfExtents.y}, color},
            {{rect.center + right * rect.halfExtents.x - up * rect.halfExtents.y}, color},
            {{rect.center - right * rect.halfExtents.x - up * rect.halfExtents.y}, color},
    };

    gfx_triangle_strip_add_segment_immediate(&corners[0], vertCount);
}

void gfx_im_draw_line_rect(const GfxRect &rect, uint32_t color, float lineThickness) {
    ASSERT_MSG(rect.normal != rect.up, "Err: normal & rect can't match, if they do right will be NaN i.e. normalize(vec3f(0,0,0) == vec3f(NaN,NaN,NaN)");
    const vec3f right = glm::normalize(glm::cross(rect.normal, rect.up));
    const vec3f up = glm::normalize(glm::cross(right, rect.normal));

    constexpr uint32_t vertCount = 4;
    const vec3f corners[vertCount] = {
            {rect.center + right * rect.halfExtents.x + up * rect.halfExtents.y},
            {rect.center - right * rect.halfExtents.x + up * rect.halfExtents.y},
            {rect.center - right * rect.halfExtents.x - up * rect.halfExtents.y},
            {rect.center + right * rect.halfExtents.x - up * rect.halfExtents.y},
    };

    for (uint32_t i = 0; i < vertCount; ++i) {
        gfx_line_add_segment_immediate({corners[i], color}, {corners[(i + 1) % 4], color}, lineThickness);
    }
}

void gfx_im_draw_line_arc(const GfxCircle &arc,
                          uint32_t color,
                          const float arcPercent,
                          const float startOffsetPercent,
                          const uint32_t segments,
                          const float lineWidth) {

    const vec3f normal = glm::normalize(arc.normal);
    vec3f up = glm::normalize(arc.up);
    up = glm::normalize(up - normal * glm::dot(up, normal));
    vec3f tangent = glm::cross(normal, up);

    float totalSweep = glm::tau<float>() * arcPercent;
    float startOffset = glm::tau<float>() * startOffsetPercent;

    for (uint32_t i = 0; i < segments; ++i) {
        const float t0 = startOffset + (i / float(segments)) * totalSweep;
        const float t1 = startOffset + ((i + 1) / float(segments)) * totalSweep;
        const vec3f start = arc.center + arc.radius * (glm::cos(t0) * up + glm::sin(t0) * tangent);
        const vec3f end = arc.center + arc.radius * (glm::cos(t1) * up + glm::sin(t1) * tangent);

        gfx_line_add_segment_immediate({start, color}, {end, color}, lineWidth);
    }
}

void gfx_im_draw_line_arc_dial_marker(const GfxCircle &arc,
                                      uint32_t color,
                                      const float arcPercent,
                                      const float startOffsetPercent,
                                      const float lineWidth,
                                      const float sunRayLength,
                                      const uint32_t numRays,
                                      const float raySpacing) {

    const vec3f normal = glm::normalize(arc.normal);
    vec3f up = glm::normalize(arc.up);
    up = glm::normalize(up - normal * glm::dot(up, normal));
    const vec3f tangent = glm::cross(normal, up);

    const float totalSweep = glm::tau<float>() * arcPercent;
    const float startOffset = glm::tau<float>() * startOffsetPercent;

    for (uint32_t i = 0; i < numRays; ++i) {
        const float angle = startOffset + (i / float(numRays)) * totalSweep;
        const vec3f rayStart = arc.center + (arc.radius + raySpacing) * (glm::cos(angle) * up + glm::sin(angle) * tangent);
        const vec3f rayDirection = glm::normalize(rayStart - arc.center);
        const vec3f rayEnd = rayStart + sunRayLength * rayDirection;

        gfx_line_add_segment_immediate({rayStart, color}, {rayEnd, color}, lineWidth);
    }
}

void gfx_im_draw_line_sun(const GfxCircle &arc,
                          uint32_t color,
                          const float arcPercent,
                          const float startOffsetPercent,
                          const uint32_t segments,
                          const float lineWidth,
                          const float sunRayLength,
                          const uint32_t numRays,
                          const float raySpacing) {
    gfx_im_draw_line_arc(arc, color, arcPercent, startOffsetPercent, segments, lineWidth);
    gfx_im_draw_line_arc_dial_marker(arc, color, arcPercent, startOffsetPercent, lineWidth, sunRayLength, numRays, raySpacing);
}

void gfx_im_draw_view_frustum(const GfxViewFrustum &viewFrustum, uint32_t color, float lineWidth) {
    constexpr uint32_t whiteColour = 0xFFFFFF00;
    const vec3f zFarOrigin = vec3f{viewFrustum.origin + (viewFrustum.normal * viewFrustum.zFar)};

    const Frustum frustum = view_frustum_to_frustum(viewFrustum);
    gfx_line_add_segment_immediate({viewFrustum.origin, whiteColour}, {zFarOrigin, whiteColour}, lineWidth);

    gfx_line_add_segment_immediate({viewFrustum.origin, whiteColour}, {frustum.nearBottomLeft, whiteColour}, lineWidth);
    gfx_line_add_segment_immediate({viewFrustum.origin, whiteColour}, {frustum.nearBottomRight, whiteColour}, lineWidth);
    gfx_line_add_segment_immediate({viewFrustum.origin, whiteColour}, {frustum.nearTopLeft, whiteColour}, lineWidth);
    gfx_line_add_segment_immediate({viewFrustum.origin, whiteColour}, {frustum.nearTopRight, whiteColour}, lineWidth);

    gfx_im_draw_line_arc({zFarOrigin, min(viewFrustum.farSize.x * 0.5f, viewFrustum.farSize.y * 0.5f), viewFrustum.normal, viewFrustum.up}, color, 1, 0, 32, lineWidth);
    gfx_im_draw_frustum(frustum, color, lineWidth);
}


void gfx_im_draw_frustum(const Frustum &frustum, uint32_t color, float lineWidth) {
    gfx_line_add_segment_immediate({frustum.nearTopLeft, color}, {frustum.nearTopRight, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.nearTopRight, color}, {frustum.nearBottomRight, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.nearBottomRight, color}, {frustum.nearBottomLeft, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.nearBottomLeft, color}, {frustum.nearTopLeft, color}, lineWidth);

    gfx_line_add_segment_immediate({frustum.farTopLeft, color}, {frustum.farTopRight, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.farTopRight, color}, {frustum.farBottomRight, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.farBottomRight, color}, {frustum.farBottomLeft, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.farBottomLeft, color}, {frustum.farTopLeft, color}, lineWidth);

    gfx_line_add_segment_immediate({frustum.nearTopLeft, color}, {frustum.farTopLeft, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.nearTopRight, color}, {frustum.farTopRight, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.nearBottomLeft, color}, {frustum.farBottomLeft, color}, lineWidth);
    gfx_line_add_segment_immediate({frustum.nearBottomRight, color}, {frustum.farBottomRight, color}, lineWidth);
}

