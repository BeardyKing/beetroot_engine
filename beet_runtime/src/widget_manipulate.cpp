#include <runtime/widget_manipulate.h>
#include <beet_gfx/IconsFontAwesome5.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <beet_shared/defer.h>
#include <beet_core/input.h>

#include <cstdint>

enum Manipulator : uint32_t {
    Manipulator_None = 0,
    Manipulator_Translate = 1,
    Manipulator_Rotate = 2,
    Manipulator_Scale = 3,
};

static Manipulator s_manipulator = Manipulator_None;

void widget_manipulate_update(bool &enabled) {
    if (!enabled) {
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