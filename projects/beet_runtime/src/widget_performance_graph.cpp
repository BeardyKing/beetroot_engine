#include <runtime/widget_performance_graph.h>
#include <beet_core/time.h>
#include <cstdint>

#include "imgui.h"

constexpr uint32_t PLOT_ARRAY_SIZE = 256;
static float s_plotArrayCPU[PLOT_ARRAY_SIZE] = {};
static float s_rollingAverageCPU = 0;
constexpr float ROLLING_AVERAGE_ALPHA = 0.005f;

//===INTERNAL_FUNCTIONS=================================================================================================
void performance_graph_view(bool &enabled) {
    ImGui::Begin("Frame graph", &enabled);
    {
        const float current = 1000.0f * time_delta_f();
        s_rollingAverageCPU = ROLLING_AVERAGE_ALPHA * current + (1.0f - ROLLING_AVERAGE_ALPHA) * s_rollingAverageCPU;
        s_plotArrayCPU[PLOT_ARRAY_SIZE - 1] = current;
        memcpy(s_plotArrayCPU, s_plotArrayCPU + 1, sizeof(s_plotArrayCPU) - sizeof(float));
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.1f, 1.0f, 0.1f, 1.0f));
        ImGui::PlotLines("##CPU: ", s_plotArrayCPU, PLOT_ARRAY_SIZE, 0, nullptr, 0, 32, ImVec2(ImGui::GetContentRegionAvail().x, 16), sizeof(float));
        ImGui::PopStyleColor();
        ImGui::Text("CPU - %.2f ms (avg %.2fms)", current, s_rollingAverageCPU);
    }
    ImGui::End();
}
//======================================================================================================================

//===API================================================================================================================
void widget_performance_graph_update(bool &enabled) {
    if (enabled) {
        performance_graph_view(enabled);
    }
}
//======================================================================================================================

