#include  "SearchBar.h"

#include <string>

#include "imgui.h"

void SearchBar::Init()
{
}

void SearchBar::Begin()
{
}

void SearchBar::Show()
{
    ImGuiWindowFlags no_decoration =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoDecoration;
    
    char* search_symbol = new char();

    if (ImGui::Begin("SearchBar", nullptr, no_decoration))
    {

        if (ImGui::BeginTable("table1", 2, ImGuiTableFlags_NoBordersInBody, { -1, 0 }))
        {
            ImGui::TableSetupColumn("A", ImGuiTableColumnFlags_WidthFixed, 0);
            ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_None, 0);
            //ImGui::TableSetupColumn("C", ImGuiTableColumnFlags_None, 0);
            //ImGui::TableHeadersRow();
            ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);
            ImGui::TableSetColumnIndex(0);
            
            /// @separator

            /// @begin Text
            ImGuiStyle& style = ImGui::GetStyle();
            style.SelectableTextAlign = ImVec2(1.0f, 0.5f);
            ImGui::Selectable("Search: ", false, ImGuiSelectableFlags_Disabled);
            /// @end Text

            /// @begin Input
            ImGui::TableSetColumnIndex(1);
            //ImGui::NextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##value3", search_symbol, 256, ImGuiInputTextFlags_None);
            
            /// @end Input


            /// @separator
            ImGui::EndTable();
        }

        
    }
    ImGui::End();
}

void SearchBar::Shutdown()
{
}
