#include <runtime/widget_db.h>

#include <beet_shared/beet_types.h>
#include <beet_gfx/db_asset.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <string>
#include <beet_gfx/gfx_imgui.h>
#include <beet_shared/assert.h>

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

static bool widget_draw_transform(Transform &transform) {
    bool outEdited = false;
    if (ImGui::CollapsingHeader("Transform")) {
        outEdited |= DrawVec3Control("Pos", transform.position, 0.0f, 35.0f);
        outEdited |= DrawVec3Control("Rot", transform.rotation, 0.0f, 35.0f);
        outEdited |= DrawVec3Control("Scl", transform.scale, 0.0f, 35.0f);
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
