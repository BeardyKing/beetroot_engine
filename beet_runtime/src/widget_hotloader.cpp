#include <runtime/widget_hotloader.h>
#include <beet_gfx/gfx_lit.h>
#include <beet_gfx/gfx_sky.h>
#include <imgui.h>

//===API================================================================================================================
void widget_hot_reload_shaders(bool &enabled) {
    if (enabled) {
        ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_Always);
        ImGui::Begin("Hot-Reload: Shaders", &enabled);
        if (ImGui::Button("Reload: Lit")) {
            gfx_rebuild_lit_pipeline();
        }
        if (ImGui::Button("Reload: Sky")) {
            gfx_rebuild_sky_pipeline();
        }
        ImGui::End();
    }
}
//======================================================================================================================
