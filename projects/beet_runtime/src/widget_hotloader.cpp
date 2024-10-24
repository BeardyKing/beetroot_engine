#include <runtime/widget_hotloader.h>
#include <beet_gfx/gfx_lit.h>
#include <beet_gfx/gfx_sky.h>
#include <imgui.h>
#include <beet_gfx/gfx_line.h>
#include "beet_gfx/gfx_triangle_strip.h"

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
        if (ImGui::Button("Reload: Lines")) {
            gfx_rebuild_line_pipeline();
        }
        if (ImGui::Button("Reload: Triangle Strip")) {
            gfx_rebuild_triangle_strip_pipeline();
        }
        ImGui::End();
    }
}
//======================================================================================================================
