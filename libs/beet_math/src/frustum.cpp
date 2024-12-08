#include <beet_math/frustum.h>
#include <glm/geometric.hpp>

//===API================================================================================================================
Frustum view_frustum_to_frustum(const GfxViewFrustum &viewFrustum) {
    const vec3f forward = glm::normalize(viewFrustum.normal);
    vec3f up = glm::normalize(viewFrustum.up);
    const vec3f right = glm::normalize(glm::cross(forward, up));
    up = glm::normalize(glm::cross(right, forward));

    const vec3f nearCenter = viewFrustum.origin + forward * viewFrustum.zNear;
    const vec3f farCenter = viewFrustum.origin + forward * viewFrustum.zFar;

    const vec3f nearUpOffset = up * (viewFrustum.nearSize.y * 0.5f);
    const vec3f nearRightOffset = right * (viewFrustum.nearSize.x * 0.5f);
    const vec3f farUpOffset = up * (viewFrustum.farSize.y * 0.5f);
    const vec3f farRightOffset = right * (viewFrustum.farSize.x * 0.5f);

    const vec3f nearTopLeft = nearCenter + nearUpOffset - nearRightOffset;
    const vec3f nearTopRight = nearCenter + nearUpOffset + nearRightOffset;
    const vec3f nearBottomLeft = nearCenter - nearUpOffset - nearRightOffset;
    const vec3f nearBottomRight = nearCenter - nearUpOffset + nearRightOffset;

    const vec3f farTopLeft = farCenter + farUpOffset - farRightOffset;
    const vec3f farTopRight = farCenter + farUpOffset + farRightOffset;
    const vec3f farBottomLeft = farCenter - farUpOffset - farRightOffset;
    const vec3f farBottomRight = farCenter - farUpOffset + farRightOffset;

    return (Frustum) {
            .nearTopLeft = nearTopLeft,
            .nearTopRight = nearTopRight,
            .nearBottomLeft = nearBottomLeft,
            .nearBottomRight = nearBottomRight,
            .farTopLeft = farTopLeft,
            .farTopRight = farTopRight,
            .farBottomLeft = farBottomLeft,
            .farBottomRight = farBottomRight
    };
}
//======================================================================================================================