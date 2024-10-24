#include <beet_gfx/gfx_generate_geometry.h>
#include <beet_shared/assert.h>

std::vector<LinePoint3D> gfx_generate_geometry_cone(const vec3f &baseCenter, const float radius, const float height, const uint32_t color, const uint32_t segments) {
    std::vector<LinePoint3D> vertices;

    // top
    vec3f topVertex = baseCenter + vec3f(0.0f, height, 0.0f);

    // base
    vec3f centerVertex = baseCenter;
    for (uint32_t i = 0; i <= segments; ++i) {
        const float angle = glm::two_pi<float>() * float(i) / float(segments);
        const float x = baseCenter.x + radius * glm::cos(angle);
        const float y = baseCenter.y;
        const float z = baseCenter.z + radius * glm::sin(angle);

        const vec3f baseVertex(x, y, z);
        vertices.push_back({centerVertex, color});
        vertices.push_back({baseVertex, color});
    }

    // side
    for (uint32_t i = 0; i <= segments; ++i) {
        const float angle = glm::two_pi<float>() * float(i) / float(segments);
        const float x = baseCenter.x + radius * glm::cos(angle);
        const float y = baseCenter.y;
        const float z = baseCenter.z + radius * glm::sin(angle);

        const vec3f baseVertex(x, y, z);
        vertices.push_back({baseVertex, color});
        vertices.push_back({topVertex, color});
    }

    return vertices;
}

std::vector<LinePoint3D> gfx_generate_geometry_thick_polyline(const std::vector<vec2f> &points, const float lineWidth, const uint32_t color, const bool closedLoop) {
    std::vector<LinePoint3D> outVertices;
    outVertices.reserve(points.size() * 2);

    ASSERT_MSG(points.size() > 2, "Err: Not enough points to create a thick polyline");

    if (points.size() < 2) {
        return {};
    }

    const vec2 firstDir = glm::normalize(points[1] - points[0]);
    const vec2 firstNormal = vec2f(-firstDir.y, firstDir.x);
    const vec2 firstOffset = firstNormal * (lineWidth / 2.0f);

    outVertices.push_back({vec3f(points[0] + firstOffset, 0.0f), color});
    outVertices.push_back({vec3f(points[0] - firstOffset, 0.0f), color});

    for (size_t i = 1; i < points.size() - 1; ++i) {
        const vec2f prevDir = glm::normalize(points[i] - points[i - 1]);
        const vec2f nextDir = glm::normalize(points[i + 1] - points[i]);

        const vec2f prevNormal = vec2f(-prevDir.y, prevDir.x);
        const vec2f nextNormal = vec2f(-nextDir.y, nextDir.x);

        const vec2f joinDir = prevNormal + nextNormal;
        const vec2f joinNormal = glm::normalize(joinDir);

        const vec2f bevelOffset = joinNormal * (lineWidth / 2.0f);

        outVertices.push_back({vec3f(points[i] + bevelOffset, 0.0f), color});
        outVertices.push_back({vec3f(points[i] - bevelOffset, 0.0f), color});
    }

    const size_t lastIndex = points.size() - 1;
    const vec2f lastDir = glm::normalize(points[lastIndex] - points[lastIndex - 1]);
    const vec2f lastNormal = vec2f(-lastDir.y, lastDir.x);
    const vec2f lastOffset = lastNormal * (lineWidth / 2.0f);

    outVertices.push_back({vec3f(points[lastIndex] + lastOffset, 0.0f), color});
    outVertices.push_back({vec3f(points[lastIndex] - lastOffset, 0.0f), color});

    if (closedLoop) {
        outVertices.push_back({vec3f(points[0] + firstOffset, 0.0f), color});
        outVertices.push_back({vec3f(points[0] - firstOffset, 0.0f), color});
    }

    return outVertices;
}