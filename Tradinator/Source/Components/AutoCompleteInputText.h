#pragma once

#include <string>
#include  <vector>

#include "imgui.h"

class AutoCompleteInputText
{
public:
    bool Draw(std::string id, std::string text, std::vector<std::string> items)
    {
        ImGuiID inputId = ImGui::GetID(id.c_str());
        bool isSearchResultWindowOpen = (inputId == _activeInputId);

        if (isSearchResultWindowOpen)
        {
            if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_DownArrow, true))
            {
                if (_lastTypeAheadResults.size() > 0)
                {
                    _selectedResultIndex++;
                    _selectedResultIndex %= _lastTypeAheadResults.size();
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_UpArrow, true))
            {
                if (_lastTypeAheadResults.size() > 0)
                {
                    _selectedResultIndex--;
                    if (_selectedResultIndex < 0)
                        _selectedResultIndex = _lastTypeAheadResults.size() - 1;
                }
            }
        }

        //char* search_symbol = new char();
        char search_symbol[128] = "";
        bool wasChanged = ImGui::InputTextWithHint(id.c_str(), "Search for company", search_symbol, IM_ARRAYSIZE(search_symbol), ImGuiInputTextFlags_None);
        //var wasChanged = ImGui.InputText(id, ref text, 256);

        if (ImGui::IsItemActivated())
        {
            _lastTypeAheadResults.clear();
            _selectedResultIndex = -1;
            ImGui::SetNavCursorVisible(false);
            //THelpers.DisableImGuiKeyboardNavigation();
        }

        bool isItemDeactivated = ImGui::IsItemDeactivated();

        // We defer exit to get clicks on opened popup list
        bool lostFocus = isItemDeactivated || ImGui::IsKeyDown(ImGuiKey_Escape);

        if (ImGui::IsItemActive() || isSearchResultWindowOpen)
        {
            _activeInputId = inputId;

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetItemRectSize().x, 0));

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7.0f, 7.0f));

            if (ImGui::Begin("##typeAheadSearchPopup", &isSearchResultWindowOpen,
                ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_Tooltip
                | ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_ChildWindow
                | ImGuiWindowFlags_AlwaysAutoResize
            ))
            {
                _lastTypeAheadResults.clear();

                int index = 0;
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.24f, 0.24f, 1.0f));
                for(std::string& word : items)
                {
                    int str_find_index = word.find(search_symbol);
                    if (str_find_index != -1)
                    {
                        bool isSelected = index == _selectedResultIndex;
                        std::string tmp = word +"\n  Symbol: SYNB\n  ISIN Number: IN340";
                        ImGui::Selectable(tmp.c_str(), isSelected);
                        if (ImGui::IsItemClicked() || (isSelected && ImGui::IsKeyPressed(ImGuiKey_Enter)))
                        {
                            text = word;
                            wasChanged = true;
                            _activeInputId = 0;
                            //isSearchResultWindowOpen = false;
                        }

                        _lastTypeAheadResults.push_back(word);
                        if (++index > 30)
                            break;
                    }
                }
                ImGui::PopStyleColor();

                if (_lastTypeAheadResults.size() == 0)
                {
                    ImGui::TextWrapped("0 items found.");
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
        }

        if (lostFocus)
        {
            ImGui::SetNavCursorVisible(true);
            //THelpers.RestoreImGuiKeyboardNavigation();
            _activeInputId = 0;
            //isSearchResultWindowOpen = false;
        }

        return wasChanged;
    }

private:
    std::vector<std::string> _lastTypeAheadResults;
    int _selectedResultIndex = 0;
    unsigned int _activeInputId;
    //private static bool _isSearchResultWindowOpen;
};

