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

#include <cstdint>

//===INTERNAL_STRUCTS===================================================================================================
enum Manipulator : uint32_t {
    Manipulator_None = 0,
    Manipulator_Translate = 1,
    Manipulator_Rotate = 2,
    Manipulator_Scale = 3,
};
static Manipulator s_manipulator = Manipulator_None;
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static void widget_switch_between_manipulators() {

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
    const uint32_t itemCount = 5;
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
    }
    ImGui::End();
}

struct Ray {
    glm::vec3 origin = {FLT_MAX, FLT_MAX, FLT_MAX};
    glm::vec3 direction = {0, 0, 0};
};

struct OOBB {
    glm::vec3 center;
    glm::vec3 extents; //halfSize
    glm::quat orientation;
};

static bool ray_oob_intersection(const Ray &ray, const OOBB &oobb, float &tMin, glm::vec3 &intersectionPoint) {
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), oobb.center) * glm::mat4_cast(oobb.orientation);
    glm::mat4 invModelMatrix = glm::inverse(modelMatrix);
    glm::vec3 localOrigin = glm::vec3(invModelMatrix * glm::vec4(ray.origin, 1.0f));
    glm::vec3 localDirection = glm::normalize(glm::vec3(invModelMatrix * glm::vec4(ray.direction, 0.0f)));

    glm::vec3 min = -oobb.extents;
    glm::vec3 max = oobb.extents;

    tMin = 0.0f;
    float tMax = FLT_MAX;

    for (int i = 0; i < 3; ++i) {
        if (glm::abs(localDirection[i]) < std::numeric_limits<float>::epsilon()) {
            if (localOrigin[i] < min[i] || localOrigin[i] > max[i]) {
                return false;
            }
        } else {
            float ood = 1.0f / localDirection[i];
            float t1 = (min[i] - localOrigin[i]) * ood;
            float t2 = (max[i] - localOrigin[i]) * ood;
            if (t1 > t2) std::swap(t1, t2);

            tMin = fmax(tMin, t1);
            tMax = fmin(tMax, t2);

            if (tMin > tMax) {
                return false;
            }
        }
    }

    intersectionPoint = ray.origin + ray.direction * tMin;
    return true;
}

static void draw_box(const mat4 &model, const glm::vec3 &halfSizes, const uint32_t color, const float lineWidth) {
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

static Ray screen_to_ray(int32_t mouseX, int32_t mouseY, int32_t screenWidth, int32_t screenHeight, const Camera &camera, const Transform &cameraTransform) {
    const vec3f camForward = glm::quat(cameraTransform.rotation) * WORLD_FORWARD;
    const vec3f lookTarget = cameraTransform.position + camForward;

    const mat4 view = lookAt(cameraTransform.position, lookTarget, WORLD_UP);
    const mat4 projection = perspective(as_radians(camera.fov), (float) screenWidth / (float) screenHeight, camera.zNear, camera.zFar);

    float x = (2.0f * float(mouseX)) / float(screenWidth) - 1.0f;
    float y = 1.0f - (2.0f * float(mouseY)) / float(screenHeight);

    const vec2 ndc(x, y);
    const vec4 clipCoords(ndc, -1.0f, 1.0f);
    const mat4 invProjectionMatrix = glm::inverse(projection);

    vec4 viewCoords = invProjectionMatrix * clipCoords;
    viewCoords.z = -1.0f;
    viewCoords.w = 0.0f;

    const mat4 invViewMatrix = glm::inverse(view);
    const vec4 worldCoords = invViewMatrix * viewCoords;
    const vec3 direction = glm::normalize(glm::vec3(worldCoords));

    return {.origin = cameraTransform.position, .direction = direction};
}

static glm::vec3 closest_point_on_line_from_ray(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection, const glm::vec3 &linePoint, const glm::vec3 &lineDirection) {
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

static void gizmo_translate_object(Transform &transform, Ray &ray, const vec3f moveAxisConstraint, const bool isGizmoSelected, bool isGrabbing) {
    if (isGizmoSelected) {
        static glm::vec3 initialOffset = {};
        if (!isGrabbing) {
            initialOffset = {};
            isGrabbing = true;
            glm::quat rotation = transform.rotation;
            rotation = glm::normalize(rotation);
            const vec3 forwardDirection = glm::normalize(rotation * moveAxisConstraint);
            const vec3 forwardStart = transform.position;
            const vec3 closestPoint = closest_point_on_line_from_ray(ray.origin, ray.direction, forwardStart, forwardDirection);
            initialOffset = transform.position - closestPoint;
        }

        const quat rotation = transform.rotation;
        const vec3 forwardDirection = glm::normalize(rotation * moveAxisConstraint);
        const vec3 forwardStart = transform.position;
        const vec3 closestPoint = closest_point_on_line_from_ray(ray.origin, ray.direction, forwardStart, forwardDirection);
        const vec3 projectedPoint = forwardStart + forwardDirection * glm::dot(closestPoint - forwardStart, forwardDirection);
        transform.position = projectedPoint + initialOffset;
    }
}

static void widget_manipulate_transform(Transform &transform) {
    constexpr uint32_t BOX_GREEN_RGB = 0x33FF3300; // 0 Alpha lowers opacity when line is behind geo

    constexpr uint32_t RGBA_RED = 0xFF3333FF;
    constexpr uint32_t RGBA_RED_HOVERED = 0xFF9999FF;

    constexpr uint32_t RGBA_GREEN = 0x33FF33FF;
    constexpr uint32_t RGBA_GREEN_HOVERED = 0x99FF99FF;

    constexpr uint32_t RGBA_BLUE = 0x3333FFFF;
    constexpr uint32_t RGBA_BLUE_HOVERED = 0x9999FFFF;

    constexpr uint32_t RGBA_WHITE = 0xFFFFFFFF;
    constexpr float GIZMO_LINE_THICKNESS = 2.0f;

    const CameraEntity &camEntity = *db_get_camera_entity(0);
    Transform *cameraTransform = db_get_transform(camEntity.transformIndex);
    Camera *camera = db_get_camera(camEntity.cameraIndex);

    float constantSizeScale = 1.0f * (glm::distance(cameraTransform->position, transform.position) / tanf(camera->fov) / 2.0f);
    const mat4 model = translate(mat4(1.0f), transform.position) * toMat4(quat(transform.rotation)) * scale(glm::mat4(1.0f), vec3(-constantSizeScale));

    const OOBB oobb = {
            .center = transform.position,
            .extents = transform.scale * 0.5f,
            .orientation = glm::quat(transform.rotation),
    };

    vec2i mousePos = input_mouse_position();
    float tMin = {};
    Ray ray = {};

    static bool forwardIsSelected = false;
    static bool upIsSelected = false;
    static bool rightIsSelected = false;

    bool gizmoInUse = (forwardIsSelected || upIsSelected || rightIsSelected);

    if (input_mouse_released(MouseButton::Left)) {
        gizmoInUse = false;
        forwardIsSelected = false;
        upIsSelected = false;
        rightIsSelected = false;
    }

    if (input_mouse_down(MouseButton::Right)) {
        gizmoInUse = true;
    }

    const vec2i screenSize = gfx_screen_size();
    ray = screen_to_ray(mousePos.x, mousePos.y, screenSize.x, screenSize.y, *camera, *cameraTransform);

    bool forwardHovered = false;
    {
        if (!gizmoInUse) {
            const mat4 modelForward = translate(model, ((WORLD_FORWARD * 0.5f)));
            const vec3 scaleForward = {0.1f, 0.1f, 0.5f};
            OOBB forwardBound = {
                    .center = glm::vec3(modelForward[3]), // I don't want to decompose the whole thing
                    .extents = scaleForward * vec3(glm::length(vec3(modelForward[0])), glm::length(vec3(modelForward[1])), glm::length(vec3(modelForward[2]))),
                    .orientation = glm::quat(transform.rotation),
            };
            glm::vec3 intersectionPoint;
            if (ray_oob_intersection(ray, forwardBound, tMin, intersectionPoint)) {
                forwardHovered = true;
                forwardIsSelected = input_mouse_pressed(MouseButton::Left);
            }
            draw_box(modelForward, scaleForward, BOX_GREEN_RGB, 1);
        }
    }

    bool upHovered = false;
    if (!gizmoInUse) {
        mat4 modelUp = translate(model, ((WORLD_UP * 0.5f)));
        const vec3 scaleUp = {0.1f, 0.5f, 0.1f};
        OOBB upBound = {
                .center = glm::vec3(modelUp[3]), // I don't want to decompose the whole thing
                .extents = scaleUp * vec3(glm::length(vec3(modelUp[0])), glm::length(vec3(modelUp[1])), glm::length(vec3(modelUp[2]))),
                .orientation = glm::quat(transform.rotation),
        };
        glm::vec3 intersectionPoint;
        if (ray_oob_intersection(ray, upBound, tMin, intersectionPoint)) {
            upHovered = true;
            upIsSelected = input_mouse_pressed(MouseButton::Left);
        }
        draw_box(modelUp, scaleUp, BOX_GREEN_RGB, 1);
    }

    bool rightHovered = false;
    if (!gizmoInUse) {
        mat4 modelRight = translate(model, ((WORLD_RIGHT * 0.5f)));
        const vec3 scaleRight = {0.5f, 0.1f, 0.1f};
        OOBB upBound = {
                .center = glm::vec3(modelRight[3]), // I don't want to decompose the whole thing
                .extents = scaleRight * vec3(glm::length(vec3(modelRight[0])), glm::length(vec3(modelRight[1])), glm::length(vec3(modelRight[2]))),
                .orientation = glm::quat(transform.rotation),
        };
        glm::vec3 intersectionPoint;
        if (ray_oob_intersection(ray, upBound, tMin, intersectionPoint)) {
            rightHovered = true;
            rightIsSelected = input_mouse_pressed(MouseButton::Left);
        }
        draw_box(modelRight, scaleRight, BOX_GREEN_RGB, 1);
    }

    gizmo_translate_object(transform, ray, WORLD_UP, upIsSelected, gizmoInUse);
    gizmo_translate_object(transform, ray, WORLD_RIGHT, rightIsSelected, gizmoInUse);
    gizmo_translate_object(transform, ray, WORLD_FORWARD, forwardIsSelected, gizmoInUse);

    draw_box(glm::translate(glm::mat4(1.0f), transform.position) * glm::toMat4(glm::quat(transform.rotation)), transform.scale * glm::vec3(0.5f), BOX_GREEN_RGB,
             GIZMO_LINE_THICKNESS);

    LinePoint3D rayStart = {ray.origin, RGBA_WHITE};
    LinePoint3D rayEnd = {ray.origin + ray.direction * ((tMin == 0.0f) ? 1000.0f : tMin), RGBA_WHITE};
    gfx_line_add_segment_immediate(rayStart, rayEnd, GIZMO_LINE_THICKNESS);

    {
        uint32_t UP_GIZMO_COLOUR = upHovered ? RGBA_GREEN_HOVERED : RGBA_GREEN;
        UP_GIZMO_COLOUR = upIsSelected ? RGBA_WHITE : UP_GIZMO_COLOUR;
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(vec3{0.0, 0.0, 0.0}, 1)), .color = UP_GIZMO_COLOUR},
                {.position = vec3(model * vec4(WORLD_UP, 1)), .color = UP_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );

        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(vec3{0.0, 1.0, 0.0}, 1)), .color = UP_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{0.1, 0.9, 0.0}, 1)), .color = UP_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );

        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(vec3{0.0, 1.0, 0.0}, 1)), .color = UP_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{0.0, 0.9, 0.1}, 1)), .color = UP_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );

        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(vec3{0.0, 1.0, 0.0}, 1)), .color = UP_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{0.0, 0.9, -0.1}, 1)), .color = UP_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(vec3{0.0, 1.0, 0.0}, 1)), .color = UP_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{-0.1, 0.9, 0.0}, 1)), .color = UP_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
    }

    {
        uint32_t FORWARD_GIZMO_COLOUR = forwardHovered ? RGBA_BLUE_HOVERED : RGBA_BLUE;
        FORWARD_GIZMO_COLOUR = forwardIsSelected ? RGBA_WHITE : FORWARD_GIZMO_COLOUR;
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(vec3{0.0, 0.0, 0.0}, 1)), .color = FORWARD_GIZMO_COLOUR},
                {.position = vec3(model * vec4(WORLD_FORWARD, 1)), .color = FORWARD_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );

        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(WORLD_FORWARD, 1)), .color = FORWARD_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{0.0, 0.1, -0.9}, 1)), .color = FORWARD_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(WORLD_FORWARD, 1)), .color = FORWARD_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{0.0, -0.1, -0.9}, 1)), .color = FORWARD_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(WORLD_FORWARD, 1)), .color = FORWARD_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{0.1, 0.0, -0.9}, 1)), .color = FORWARD_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(WORLD_FORWARD, 1)), .color = FORWARD_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{-0.1, 0.0, -0.9}, 1)), .color = FORWARD_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
    }

    {
        uint32_t RIGHT_GIZMO_COLOUR = rightHovered ? RGBA_RED_HOVERED : RGBA_RED;
        RIGHT_GIZMO_COLOUR = rightIsSelected ? RGBA_WHITE : RIGHT_GIZMO_COLOUR;
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(vec3{0.0, 0.0, 0.0}, 1)), .color = RIGHT_GIZMO_COLOUR},
                {.position = vec3(model * vec4(WORLD_RIGHT, 1)), .color = RIGHT_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );

        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(WORLD_RIGHT, 1)), .color = RIGHT_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{-0.9, 0.1, 0.0}, 1)), .color = RIGHT_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(WORLD_RIGHT, 1)), .color = RIGHT_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{-0.9, -0.1, 0.0}, 1)), .color = RIGHT_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(WORLD_RIGHT, 1)), .color = RIGHT_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{-0.9, 0.0, 0.1}, 1)), .color = RIGHT_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
        gfx_line_add_segment_immediate(
                {.position = vec3(model * vec4(WORLD_RIGHT, 1)), .color = RIGHT_GIZMO_COLOUR},
                {.position = vec3(model * vec4(vec3{-0.9, 0.0, -0.1}, 1)), .color = RIGHT_GIZMO_COLOUR},
                GIZMO_LINE_THICKNESS
        );
    }

}
//======================================================================================================================

//===API================================================================================================================
void widget_manipulate_update(bool &enabled) {
    if (!enabled) {
        return;
    }
    widget_switch_between_manipulators();

    const SelectedPool &poolType = *get_selected_pool_type();
    const int32 &poolIndex = *get_selected_pool_index();

    if (poolType != SELECTED_POOL_NONE && poolIndex != -1) {
        Transform *currentTransform = nullptr;
        if (s_manipulator == Manipulator_Translate) {
            switch (poolType) {
                case SELECTED_POOL_LIT_ENT: {
                    const LitEntity &litEntity = *db_get_lit_entity(poolIndex);
                    currentTransform = db_get_transform(litEntity.transformIndex);
                    widget_manipulate_transform(*currentTransform);
                    break;
                }
                case SELECTED_POOL_CAMERA_ENT: {
                    const CameraEntity &camEntity = *db_get_camera_entity(poolIndex);
                    currentTransform = db_get_transform(camEntity.transformIndex);
                    widget_manipulate_transform(*currentTransform);
                    break;
                }
                case SELECTED_POOL_NONE: {
                    break;
                }
            }
        } else if (s_manipulator == Manipulator_Rotate) {
        } else if (s_manipulator == Manipulator_Scale) {
        } else if (s_manipulator == Manipulator_None) {
        } else {
            NOT_IMPLEMENTED();
        }
        //do gizmo draw
    }

}
//======================================================================================================================