#include <runtime/widget_manager.h>
#include <runtime/widget_db.h>
#include <runtime/widget_hotloader.h>
#include <runtime/widget_manipulate.h>

#include <imgui.h>

//===INTERNAL_STRUCTS===================================================================================================
static struct WidgetState {
    bool mainMenuActive = true;
    bool infoActive = false;
    bool DBActive = true;
    bool shaderHotLoader = true;
    bool manipulatorActive = true;
} s_widgetState;
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static void widget_toolbar_update() {
    if (ImGui::BeginMainMenuBar() && s_widgetState.mainMenuActive) {
        if (ImGui::BeginMenu("Editor")) {
            ImGui::MenuItem("Editor Widget Info", "", &s_widgetState.infoActive);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug Tools")) {
            ImGui::MenuItem("Hot-Reload: Shaders", "", &s_widgetState.shaderHotLoader);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static void widget_state_update() {
    if (s_widgetState.infoActive) {
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Widget Panel", &s_widgetState.infoActive, ImGuiWindowFlags_None);
        {
            ImGui::Checkbox("DB widget", &s_widgetState.DBActive);
            ImGui::Checkbox("Manipulator widget", &s_widgetState.manipulatorActive);
        }
        ImGui::End();
    }
}
//======================================================================================================================

//===API================================================================================================================
void widget_manager_update() {
    widget_toolbar_update();
    widget_state_update();

    widget_db_update(s_widgetState.DBActive);
    widget_manipulate_update(s_widgetState.manipulatorActive);
    widget_hot_reload_shaders(s_widgetState.shaderHotLoader);
}
//======================================================================================================================
