#include <beet_core/window.h>
#include <beet_core/time.h>
#include <beet_core/input.h>
#include <beet_shared/log.h>
#include <beet_gfx/gfx_interface.h>

#include <beet_gfx/gfx_imgui.h>
#include <imgui.h>

#if BEET_GFX_IMGUI
void imgui_update() {
    //TODO: setup the rest of the imgui kb/mouse mappings.
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[ImGuiMouseButton_Left] = input_mouse_down(MouseButton::Left);

    gfx_imgui_begin();
    {
        gfx_imgui_demo_window();
        bool isHovered = ImGui::IsItemHovered();
        bool isFocused = ImGui::IsItemFocused();
        ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
        ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
        ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);
        ImGui::Text("Is mouse over screen? %s", isHovered ? "Yes" : "No");
        ImGui::Text("Is screen focused? %s", isFocused ? "Yes" : "No");
        ImGui::Text("Position: %f, %f", mousePositionRelative.x, mousePositionRelative.y);
        ImGui::Text("Mouse clicked: %s", ImGui::IsMouseDown(ImGuiMouseButton_Left) ? "Yes" : "No");
    }
}
#endif // BEET_GFX_IMGUI

int main() {
    window_create("beetroot engine - runtime", {1024, 769});
    time_create();
    input_create();
    gfx_create(window_get_handle());

    log_info(MSG_RUNTIME, "hello beetroot engine\n");
    while (window_is_open()) {
        time_tick();
        input_set_time(time_current());
        window_update();
        input_update();
#if BEET_GFX_IMGUI
        imgui_update();
#endif //BEET_GFX_IMGUI
        gfx_update(time_delta());
    }

    gfx_cleanup();
    input_cleanup();
    time_cleanup();
    window_cleanup();
}