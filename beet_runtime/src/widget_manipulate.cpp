#include <runtime/widget_manipulate.h>
#include <runtime/widget_db.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <beet_math/quat.h>
#include <beet_math/utilities.h>

#include <beet_core/input.h>

#include <beet_shared/defer.h>
#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>

#include <beet_gfx/gfx_line.h>
#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/db_asset.h>
#include <beet_gfx/IconsFontAwesome5.h>
#include <beet_gfx/gfx_triangle_strip.h>
#include <beet_gfx/gfx_generate_geometry.h>

#include <cstdint>

//===INTERNAL_STRUCTS===================================================================================================
enum Manipulator : uint32_t {
    Manipulator_None = 0,
    Manipulator_Translate = 1,
    Manipulator_Rotate = 2,
    Manipulator_Scale = 3,
};

static Manipulator s_manipulator = Manipulator_None;
static bool s_manipulatorIsWorldSpace = true;
static bool s_gizmoInUse = false;
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static void widget_switch_between_manipulators() {

    if (s_gizmoInUse) {
        return;
    }

    if (input_key_down(KeyCode::N1)) {
        s_manipulator = Manipulator_None;
    }
    if (input_key_down(KeyCode::N2)) {
        s_manipulator = Manipulator_Translate;
    }
    if (input_key_down(KeyCode::N3)) {
        s_manipulator = Manipulator_Rotate;
    }
    if (input_key_down(KeyCode::N4)) {
        s_manipulator = Manipulator_Scale;
    }
    if (input_key_pressed(KeyCode::T)) {
        s_manipulatorIsWorldSpace = !s_manipulatorIsWorldSpace;
    }

    const ImGuiStyle style = ImGui::GetStyle();
    const float windowPadding = style.WindowPadding.x;
    ImVec4 hoveredColour = style.Colors[ImGuiCol_HeaderHovered];
    hoveredColour.w *= 0.4f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
    DEFER([] { ImGui::PopStyleVar(3); });

    ImGui::PushStyleColor(ImGuiCol_Border, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_Header, hoveredColour);
    DEFER([] { ImGui::PopStyleColor(2); });

    const float iconSize = ImGui::CalcTextSize("A").y;
    const uint32_t itemCount = 7;
    const ImVec2 toolbarItemSize = ImVec2{iconSize * 2.0f, iconSize * 2.0f};
    const ImVec2 toolbarSize = {toolbarItemSize.x + windowPadding * 2.0f, toolbarItemSize.y * itemCount + windowPadding * 2.0f};
    ImGui::SetNextWindowSize(toolbarSize);

    const ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    const ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_NoPadWithHalfSpacing;

    ImGui::SetNextWindowBgAlpha(0.8f);
    if (ImGui::Begin("##ManipulatorSidebar", nullptr, toolbarFlags)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

        if (ImGui::Selectable(ICON_FA_MOUSE_POINTER, s_manipulator == Manipulator_None, selectableFlags, toolbarItemSize)) {
            s_manipulator = Manipulator_None;
        }

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::NewLine();

        if (ImGui::Selectable(ICON_FA_ARROWS_ALT, s_manipulator == Manipulator_Translate, selectableFlags, toolbarItemSize)) {
            s_manipulator = Manipulator_Translate;
        }

        if (ImGui::Selectable(ICON_FA_SYNC_ALT, s_manipulator == Manipulator_Rotate, selectableFlags, toolbarItemSize)) {
            s_manipulator = Manipulator_Rotate;
        }

        if (ImGui::Selectable(ICON_FA_EXPAND_ALT, s_manipulator == Manipulator_Scale, selectableFlags, toolbarItemSize)) {
            s_manipulator = Manipulator_Scale;
        };

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::NewLine();

        if (ImGui::Selectable(s_manipulatorIsWorldSpace ? ICON_FA_GLOBE_AFRICA : ICON_FA_CUBE, false, selectableFlags, toolbarItemSize)) {
            s_manipulatorIsWorldSpace = !s_manipulatorIsWorldSpace;
        };
    }
    ImGui::End();
}

struct Ray {
    vec3f origin = {FLT_MAX, FLT_MAX, FLT_MAX};
    vec3f direction = {0, 0, 0};
};

struct OOBB {
    vec3f center;
    vec3f extents; //halfSize
    glm::quat orientation;
};

struct BeetRect {
    vec3f center;
    vec2f halfExtents;
    vec3f normal;
    vec3f up;
};

void draw_grid(const vec3f &planePoint, const glm::vec3 &planeNormal, float gridSize, int numLines, float lineWidth, uint32_t color) {
    vec3f up = {};
    if (glm::abs(planeNormal.z) < 0.999f) {
        up = glm::vec3(0, 0, 1);
    } else {
        up = glm::vec3(0, 1, 0);
    }
    const vec3f right = glm::normalize(glm::cross(planeNormal, up));
    up = glm::normalize(glm::cross(right, planeNormal));

    const float halfSize = gridSize * numLines / 2.0f;

    for (int32_t i = -numLines / 2; i <= numLines / 2; ++i) {
        const vec3f localStartX = glm::vec3(-halfSize, i * gridSize, 0);
        const vec3f localEndX = glm::vec3(halfSize, i * gridSize, 0);

        const vec3f startX = planePoint + localStartX.x * right + localStartX.y * up;
        const vec3f endX = planePoint + localEndX.x * right + localEndX.y * up;

        gfx_line_add_segment_immediate({startX, color}, {endX, color}, lineWidth);
    }

    for (int32_t i = -numLines / 2; i <= numLines / 2; ++i) {
        const vec3f localStartY = glm::vec3(i * gridSize, -halfSize, 0);
        const vec3f localEndY = glm::vec3(i * gridSize, halfSize, 0);

        const vec3f startY = planePoint + localStartY.x * right + localStartY.y * up;
        const vec3f endY = planePoint + localEndY.x * right + localEndY.y * up;

        gfx_line_add_segment_immediate({startY, color}, {endY, color}, lineWidth);
    }
}

struct Hit {
    bool collided = false;
    float distance = 0.0f;
    vec3f intersectionPoint = vec3f{0.0f, 0.0f, 0.0f};
};

static bool ray_plane_intersection(const Ray &ray, const vec3f &planePoint, const vec3f &planeNormal, Hit &outHit) {
    const float denom = glm::dot(ray.direction, planeNormal);

    if (glm::abs(denom) < std::numeric_limits<float>::epsilon()) {
        return false;
    }

    outHit.distance = glm::dot(planePoint - ray.origin, planeNormal) / denom;

    if (outHit.distance < 0) {
        return (outHit.collided = false);
    }

    outHit.intersectionPoint = ray.origin + outHit.distance * ray.direction;
    return (outHit.collided = true);
}

void draw_line_rect(const BeetRect &rect, bool intersection) {
    ASSERT_MSG(rect.normal != rect.up, "Err: normal & rect can't match, if they do right will be NaN i.e. normalize(vec3f(0,0,0) == vec3f(NaN,NaN,NaN)");
    const vec3f right = glm::normalize(glm::cross(rect.normal, rect.up));
    const vec3f up = glm::normalize(glm::cross(right, rect.normal));

    const vec3f corners[4] = {
            {rect.center + right * rect.halfExtents.x + up * rect.halfExtents.y},
            {rect.center - right * rect.halfExtents.x + up * rect.halfExtents.y},
            {rect.center - right * rect.halfExtents.x - up * rect.halfExtents.y},
            {rect.center + right * rect.halfExtents.x - up * rect.halfExtents.y},
    };

    constexpr uint32_t RGBA_HIT = 0x00FF00FF;
    constexpr uint32_t RGBA_MISS = 0xFF0000FF;
    const uint32_t color = intersection ? RGBA_HIT : RGBA_MISS;

    for (uint32_t i = 0; i < 4; ++i) {
        gfx_line_add_segment_immediate({corners[i], color}, {corners[(i + 1) % 4], color}, 3.0f);
    }
}

void draw_line_rect(const BeetRect &rect, uint32_t color) {
    ASSERT_MSG(rect.normal != rect.up, "Err: normal & rect can't match, if they do right will be NaN i.e. normalize(vec3f(0,0,0) == vec3f(NaN,NaN,NaN)");
    const vec3f right = glm::normalize(glm::cross(rect.normal, rect.up));
    const vec3f up = glm::normalize(glm::cross(right, rect.normal));

    const vec3f corners[4] = {
            {rect.center + right * rect.halfExtents.x + up * rect.halfExtents.y},
            {rect.center - right * rect.halfExtents.x + up * rect.halfExtents.y},
            {rect.center - right * rect.halfExtents.x - up * rect.halfExtents.y},
            {rect.center + right * rect.halfExtents.x - up * rect.halfExtents.y},
    };

    for (uint32_t i = 0; i < 4; ++i) {
        gfx_line_add_segment_immediate({corners[i], color}, {corners[(i + 1) % 4], color}, 3.0f);
    }
}

void draw_poly_rect(const BeetRect &rect, uint32_t color) {
    ASSERT_MSG(rect.normal != rect.up, "Err: normal & rect can't match, if they do right will be NaN i.e. normalize(vec3f(0,0,0) == vec3f(NaN,NaN,NaN)");
    const vec3f right = glm::normalize(glm::cross(rect.normal, rect.up));
    const vec3f up = glm::normalize(glm::cross(right, rect.normal));

    const std::vector<LinePoint3D> corners = {
            {{rect.center + right * rect.halfExtents.x + up * rect.halfExtents.y}, color},
            {{rect.center - right * rect.halfExtents.x + up * rect.halfExtents.y}, color},
            {{rect.center + right * rect.halfExtents.x - up * rect.halfExtents.y}, color},
            {{rect.center - right * rect.halfExtents.x - up * rect.halfExtents.y}, color},
    };

    for (uint32_t i = 0; i < 4; ++i) {
        gfx_triangle_strip_add_segment_immediate(corners);
    }
}


static bool ray_rect_intersection(const Ray &ray, const BeetRect &rect, Hit &outHit) {
    if (ray_plane_intersection(ray, rect.center, rect.normal, outHit)) {
        ASSERT_MSG(rect.normal != rect.up, "Err: normal & rect can't match, if they do right will be NaN i.e. normalize(vec3f(0,0,0) == vec3f(NaN,NaN,NaN)");
        const vec3f right = glm::normalize(glm::cross(rect.normal, rect.up));
        const vec3f up = glm::normalize(glm::cross(right, rect.normal));

        const vec3f localPoint = outHit.intersectionPoint - rect.center;
        const float localX = glm::dot(localPoint, right);
        const float localY = glm::dot(localPoint, up);

        outHit.collided = (std::abs(localX) > rect.halfExtents.x || std::abs(localY) > rect.halfExtents.y) ? false : true;
    }
    return outHit.collided;
}


static bool ray_oob_intersection(const Ray &ray, const OOBB &oobb, Hit &outHit) {
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), oobb.center) * glm::mat4_cast(oobb.orientation);
    glm::mat4 invModelMatrix = glm::inverse(modelMatrix);
    glm::vec3 localOrigin = glm::vec3(invModelMatrix * glm::vec4(ray.origin, 1.0f));
    glm::vec3 localDirection = glm::normalize(glm::vec3(invModelMatrix * glm::vec4(ray.direction, 0.0f)));

    glm::vec3 min = -oobb.extents;
    glm::vec3 max = oobb.extents;

    outHit.distance = 0.0f;
    float tMax = FLT_MAX;

    for (int i = 0; i < 3; ++i) {
        if (glm::abs(localDirection[i]) < std::numeric_limits<float>::epsilon()) {
            if (localOrigin[i] < min[i] || localOrigin[i] > max[i]) {
                outHit.collided = false;
                return false;
            }
        } else {
            float ood = 1.0f / localDirection[i];
            float t1 = (min[i] - localOrigin[i]) * ood;
            float t2 = (max[i] - localOrigin[i]) * ood;
            if (t1 > t2) std::swap(t1, t2);

            outHit.distance = fmax(outHit.distance, t1);
            tMax = fmin(tMax, t2);

            if (outHit.distance > tMax) {
                outHit.collided = false;
                return false;
            }
        }
    }

    outHit.intersectionPoint = ray.origin + ray.direction * outHit.distance;
    outHit.collided = true;
    return true;
}

void draw_line_square(const glm::vec3 &center, float size, uint32_t color, const glm::mat4 &modelTransform, float lineWidth = 1.0f) {
    const float halfSize = size * 0.5f;
    vec3f vertices[4] = {
            glm::vec3(-halfSize, -halfSize, 0.0f),
            glm::vec3(halfSize, -halfSize, 0.0f),
            glm::vec3(halfSize, halfSize, 0.0f),
            glm::vec3(-halfSize, halfSize, 0.0f)
    };

    for (uint32_t i = 0; i < 4; ++i) {
        vertices[i] = glm::vec3(modelTransform * glm::vec4(vertices[i] + center, 1.0f));
    }

    const uint32_t edges[4][2] = {
            {0, 1},
            {1, 2},
            {2, 3},
            {3, 0}
    };

    for (uint32_t i = 0; i < 4; ++i) {
        const LinePoint3D start = {
                .position = vertices[edges[i][0]],
                .color = color,
        };
        const LinePoint3D end = {
                .position = vertices[edges[i][1]],
                .color = color,
        };
        gfx_line_add_segment_immediate(start, end, lineWidth);
    }
}

static void draw_line_box(const mat4 &model, const vec3f &halfSizes, const uint32_t color, const float lineWidth) {
    constexpr uint32_t vertCount = 8;
    glm::vec3 vertices[vertCount] = {
            vec3(model * vec4(vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z), 1)),
            vec3(model * vec4(vec3(halfSizes.x, -halfSizes.y, -halfSizes.z), 1)),
            vec3(model * vec4(vec3(halfSizes.x, halfSizes.y, -halfSizes.z), 1)),
            vec3(model * vec4(vec3(-halfSizes.x, halfSizes.y, -halfSizes.z), 1)),
            vec3(model * vec4(vec3(-halfSizes.x, -halfSizes.y, halfSizes.z), 1)),
            vec3(model * vec4(vec3(halfSizes.x, -halfSizes.y, halfSizes.z), 1)),
            vec3(model * vec4(vec3(halfSizes.x, halfSizes.y, halfSizes.z), 1)),
            vec3(model * vec4(vec3(-halfSizes.x, halfSizes.y, halfSizes.z), 1)),
    };

    constexpr uint32_t edgeCount = 12;
    vec2i edges[edgeCount] = {{0, 1},
                              {1, 2},
                              {2, 3},
                              {3, 0},
                              {4, 5},
                              {5, 6},
                              {6, 7},
                              {7, 4},
                              {0, 4},
                              {1, 5},
                              {2, 6},
                              {3, 7}
    };

    for (uint32_t i = 0; i < edgeCount; ++i) {
        const LinePoint3D start = {vertices[edges[i].x], color};
        const LinePoint3D end = {vertices[edges[i].y], color};
        gfx_line_add_segment_immediate(start, end, lineWidth);
    }
}

static Ray
screen_to_ray(const int32_t mouseX, const int32_t mouseY, const int32_t screenWidth, const int32_t screenHeight, const Camera &camera, const Transform &cameraTransform) {
    const vec3f camForward = glm::quat(cameraTransform.rotation) * WORLD_FORWARD;
    const vec3f lookTarget = cameraTransform.position + camForward;

    const mat4 view = lookAt(cameraTransform.position, lookTarget, WORLD_UP);
    const mat4 projection = perspective(as_radians(camera.fov), (float) screenWidth / (float) screenHeight, camera.zNear, camera.zFar);

    float x = (2.0f * float(mouseX)) / float(screenWidth) - 1.0f;
    float y = 1.0f - (2.0f * float(mouseY)) / float(screenHeight);

    const vec2f ndc(x, y);
    const vec4 clipCoords(ndc, -1.0f, 1.0f);
    const mat4 invProjectionMatrix = glm::inverse(projection);

    vec4 viewCoords = invProjectionMatrix * clipCoords;
    viewCoords.z = -1.0f;
    viewCoords.w = 0.0f;

    const mat4 invViewMatrix = glm::inverse(view);
    const vec4f worldCoords = invViewMatrix * viewCoords;
    const vec3f direction = glm::normalize(vec3f(worldCoords));

    return {.origin = cameraTransform.position, .direction = direction};
}

static vec3f closest_point_on_line_segment_from_ray(const vec3f &rayOrigin, const vec3f &rayDirection, const vec3f &linePoint, const vec3f &lineDirection, float maxLength) {
    const vec3f r = rayOrigin - linePoint;
    const float a = glm::dot(rayDirection, rayDirection);
    const float b = glm::dot(rayDirection, lineDirection);
    const float c = glm::dot(lineDirection, lineDirection);
    const float d = glm::dot(rayDirection, r);
    const float e = glm::dot(lineDirection, r);
    const float denominator = a * c - b * b;

    if (glm::abs(denominator) < 1e-6) {
        return linePoint;
    }

    const float t2 = (a * e - b * d) / denominator;
    const vec3f closestPoint = linePoint + t2 * lineDirection;
    return closestPoint;
}

static void gizmo_translate_object_2_axis(Transform &transform, const Ray &ray, const vec3f &planeNormal, const bool isGizmoSelected, bool &isGrabbing) {
    static vec3f initialOffset = {};
    static vec3f initialPosition = {};

    if (isGizmoSelected) {
        if (!isGrabbing) {
            initialOffset = {};
            initialPosition = {};
            isGrabbing = true;
            Hit hitResult = {};
            if (ray_plane_intersection(ray, transform.position, planeNormal, hitResult)) {
                initialOffset = transform.position - hitResult.intersectionPoint;
                initialPosition = transform.position;
            }
        }

        Hit hitResult = {};
        if (ray_plane_intersection(ray, transform.position, planeNormal, hitResult)) {
            transform.position = hitResult.intersectionPoint + initialOffset;
            gfx_line_add_segment_immediate({initialPosition, 0xFFFFFF00}, {transform.position, 0xFFFFFF00});
        }
    }
}

static void gizmo_translate_object_1_axis(Transform &transform, const Ray &ray, const vec3f moveAxisConstraint, const bool isGizmoSelected, bool isGrabbing) {
    if (isGizmoSelected) {
        static vec3f initialOffset = {};
        static vec3f initialPosition = {};
        if (!isGrabbing) {
            initialOffset = {};
            isGrabbing = true;
            const vec3f forwardDirection = moveAxisConstraint;
            const vec3f forwardStart = transform.position;
            const vec3f closestPoint = closest_point_on_line_segment_from_ray(ray.origin, ray.direction, forwardStart, forwardDirection, 10.0f);
            if (forwardStart != closestPoint) {
                initialOffset = transform.position - closestPoint;
                initialPosition = transform.position;
            }
        }

        const vec3f forwardDirection = moveAxisConstraint;
        const vec3f forwardStart = transform.position;
        const vec3f closestPoint = closest_point_on_line_segment_from_ray(ray.origin, ray.direction, forwardStart, forwardDirection, 10.0f);
        const vec3f projectedPoint = forwardStart + forwardDirection * glm::dot(closestPoint - forwardStart, forwardDirection);
        gfx_line_add_segment_immediate({initialPosition, 0xAAAAAAC0}, {initialPosition + (forwardDirection * 100.0f), 0xAAAAAA01});
        gfx_line_add_segment_immediate({initialPosition, 0xAAAAAAC0}, {initialPosition - (forwardDirection * 100.0f), 0xAAAAAA01});

        if (forwardStart != closestPoint) {
            transform.position = projectedPoint + initialOffset;
            gfx_line_add_segment_immediate({initialPosition, 0xFFFFFF00}, {transform.position, 0xFFFFFF00});
        }
    }
}

void draw_arc(const vec3f &center,
              float radius, uint32_t color,
              const mat4 &modelTransform,
              const float arcPercent = 1.0f,
              const float startOffsetPercent = 0.0f,
              const uint32_t segments = 36,
              const float lineWidth = 1.0f) {

    const float arcPercentClamped = glm::clamp(arcPercent, 0.0f, 1.0f);
    const float startOffsetPercentClamped = glm::clamp(startOffsetPercent, 0.0f, 1.0f);

    const uint32_t arcSegments = uint32_t(segments * arcPercentClamped);
    const float startAngle = 2.0f * glm::pi<float>() * startOffsetPercentClamped;
    const float endAngle = startAngle + 2.0f * glm::pi<float>() * arcPercentClamped;

    std::vector<vec3f> arcPoints(arcSegments + 1);
    for (uint32_t i = 0; i <= arcSegments; ++i) {
        const float theta = startAngle + (endAngle - startAngle) * float(i) / float(arcSegments);
        const float x = radius * cos(theta);
        const float y = radius * sin(theta);
        arcPoints[i] = vec3f(x, y, 0.0f);
    }

    for (uint32_t i = 0; i < arcSegments; ++i) {
        const LinePoint3D start = {
                .position = glm::vec3(modelTransform * glm::vec4(arcPoints[i] + center, 1.0f)),
                .color = color
        };
        const LinePoint3D end = {
                .position = glm::vec3(modelTransform * glm::vec4(arcPoints[i + 1] + center, 1.0f)),
                .color = color
        };
        gfx_line_add_segment_immediate(start, end, lineWidth);
    }
}

void draw_arc_polyline(const vec3f &center,
                       const float radius,
                       const uint32_t color,
                       const mat4 &modelTransform,
                       const float arcPercent = 1.0f,
                       const float startOffsetPercent = 0.0f,
                       const uint32_t segments = 36,
                       const float lineWidth = 1.0f,
                       const bool closedLoop = false) {

    std::vector<vec2f> points;
    points.reserve(segments + 1);

    const float startAngle = glm::tau<float>() * startOffsetPercent;
    const float endAngle = glm::tau<float>() * arcPercent + startAngle;

    for (uint32_t i = 0; i <= segments; ++i) {
        const float angle = startAngle + (endAngle - startAngle) * float(i) / float(segments);
        const float x = center.x + radius * glm::cos(angle);
        const float y = center.y + radius * glm::sin(angle);
        points.emplace_back(x, y);
    }

    std::vector<LinePoint3D> vertices = gfx_generate_geometry_thick_polyline(points, lineWidth, color, closedLoop);
    for (auto &p: vertices) {
        p.position = vec3f(modelTransform * vec4f(p.position, 1));
    }
    gfx_triangle_strip_add_segment_immediate(vertices);
}

void draw_cone(const vec3f &baseCenter, const float radius, const float height, const uint32_t color, const mat4 &modelTransform, const uint32_t segments) {
    std::vector<LinePoint3D> vertices = gfx_generate_geometry_cone(baseCenter, radius, height, color, segments);
    for (auto &p: vertices) {
        p.position = vec3f(modelTransform * vec4f(p.position, 1));
    }
    gfx_triangle_strip_add_segment_immediate(vertices);
}

static void widget_manipulate_rotate(Transform &transform) {
    const CameraEntity &camEntity = *db_get_camera_entity(0);
    const Transform *cameraTransform = db_get_transform(camEntity.transformIndex);
    const Camera *camera = db_get_camera(camEntity.cameraIndex);

    const float constantSizeScale = 1.0f * (glm::distance(cameraTransform->position, transform.position) / tanf(camera->fov) / 2.0f);
    const mat4 model = translate(mat4(1.0f), transform.position) * toMat4(quat(transform.rotation)) * scale(glm::mat4(1.0f), vec3f(-constantSizeScale));
    constexpr uint32_t RGBA_BLUE = 0x3333FFFF;
    constexpr uint32_t RGBA_BLUE_LOW_ALPHA = 0x3333FF80;

    constexpr uint32_t RGBA_GREEN = 0x33FF33FF;
    constexpr uint32_t RGBA_GREEN_LOW_ALPHA = 0x33FF3380;

    constexpr uint32_t RGBA_RED = 0xFF3333FF;
    constexpr uint32_t RGBA_RED_LOW_ALPHA = 0xFF333380;

    //TODO: I would like to do some dot product test against the camera -> transform and if we are over some threshold we should re-orient the gizmo.
    draw_arc({}, 1.0f, RGBA_GREEN, model, 0.25f, 0.0f, 36, 3.0f);
    draw_arc({}, 0.9f, RGBA_GREEN, model, 0.25f, 0.0f, 36, 3.0f);
    draw_arc_polyline({}, 0.95f, RGBA_GREEN_LOW_ALPHA, model, 0.25f, 0.0f, 36, 0.1f, false);

    mat4 model2 = rotate(model, glm::pi<float>() / 2.0f, vec3f(0, -1.0f, 0));
    draw_arc({}, 1.0f, RGBA_RED, model2, 0.25f, 0.0f, 36, 3.0f);
    draw_arc({}, 0.9f, RGBA_RED, model2, 0.25f, 0.0f, 36, 3.0f);
    draw_arc_polyline({}, 0.95f, RGBA_RED_LOW_ALPHA, model2, 0.25f, 0.0f, 36, 0.1f, false);

    mat4 model3 = rotate(model, glm::pi<float>() / 2.0f, vec3f(1.0f, 0, 0));
    draw_arc({}, 1.0f, RGBA_BLUE, model3, 0.25f, 0.0f, 36, 3.0f);
    draw_arc({}, 0.9f, RGBA_BLUE, model3, 0.25f, 0.0f, 36, 3.0f);
    draw_arc_polyline({}, 0.95f, RGBA_BLUE_LOW_ALPHA, model3, 0.25f, 0.0f, 36, 0.1f, false);
}


std::vector<LinePoint3D> gfx_generate_geometry_cylinder(const glm::vec3 &baseCenter, float radius, float height, uint32_t color, int segments, bool cappedEnds = true) {
    std::vector<LinePoint3D> vertices;

    const glm::vec3 topCenter = baseCenter + glm::vec3(0.0f, height, 0.0f);

    if (cappedEnds) {
        for (uint32_t i = 0; i <= segments; ++i) {
            const float angle = glm::two_pi<float>() * float(i) / float(segments);
            const float x = radius * glm::cos(angle);
            const float z = radius * glm::sin(angle);

            glm::vec3 bottomVertex(baseCenter.x + x, baseCenter.y, baseCenter.z + z);
            vertices.push_back({baseCenter, color});
            vertices.push_back({bottomVertex, color});
        }
    }

    for (uint32_t i = 0; i <= segments; ++i) {
        const float angle = glm::two_pi<float>() * float(i) / float(segments);
        const float x = radius * glm::cos(angle);
        const float z = radius * glm::sin(angle);

        glm::vec3 bottomVertex(baseCenter.x + x, baseCenter.y, baseCenter.z + z);
        glm::vec3 topVertex(topCenter.x + x, topCenter.y, topCenter.z + z);

        vertices.push_back({bottomVertex, color});
        vertices.push_back({topVertex, color});

    }

    if (cappedEnds) {
        for (uint32_t i = 0; i <= segments; ++i) {
            const float angle = glm::two_pi<float>() * float(i) / float(segments);
            const float x = radius * glm::cos(angle);
            const float z = radius * glm::sin(angle);

            glm::vec3 topVertex(topCenter.x + x, topCenter.y, topCenter.z + z);

            vertices.push_back({topVertex, color});
            vertices.push_back({topCenter, color});
        }
    }

    return vertices;
}

void draw_cylinder(const glm::vec3 &baseCenter, float radius, float height, uint32_t color, const mat4 &modelTransform, int segments) {
    std::vector<LinePoint3D> vertices = gfx_generate_geometry_cylinder(baseCenter, radius, height, color, segments);
    for (auto &p: vertices) {
        p.position = vec3f(modelTransform * vec4f(p.position, 1));
    }
    gfx_triangle_strip_add_segment_immediate(vertices);
}

std::vector<LinePoint3D> generateSphere(const glm::vec3 &center, float radius, uint32_t color, int segments, int rings) {
    std::vector<LinePoint3D> vertices;

    for (uint32_t i = 0; i < rings; ++i) {
        const float theta1 = glm::pi<float>() * float(i) / float(rings);
        const float theta2 = glm::pi<float>() * float(i + 1) / float(rings);
        const float sinTheta1 = glm::sin(theta1);
        const float cosTheta1 = glm::cos(theta1);
        const float sinTheta2 = glm::sin(theta2);
        const float cosTheta2 = glm::cos(theta2);

        for (uint32_t j = 0; j <= segments; ++j) {
            const float phi = glm::two_pi<float>() * float(j) / float(segments);
            const float sinPhi = glm::sin(phi);
            const float cosPhi = glm::cos(phi);

            vec3f position1(
                    center.x + radius * sinTheta1 * cosPhi,
                    center.y + radius * cosTheta1,
                    center.z + radius * sinTheta1 * sinPhi
            );

            vec3f position2(
                    center.x + radius * sinTheta2 * cosPhi,
                    center.y + radius * cosTheta2,
                    center.z + radius * sinTheta2 * sinPhi
            );

            vertices.push_back({position1, color});
            vertices.push_back({position2, color});
        }
    }

    return vertices;
}

void draw_sphere(const glm::vec3 &center, float radius, uint32_t color, const mat4 &modelTransform, int segments = 36, int rings = 18) {
    std::vector<LinePoint3D> vertices = generateSphere(center, radius, color, segments, rings);

    for (auto &p: vertices) {
        p.position = vec3f(modelTransform * vec4f(p.position, 1));
    }
    gfx_triangle_strip_add_segment_immediate(vertices);
}

std::vector<LinePoint3D> generate_square(const glm::vec3 &center, float size, uint32_t color) {
    std::vector<LinePoint3D> vertices;

    const float halfSize = size / 2.0f;

    const vec3f topLeft = vec3f(center.x - halfSize, center.y + halfSize, center.z);
    const vec3f topRight = vec3f(center.x + halfSize, center.y + halfSize, center.z);
    const vec3f bottomLeft = vec3f(center.x - halfSize, center.y - halfSize, center.z);
    const vec3f bottomRight = vec3f(center.x + halfSize, center.y - halfSize, center.z);

    vertices.push_back({bottomLeft, color});
    vertices.push_back({topLeft, color});
    vertices.push_back({bottomRight, color});
    vertices.push_back({topRight, color});

    return vertices;
}

void draw_square(const glm::vec3 &center, float size, uint32_t color, const mat4 &modelTransform) {
    std::vector<LinePoint3D> vertices = generate_square(center, size, color);
    for (auto &p: vertices) {
        p.position = vec3f(modelTransform * vec4f(p.position, 1));
    }
    gfx_triangle_strip_add_segment_immediate(vertices);
}

bool orientation_dot_test(const glm::vec3 &gizmoPosition, const glm::vec3 &cameraPosition, const vec3f referenceDirection) {
    glm::vec3 gizmoToCamera = glm::normalize(cameraPosition - gizmoPosition);
    const float dotProduct = glm::dot(referenceDirection, gizmoToCamera);
    return (dotProduct > 0.0f) ? true : false;
}

void set_closest_hovered(const Hit &hitResultForward,
                         const Hit &hitResultUp,
                         const Hit &hitResultRight,
                         const Hit &hitResultUpRight,
                         const Hit &hitResultForwardUp,
                         const Hit &hitResultForwardRight,
                         bool &forwardHovered,
                         bool &upHovered,
                         bool &rightHovered,
                         bool &upRightHovered,
                         bool &forwardRightHovered,
                         bool &forwardUpHovered
) {
    forwardHovered = false;
    upHovered = false;
    rightHovered = false;
    upRightHovered = false;
    forwardRightHovered = false;
    forwardUpHovered = false;

    struct HitHoverSelection {
        const Hit &hit;
        bool &isHovered;
    };

    std::vector<HitHoverSelection> hoverHitTests = {
            {hitResultForward,      forwardHovered},
            {hitResultUp,           upHovered},
            {hitResultRight,        rightHovered},
            {hitResultUpRight,      upRightHovered},
            {hitResultForwardUp,    forwardUpHovered},
            {hitResultForwardRight, forwardRightHovered}
    };

    float minDistance = FLT_MAX;
    HitHoverSelection *closestHit = nullptr;

    for (auto &hitResult: hoverHitTests) {
        if (hitResult.hit.collided && hitResult.hit.distance < minDistance) {
            minDistance = hitResult.hit.distance;
            closestHit = &hitResult;
        }
    }

    if (closestHit) {
        closestHit->isHovered = true;
    }
}

static void widget_manipulate_transform(Transform &transform) {
    const CameraEntity &camEntity = *db_get_camera_entity(0);
    const Transform &cameraTransform = *db_get_transform(camEntity.transformIndex);
    const Camera &camera = *db_get_camera(camEntity.cameraIndex);

    const float constantSizeScale = 1.0f * (glm::distance(cameraTransform.position, transform.position) / tanf(camera.fov) / 2.0f);
    mat4 model = mat4(1.0f);

    if (s_manipulatorIsWorldSpace) {
        model = translate(mat4(1.0f), transform.position) * scale(glm::mat4(1.0f), vec3(-constantSizeScale));
    } else {
        model = translate(mat4(1.0f), transform.position) * toMat4(quat(transform.rotation)) * scale(glm::mat4(1.0f), vec3(-constantSizeScale));
    }

    const vec3f forwardUpRectNormal = glm::normalize(vec3f(model * vec4f(1.0f, 0.0f, 0.0f, 0.0f)));
    const vec3f upRightRectNormal = glm::normalize(vec3f(model * vec4f(0.0f, 0.0f, 1.0f, 0.0f)));
    const vec3f forwardRightRectNormal = glm::normalize(vec3f(model * vec4f(0.0f, 1.0f, 0.0f, 0.0f)));

    const vec3f upTranslateNormal = glm::normalize(mat4f_extract_rotation_quat(model) * WORLD_UP);
    const vec3f rightTranslateNormal = glm::normalize(mat4f_extract_rotation_quat(model) * WORLD_RIGHT);
    const vec3f forwardTranslateNormal = glm::normalize(mat4f_extract_rotation_quat(model) * WORLD_FORWARD);

    const bool flipUp = orientation_dot_test(transform.position, cameraTransform.position, -mat4f_extract_up(model));
    const bool flipRight = orientation_dot_test(transform.position, cameraTransform.position, mat4f_extract_right(model));
    const bool flipForward = orientation_dot_test(transform.position, cameraTransform.position, mat4f_extract_forward(model));

    static bool forwardIsSelected = false;
    static bool upIsSelected = false;
    static bool rightIsSelected = false;
    static bool upRightIsSelected = false;
    static bool forwardRightIsSelected = false;
    static bool forwardUpIsSelected = false;

    s_gizmoInUse = (forwardIsSelected || upIsSelected || rightIsSelected || upRightIsSelected || forwardRightIsSelected || forwardUpIsSelected);

    if (input_mouse_released(MouseButton::Left)) {
        s_gizmoInUse = false;
        forwardIsSelected = false;
        upIsSelected = false;
        rightIsSelected = false;
        upRightIsSelected = false;
        forwardRightIsSelected = false;
        forwardUpIsSelected = false;
    }

    bool forwardHovered = false;
    bool upHovered = false;
    bool rightHovered = false;

    bool upRightHovered = false;
    bool forwardRightHovered = false;
    bool forwardUpHovered = false;

    const vec2i mousePos = input_mouse_position();
    const vec2i screenSize = gfx_screen_size();

    float tMin = {};
    const Ray ray = screen_to_ray(mousePos.x, mousePos.y, screenSize.x, screenSize.y, camera, cameraTransform);

    Hit hitResultForward = {};
    Hit hitResultUp = {};
    Hit hitResultRight = {};
    Hit hitResultUpRight = {};
    Hit hitResultForwardUp = {};
    Hit hitResultForwardRight = {};

    static BeetRect lastHitRect = {};

    const mat4 modelForward = translate(model, (((WORLD_FORWARD * (flipForward ? -1.0f : 1.0f)) * 0.5f)));
    const vec3 scaleForward = {0.1f, 0.1f, 0.5f};
    const OOBB forwardOOBB = {
            .center = mat4f_extract_position(modelForward),
            .extents = scaleForward * mat4f_extract_scale(modelForward),
            .orientation = mat4f_extract_rotation_quat(modelForward),
    };

    const mat4 modelUp = translate(model, (((WORLD_UP * (flipUp ? -1.0f : 1.0f)) * 0.5f)));
    const vec3 scaleUp = {0.1f, 0.5f, 0.1f};
    const OOBB upOOBB = {
            .center = mat4f_extract_position(modelUp),
            .extents = scaleUp * mat4f_extract_scale(modelUp),
            .orientation = mat4f_extract_rotation_quat(modelUp),
    };


    const mat4 modelRight = translate(model, (((WORLD_RIGHT * (flipRight ? -1.0f : 1.0f)) * 0.5f)));
    const vec3 scaleRight = {0.5f, 0.1f, 0.1f};
    const OOBB rightOOBB = {
            .center = mat4f_extract_position(modelRight),
            .extents = scaleRight * mat4f_extract_scale(modelRight),
            .orientation = mat4f_extract_rotation_quat(modelRight),
    };


    const float upRightAngle = flipUp ? glm::pi<float>() : 0;
    const vec3f upRightNormal = vec3f(1.0f, 0.0f, 0.0f);
    const mat4 mdlUp = rotate(model, upRightAngle, upRightNormal);
    const vec3f upRightOffset = vec3f{
            flipRight ? 0.35f : -0.35f,
            flipUp ? -0.35f : 0.35f,
            0.0f
    };
    const BeetRect upRightRect{
            .center = mat4f_extract_position(translate(model, upRightOffset)),
            .halfExtents = vec2f{0.07f, 0.07f} * vec2f(glm::length(vec3(mdlUp[0])), glm::length(vec3(mdlUp[1]))),
            .normal = upRightRectNormal,
            .up = mat4f_extract_up(model),
    };


    const vec3f forwardUpNormal = vec3f(0.0f, 0.0f, 1.0f);
    const float forwardUpAngle = flipRight ? ((glm::pi<float>() / 2) * 3) : (glm::pi<float>() / 2); // i.e. 3/4 TAU : 1/4 TAU
    const mat4 mdlRight = rotate(model, forwardUpAngle, forwardUpNormal);
    const vec3 forwardUpOffset = vec3f{
            0.0f,
            flipUp ? -0.35f : 0.35f,
            flipForward ? 0.35f : -0.35f
    };
    const BeetRect forwardUpRect{
            .center = mat4f_extract_position(translate(model, forwardUpOffset)),
            .halfExtents = vec2f{0.07f, 0.07f} * vec2f(glm::length(vec3(mdlRight[0])), glm::length(vec3(mdlRight[1]))),
            .normal = forwardUpRectNormal,
            .up = mat4f_extract_up(model),
    };


    const vec3f forwardRightNormal = vec3f(1.0f, 0.0f, 0.0f);
    const float forwardRightAngle = flipForward ? (glm::pi<float>() / 2) : ((glm::pi<float>() / 2) * 3); // i.e. 1/4 TAU : 3/4 TAU
    const mat4 mdlForward = rotate(model, forwardRightAngle, forwardRightNormal);
    const vec3 forwardRightOffset = vec3f{
            flipRight ? 0.35f : -0.35f,
            0.0f,
            flipForward ? 0.35f : -0.35f
    };
    const BeetRect forwardRightRect{
            .center = mat4f_extract_position(translate(model, forwardRightOffset)),
            .halfExtents = vec2f{0.07f, 0.07f} * vec2f(glm::length(vec3(mdlForward[0])), glm::length(vec3(mdlForward[1]))),
            .normal = forwardRightRectNormal,
            .up = mat4f_extract_right(model),
    };


    if (!s_gizmoInUse) {
        ray_rect_intersection(ray, upRightRect, hitResultUpRight);
        ray_rect_intersection(ray, forwardRightRect, hitResultForwardRight);
        ray_rect_intersection(ray, forwardUpRect, hitResultForwardUp);
        ray_oob_intersection(ray, rightOOBB, hitResultRight);
        ray_oob_intersection(ray, upOOBB, hitResultUp);
        ray_oob_intersection(ray, forwardOOBB, hitResultForward);

        set_closest_hovered(
                hitResultForward,
                hitResultUp,
                hitResultRight,
                hitResultUpRight,
                hitResultForwardUp,
                hitResultForwardRight,
                forwardHovered,
                upHovered,
                rightHovered,
                upRightHovered,
                forwardRightHovered,
                forwardUpHovered
        );

        //DEBUG BOUNDS
        //draw_line_box(modelRight, scaleRight, BOX_GREEN_RGB, 1);
        //draw_line_box(modelUp, scaleUp, BOX_GREEN_RGB, 1);
        //draw_line_box(modelForward, scaleForward, BOX_GREEN_RGB, 1);
    }

    if (upRightHovered) {
        lastHitRect = upRightRect;
        upRightIsSelected = input_mouse_pressed(MouseButton::Left);
    }
    if (forwardRightHovered) {
        lastHitRect = forwardRightRect;
        forwardRightIsSelected = input_mouse_pressed(MouseButton::Left);
    }
    if (forwardUpHovered) {
        lastHitRect = forwardUpRect;
        forwardUpIsSelected = input_mouse_pressed(MouseButton::Left);
    }
    if (rightHovered) {
        rightIsSelected = input_mouse_pressed(MouseButton::Left);
    }
    if (upHovered) {
        upIsSelected = input_mouse_pressed(MouseButton::Left);
    }
    if (forwardHovered) {
        forwardIsSelected = input_mouse_pressed(MouseButton::Left);
    }

    constexpr uint32_t BOX_GREEN_RGB = 0x66FF6600; // 0 Alpha lowers opacity when line is behind geo
    constexpr uint32_t RGBA_RED = 0xFF3333FF;
    constexpr uint32_t RGBA_RED_HOVERED = 0xFF9999FF;
    constexpr uint32_t RGBA_GREEN = 0x33FF33FF;
    constexpr uint32_t RGBA_GREEN_HOVERED = 0x99FF99FF;
    constexpr uint32_t RGBA_BLUE = 0x3333FFFF;
    constexpr uint32_t RGBA_BLUE_HOVERED = 0x9999FFFF;
    constexpr uint32_t RGBA_WHITE = 0xFFFFFFFF;
    constexpr float GIZMO_LINE_THICKNESS = 2.0f;

    const uint32_t UP_GIZMO_COLOUR = upHovered ? RGBA_GREEN_HOVERED : RGBA_GREEN;
    const uint32_t FORWARD_GIZMO_COLOUR = forwardHovered ? RGBA_BLUE_HOVERED : RGBA_BLUE;
    const uint32_t RIGHT_GIZMO_COLOUR = rightHovered ? RGBA_RED_HOVERED : RGBA_RED;

    const uint32_t UP_RIGHT_GIZMO_COLOUR = upRightHovered ? RGBA_BLUE_HOVERED : RGBA_BLUE;
    const uint32_t UP_RIGHT_GIZMO_COLOUR_ALPHA = upRightHovered ? 0x6666FFFF : 0x0000FFFF;
    const uint32_t FORWARD_RIGHT_GIZMO_COLOUR = forwardRightHovered ? RGBA_GREEN : RGBA_GREEN_HOVERED;
    const uint32_t FORWARD_RIGHT_GIZMO_COLOUR_ALPHA = forwardRightHovered ? 0x66FF6660 : 0x00FF0060;
    const uint32_t FORWARD_UP_GIZMO_COLOUR = forwardUpHovered ? RGBA_RED : RGBA_RED_HOVERED;
    const uint32_t FORWARD_UP_GIZMO_COLOUR_ALPHA = forwardUpHovered ? 0xFF666660 : 0xFF000060;

    if (s_gizmoInUse) {
        if (upRightIsSelected) {
            draw_grid(lastHitRect.center, lastHitRect.normal, 1, 60, 1, 0xAAAAAA70);
        }
        if (forwardRightIsSelected) {
            draw_grid(lastHitRect.center, lastHitRect.normal, 1, 60, 1, 0xAAAAAA70);
        }
        if (forwardUpIsSelected) {
            draw_grid(lastHitRect.center, lastHitRect.normal, 1, 60, 1, 0xAAAAAA70);
        }
    }

    draw_sphere({0, 0, 0}, 0.07, 0xEEEEEEFF, model, 12, 10);

    draw_cylinder({0, 0.07, 0}, 0.02f, 0.73, UP_GIZMO_COLOUR, mdlUp, 12);
    draw_cone({0, 0.80f, 0}, 0.07f, 0.2f, UP_GIZMO_COLOUR, mdlUp, 12);
    draw_cylinder({0, 0.07, 0}, 0.02f, 0.73, FORWARD_GIZMO_COLOUR, mdlForward, 12);
    draw_cone({0, 0.80f, 0}, 0.07f, 0.2f, FORWARD_GIZMO_COLOUR, mdlForward, 12);
    draw_cylinder({0, 0.07, 0}, 0.02f, 0.73, RIGHT_GIZMO_COLOUR, mdlRight, 12);
    draw_cone({0, 0.80f, 0}, 0.07f, 0.2f, RIGHT_GIZMO_COLOUR, mdlRight, 12);

    draw_line_rect(upRightRect, UP_RIGHT_GIZMO_COLOUR);
    draw_poly_rect(upRightRect, UP_RIGHT_GIZMO_COLOUR_ALPHA);
    draw_line_rect(forwardRightRect, FORWARD_RIGHT_GIZMO_COLOUR);
    draw_poly_rect(forwardRightRect, FORWARD_RIGHT_GIZMO_COLOUR_ALPHA);
    draw_line_rect(forwardUpRect, FORWARD_UP_GIZMO_COLOUR);
    draw_poly_rect(forwardUpRect, FORWARD_UP_GIZMO_COLOUR_ALPHA);


    gizmo_translate_object_2_axis(transform, ray, upRightRectNormal, upRightIsSelected, s_gizmoInUse);
    gizmo_translate_object_2_axis(transform, ray, forwardUpRectNormal, forwardUpIsSelected, s_gizmoInUse);
    gizmo_translate_object_2_axis(transform, ray, forwardRightRectNormal, forwardRightIsSelected, s_gizmoInUse);

    gizmo_translate_object_1_axis(transform, ray, upTranslateNormal, upIsSelected, s_gizmoInUse);
    gizmo_translate_object_1_axis(transform, ray, rightTranslateNormal, rightIsSelected, s_gizmoInUse);
    gizmo_translate_object_1_axis(transform, ray, forwardTranslateNormal, forwardIsSelected, s_gizmoInUse);
}
//======================================================================================================================

//===API================================================================================================================
void widget_manipulate_update(bool &enabled) {
    constexpr uint32_t RGBA_BLUE = 0x3333FFFF;
    constexpr uint32_t RGBA_BLUE_LOW_ALPHA = 0x3333FF80;

    constexpr uint32_t RGBA_GREEN = 0x33FF33FF;
    constexpr uint32_t RGBA_GREEN_LOW_ALPHA = 0x33FF3380;

    constexpr uint32_t RGBA_RED = 0xFF3333FF;
    constexpr uint32_t RGBA_RED_LOW_ALPHA = 0xFF333380;

    if (!enabled) {
        return;
    }
    widget_switch_between_manipulators();

    const SelectedPool &poolType = *get_selected_pool_type();
    const int32 &poolIndex = *get_selected_pool_index();

    if (poolType != SELECTED_POOL_NONE && poolIndex != -1) {
        Transform *currentTransform = nullptr;
        switch (poolType) {
            case SELECTED_POOL_LIT_ENT: {
                const LitEntity &litEntity = *db_get_lit_entity(poolIndex);
                currentTransform = db_get_transform(litEntity.transformIndex);
                break;
            }
            case SELECTED_POOL_CAMERA_ENT: {
                const CameraEntity &camEntity = *db_get_camera_entity(poolIndex);
                currentTransform = db_get_transform(camEntity.transformIndex);
                break;
            }
            case SELECTED_POOL_NONE: {
                break;
            }
        }

        if (s_manipulator == Manipulator_Translate) {
            widget_manipulate_transform(*currentTransform);
        } else if (s_manipulator == Manipulator_Rotate) {
            widget_manipulate_rotate(*currentTransform);
        } else if (s_manipulator == Manipulator_Scale) {
        } else if (s_manipulator == Manipulator_None) {
        } else {
            NOT_IMPLEMENTED();
        }
        //do gizmo draw
    }
}
//======================================================================================================================