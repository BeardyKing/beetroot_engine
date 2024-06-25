#include <runtime/widget_db.h>

#include <beet_core/input.h>
#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>
#include <beet_math/quat.h>
#include <beet_math/utilities.h>
#include <beet_gfx/db_asset.h>
#include <beet_gfx/gfx_imgui.h>
#include <beet_gfx/gfx_line.h>
#include <beet_gfx/gfx_interface.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <string>

//===INTERNAL_STRUCTS===================================================================================================
static enum SelectedPool {
    SELECTED_POOL_NONE = -1,
    SELECTED_POOL_LIT_ENT = 0,
    SELECTED_POOL_CAMERA_ENT = 1,
} s_selectedPool = {SELECTED_POOL_NONE};

static int32_t s_selectedPoolItem = -1;
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static bool DrawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f, float columnWidth = 100.0f, float minDrag = 0.1f) {
    bool outEdited = false;

    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize))
        values.x = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    outEdited |= ImGui::DragFloat("##X", &values.x, minDrag, 0.0f, 0.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize))
        values.y = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    outEdited |= ImGui::DragFloat("##Y", &values.y, minDrag, 0.0f, 0.0f, "%.3f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize))
        values.z = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    outEdited |= ImGui::DragFloat("##Z", &values.z, minDrag, 0.0f, 0.0f, "%.3f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();

    ImGui::Columns(1);

    ImGui::PopID();
    return outEdited;
}

static void widget_pool_selector_lit_entity() {
    const uint32_t litEntityCount = db_get_lit_entity_count();
    char litEntTitleBuf[128] = {};
    char litEntName[DEBUG_NAME_MAX] = {};
    sprintf(litEntTitleBuf, "Pool: LitEntity [%u]", litEntityCount);
    if (ImGui::CollapsingHeader(litEntTitleBuf)) {
        for (int32_t poolIndex = 0; poolIndex < litEntityCount; ++poolIndex) {
            sprintf(litEntName, "Name: \"%s\" - %u", BEET_DEBUG ? db_get_lit_entity(poolIndex)->debug_name : "", poolIndex);
            if (ImGui::Selectable(litEntName, s_selectedPoolItem == poolIndex)) {
                s_selectedPoolItem = poolIndex;
                s_selectedPool = SELECTED_POOL_LIT_ENT;
            }
        }
    }
}

static void widget_pool_selector_camera_entity() {
    const uint32_t camEntityCount = db_get_camera_entity_count();
    char litEntTitleBuf[128] = {};
    char litEntName[DEBUG_NAME_MAX] = {};
    sprintf(litEntTitleBuf, "Pool: CameraEntity [%u]", camEntityCount);
    if (ImGui::CollapsingHeader(litEntTitleBuf)) {
        for (int32_t poolIndex = 0; poolIndex < camEntityCount; ++poolIndex) {
            sprintf(litEntName, "Name: \"%s\" - %u", BEET_DEBUG ? db_get_camera_entity(poolIndex)->debug_name : "", poolIndex);
            if (ImGui::Selectable(litEntName, s_selectedPoolItem == poolIndex)) {
                s_selectedPoolItem = poolIndex;
                s_selectedPool = SELECTED_POOL_CAMERA_ENT;
            }
        }
    }
}

static void widget_pool_selector(bool &enabled) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    ImGui::Begin("Pool Selector", &enabled);
    {
        widget_pool_selector_lit_entity();
        widget_pool_selector_camera_entity();
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

glm::vec3 closest_point_on_line_from_ray(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection, const glm::vec3 &linePoint, const glm::vec3 &lineDirection) {
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

static bool widget_draw_transform(Transform &transform) {
    bool outEdited = false;
    if (ImGui::CollapsingHeader("Transform")) {
        outEdited |= DrawVec3Control("Pos", transform.position, 0.0f, 35.0f);
        outEdited |= DrawVec3Control("Rot", transform.rotation, 0.0f, 35.0f);
        outEdited |= DrawVec3Control("Scl", transform.scale, 0.0f, 35.0f);
    }


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

    vec2i screenSize = gfx_screen_size();
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
            draw_box(modelForward, scaleForward, RGBA_GREEN, 1);
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
        draw_box(modelUp, scaleUp, RGBA_GREEN, 1);
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
        draw_box(modelRight, scaleRight, RGBA_GREEN, 1);
    }

    gizmo_translate_object(transform, ray, WORLD_UP, upIsSelected, gizmoInUse);
    gizmo_translate_object(transform, ray, WORLD_RIGHT, rightIsSelected, gizmoInUse);
    gizmo_translate_object(transform, ray, WORLD_FORWARD, forwardIsSelected, gizmoInUse);

    draw_box(glm::translate(glm::mat4(1.0f), transform.position) * glm::toMat4(glm::quat(transform.rotation)), transform.scale * glm::vec3(0.5f), RGBA_GREEN, GIZMO_LINE_THICKNESS);

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

    return outEdited;
}

static void widget_pool_inspector_lit_entity() {
    ASSERT(s_selectedPoolItem < db_get_lit_entity_count());
    const LitEntity &litEntity = *db_get_lit_entity(s_selectedPoolItem);

    if (ImGui::CollapsingHeader("Lit Entity Info")) {
        if (ImGui::BeginTable("GridTable", 2, ImGuiTableFlags_Borders)) {
#if BEET_DEBUG
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Debug Name");
                ImGui::TableNextColumn();
                ImGui::Text("%s", litEntity.debug_name);
            }
#endif //BEET_DEBUG
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Transform Index");
                ImGui::TableNextColumn();
                ImGui::Text("%u", litEntity.transformIndex);
            }
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Mesh Index");
                ImGui::TableNextColumn();
                ImGui::Text("%u", litEntity.meshIndex);
            }
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Material Index");
                ImGui::TableNextColumn();
                ImGui::Text("%u", litEntity.materialIndex);
            }
            // End table layout
            ImGui::EndTable();
        }
    }
    // we don't need to do anything with this edited state right now, but when we move transform buffers to the GPU
    // we will want to dispatch a GPU copy.
    widget_draw_transform(*db_get_transform(litEntity.transformIndex));
}

static bool widget_draw_transform(Camera &camera) {
    constexpr float FOV_DRAG_AMOUNT = 0.5f;
    constexpr float Z_DRAG_AMOUNT = 1.0f;
    constexpr float Z_MIN = 0.1f;
    constexpr float Z_MAX = 100000.0f;

    bool outEdited = false;
    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Text("fov  ");
        ImGui::SameLine();
        outEdited |= ImGui::DragFloat("##fov", &camera.fov, FOV_DRAG_AMOUNT, 0.0f, 0.0f, "%.3f");
        camera.fov = clamp(camera.fov, 0.1f, 179.9f);

        ImGui::Text("zNear");
        ImGui::SameLine();
        outEdited |= ImGui::DragFloat("##zNear", &camera.zNear, Z_DRAG_AMOUNT, 0.0f, 0.0f, "%.1f");
        camera.zNear = clamp(camera.zNear, Z_MIN, Z_MAX);

        ImGui::Text("zFar ");
        ImGui::SameLine();
        outEdited |= ImGui::DragFloat("##zFar", &camera.zFar, Z_DRAG_AMOUNT, 0.0f, 0.0f, "%.1f");
        camera.zFar = clamp(camera.zFar, Z_MIN, Z_MAX);
    }
    return outEdited;
}

static void widget_pool_inspector_camera_entity() {
    ASSERT(s_selectedPoolItem < db_get_camera_entity_count());
    const CameraEntity &camEntity = *db_get_camera_entity(s_selectedPoolItem);

    widget_draw_transform(*db_get_transform(camEntity.transformIndex));
    widget_draw_transform(*db_get_camera(camEntity.cameraIndex));
}

static void widget_pool_inspector(bool &enabled) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    ImGui::Begin("Pool Inspector", &enabled);

    switch (s_selectedPool) {
        case SELECTED_POOL_LIT_ENT:
            widget_pool_inspector_lit_entity();
            break;
        case SELECTED_POOL_CAMERA_ENT:
            widget_pool_inspector_camera_entity();
            break;
        case SELECTED_POOL_NONE:
        default:
            break;
    }

    ImGui::End();
}

//======================================================================================================================

//===API================================================================================================================
void widget_db_update(bool &enabled) {
    if (enabled) {
        widget_pool_selector(enabled);
        widget_pool_inspector(enabled);
        gfx_imgui_demo_window();
    }
}
//======================================================================================================================
