#include <runtime/widget_memory.h>
#include <beet_shared/memory.h>
#include <beet_gfx/db_asset.h>

#include <imgui.h>

#include <cstdint>

//===INTERNAL_FUNCTIONS=================================================================================================
void pool_memory_view(bool &enabled) {
    if (enabled) {
        uint32_t allocationTableEntriesSize = {};
        PoolAllocEntry *allocationTableEntries = db_get_allocation_table(allocationTableEntriesSize);
        ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
        ImGui::Begin("Memory pool inspector", &enabled);
        {
            const ImGuiTableFlags tableFlags =
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Reorderable;
            if (ImGui::BeginTable("PoolAllocTable", 6, tableFlags)) {
                ImGui::TableSetupColumn("Pool Name", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Item Size (bytes)", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Items Allocated", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Pool Filled (%)", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Current Memory Usage (bytes)", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Total Pool Size (bytes)", ImGuiTableColumnFlags_None);
                ImGui::TableHeadersRow();

                size_t totalUsedPoolMemoryBytes = 0;
                size_t totalPoolSizeBytes = 0;

                for (uint32_t i = 0; i < allocationTableEntriesSize; ++i) {
                    PoolAllocEntry &entry = allocationTableEntries[i];
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", entry.allocInfo.poolName);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%zu", entry.allocInfo.itemSize);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%u / %zu", entry.poolInfo->count, entry.allocInfo.itemCount);

                    ImGui::TableSetColumnIndex(3);
                    float percentageFilled = (float) (entry.poolInfo->count) / entry.allocInfo.itemCount * 100.0f;
                    ImGui::Text("%.2f%%", percentageFilled);

                    ImGui::TableSetColumnIndex(4);
                    size_t currentMemoryUsage = entry.poolInfo->count * entry.allocInfo.itemSize;
                    ImGui::Text("%zu", currentMemoryUsage);

                    ImGui::TableSetColumnIndex(5);
                    size_t currentPoolSize = entry.allocInfo.itemCount * entry.allocInfo.itemSize;
                    ImGui::Text("%zu", currentPoolSize);

                    totalUsedPoolMemoryBytes += currentMemoryUsage;
                    totalPoolSizeBytes += currentPoolSize;
                }

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Total");

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("NA");

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("NA");

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("NA");

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%zu", totalUsedPoolMemoryBytes);

                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%zu", totalPoolSizeBytes);

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}
//======================================================================================================================

//===API================================================================================================================
void widget_memory_update(bool &enabled) {
    if (enabled) {
        pool_memory_view(enabled);
    }
}
//======================================================================================================================

