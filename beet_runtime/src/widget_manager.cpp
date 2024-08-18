#include <runtime/widget_manager.h>
#include <runtime/widget_db.h>
#include <runtime/widget_hotloader.h>
#include <runtime/widget_manipulate.h>

#include <beet_shared/c_string.h>
#include <runtime/widget_memory.h>
#include <beet_gfx/gfx_imgui.h>

#include <imgui.h>

//===INTERNAL_STRUCTS===================================================================================================
enum class WidgetType : uint32_t {
    ACTIVE_WIDGETS_MENU = 0,

    SHADER_HOT_RELOAD_MENU,
    MEMORY_POOL_MENU,

    TOOLBAR_NAVIGATION_MENU,
    DB_POOL_MENU,
    MANIPULATOR_MENU_AND_GIZMOS,

    IMGUI_DEMO_MENU,

    COUNT
};

struct WidgetInfo {
    WidgetType type;
    bool isActive = true;
    const char name[64] = {};
    const char toolbarTabName[64] = {};
};

static WidgetInfo s_widgets[uint32_t(WidgetType::COUNT)] = {
        {.type = WidgetType::ACTIVE_WIDGETS_MENU, .isActive = false, .name = "Active widgets menu", .toolbarTabName = "Editor"},

        {.type = WidgetType::SHADER_HOT_RELOAD_MENU, .isActive = false, .name = "Shader hot reload", .toolbarTabName = "Debug Tools"},
        {.type = WidgetType::MEMORY_POOL_MENU, .isActive = false, .name = "Memory pool inspector", .toolbarTabName = "Debug Tools"},

        {.type = WidgetType::TOOLBAR_NAVIGATION_MENU, .isActive = true, .name = "Navigation bar"},
        {.type = WidgetType::DB_POOL_MENU, .isActive = true, .name = "DB pool inspector"},
        {.type = WidgetType::MANIPULATOR_MENU_AND_GIZMOS, .isActive = true, .name = "Manipulate menu and gizmos"},

        {.type = WidgetType::IMGUI_DEMO_MENU, .isActive = false, .name = "Imgui Demo menu", .toolbarTabName = "Debug Tools"},
};
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static void widget_toolbar_update() {
    WidgetInfo &navigationMenu = s_widgets[uint32_t(WidgetType::TOOLBAR_NAVIGATION_MENU)];
    if (navigationMenu.isActive && ImGui::BeginMainMenuBar()) {
        for (uint32_t i = 0; i < uint32_t(WidgetType::COUNT); ++i) {
            WidgetInfo &info = s_widgets[i];
            if (!c_str_empty(info.toolbarTabName)) {
                if (ImGui::BeginMenu(info.toolbarTabName)) {
                    ImGui::MenuItem(info.name, "", &info.isActive);
                    ImGui::EndMenu();
                }
            }
        }
        ImGui::EndMainMenuBar();
    }
}

static void widget_state_update() {
    WidgetInfo &menuWidget = s_widgets[uint32_t(WidgetType::ACTIVE_WIDGETS_MENU)];
    if (menuWidget.isActive) {
        ImGui::SetNextWindowSize(ImVec2(275, 200), ImGuiCond_FirstUseEver);
        ImGui::Begin(menuWidget.name, &menuWidget.isActive, ImGuiWindowFlags_None);

        for (uint32_t i = 0; i < uint32_t(WidgetType::COUNT); ++i) {
            WidgetInfo &currentWidget = s_widgets[i];
            ImGui::Checkbox(currentWidget.name, &currentWidget.isActive);
        }

        ImGui::End();
    }
}

static void widget_imgui_demo_update(bool &active) {
    if (active) {
        gfx_imgui_demo_window();
    }
}
//======================================================================================================================

//===API================================================================================================================
void widget_manager_update() {
    widget_toolbar_update();
    widget_state_update();

    widget_db_update(s_widgets[uint32_t(WidgetType::DB_POOL_MENU)].isActive);
    widget_manipulate_update(s_widgets[uint32_t(WidgetType::MANIPULATOR_MENU_AND_GIZMOS)].isActive);
    widget_memory_update(s_widgets[uint32_t(WidgetType::MEMORY_POOL_MENU)].isActive);
    widget_hot_reload_shaders(s_widgets[uint32_t(WidgetType::SHADER_HOT_RELOAD_MENU)].isActive);
    widget_imgui_demo_update(s_widgets[uint32_t(WidgetType::IMGUI_DEMO_MENU)].isActive);
}
//======================================================================================================================
