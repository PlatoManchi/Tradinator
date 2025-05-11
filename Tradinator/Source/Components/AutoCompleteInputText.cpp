#include "Components/AutoCompleteInputText.h"

#include <regex>

#include "imgui_markdown/imgui_markdown.h"


AutoCompleteInputText::AutoCompleteInputText()
    : m_is_initialized(false)
{

}

void AutoCompleteInputText::Init(std::vector<std::shared_ptr<IAutoCommpleteItem>>&& items)
{
	m_items = std::move(items);
    m_is_initialized = true;
}

std::shared_ptr<IAutoCommpleteItem> AutoCompleteInputText::Show(const char* id, const char* label, const char* hint)
{
    std::shared_ptr<IAutoCommpleteItem> selected_item = nullptr;

    ImGuiID inputId = ImGui::GetID(id);
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

    char search_symbol[128] = "";
    ImGui::PushItemWidth(-FLT_MIN);
    bool wasChanged = ImGui::InputTextWithHint(label, hint, search_symbol, IM_ARRAYSIZE(search_symbol), ImGuiInputTextFlags_None);
    ImGui::PopItemWidth();
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

        ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
        float max_height = work_size.y * 0.6 < 500 ? 500 : work_size.y * 0.6;

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
        //ImGui::SetNextWindowSize(ImVec2(ImGui::GetItemRectSize().x, 0));
        ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetItemRectSize().x, 0.0f), ImVec2(ImGui::GetItemRectSize().x, max_height));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7.0f, 7.0f));

        if (ImGui::Begin("##typeAheadSearchPopup", &isSearchResultWindowOpen,
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_Tooltip
            | ImGuiWindowFlags_NoFocusOnAppearing
            //| ImGuiWindowFlags_NoBringToFrontOnFocus
            //| ImGuiWindowFlags_ChildWindow
            //| ImGuiWindowFlags_AlwaysVerticalScrollbar
            | ImGuiWindowFlags_AlwaysAutoResize
        ))
        {
            _lastTypeAheadResults.clear();

            int index = 0;
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.24f, 0.24f, 1.0f));
            for (std::shared_ptr<IAutoCommpleteItem> item : m_items)
            {
                if (item->CanPassFilter(search_symbol))
                {
                    bool isSelected = index == _selectedResultIndex;
                    /*ImVec2 label_size = ImGui::CalcTextSize(item->GetItemDescription().c_str(), NULL, true);
                    std::string id = "##" + item->GetItemDescription();
                    ImVec2 pos = ImGui::GetCursorPos();*/
                    ImGui::Selectable(item->GetItemDescription().c_str(), isSelected, ImGuiSelectableFlags_NoAutoClosePopups/*, ImVec2(0, label_size.y)*/);
                    if (ImGui::IsItemClicked() || (isSelected && ImGui::IsKeyPressed(ImGuiKey_Enter)))
                    {
                        selected_item = item;
                    }
                    
                    /*
                    // Can do this to decorate the item more if desired
                    ImGui::SetItemAllowOverlap();

                    ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
                    std::string search_symbol_str = std::string(search_symbol);
                    std::string markdown = item->GetItemDescription();
                    if (search_symbol_str.size() > 0)
                    {
                        std::string markdown_replacement = "**" + std::string(search_symbol) + "**";
                        markdown = std::regex_replace(item->GetItemDescription(), std::regex(search_symbol), markdown_replacement);
                    }
                    
                    ImGui::MarkdownConfig config;
                    ImGui::Markdown(markdown.c_str(), markdown.size(), config);*/

                    _lastTypeAheadResults.push_back(item);

                    ++index;
                    // limit max items to 30
                    //if (++index > 30)
                    //    break;
                }
            }
            ImGui::PopStyleColor();

            if (_lastTypeAheadResults.size() == 0)
            {
                ImGui::TextWrapped("0 items found.");
            }
        }

        ImGui::End/*Child*/();
        ImGui::PopStyleVar();
    }

    if (lostFocus)
    {
        ImGui::SetNavCursorVisible(true);
        //THelpers.RestoreImGuiKeyboardNavigation();
        _activeInputId = 0;
        //isSearchResultWindowOpen = false;
    }

    return selected_item;
}