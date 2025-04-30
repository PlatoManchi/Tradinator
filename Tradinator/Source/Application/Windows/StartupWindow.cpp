#include "Application/Windows/StartupWindow.h"

#include <string>
#include <format>

#include "imgui.h"

#include "Utils.h"
#include "Utils/Utils.h"

#include "Application/TradinatorSettings.h"

StartupWindow::StartupWindow()
    : m_working_folder_exists(false)
    , m_nse_market_security_list_exist(false)
    , m_all_settings_valid(false)
{
    std::string working_path = TradinatorSettings::Get().GetWorkingFolder();
    std::copy(working_path.begin(), working_path.end(), m_working_folder_path);
}

void StartupWindow::Init()
{

}

void StartupWindow::Begin()
{

}


bool StartupWindow::Show()
{
    ImVec2 work_pos = ImGui::GetMainViewport()->WorkPos;
    ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;

    ImGui::SetNextWindowPos(work_pos);
    ImGui::SetNextWindowSize(work_size);

    ImGuiStyle& style = ImGui::GetStyle();

    if (ImGui::Begin("Tradinator Start Up", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
    {
        /// @separator

        // TODO: Add Draw calls of dependent popup windows here

        /// @begin Text
        std::string setting_working_folder_str = "Setting up working folder";
        ImVec2 setting_working_folder_size = ImGui::CalcTextSize(setting_working_folder_str.c_str());
        ImVec2 prev_cursor_pos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(work_size.x / 2.0f - setting_working_folder_size.x/2.0f, prev_cursor_pos.y));
        ImGui::TextUnformatted(setting_working_folder_str.c_str());
        /// @end Text

        /// @begin Separator
        ImGui::Separator();
        /// @end Separator

        
        m_working_folder_exists = TradinatorAppSpace::Utils::DoesDirectoryExist(m_working_folder_path);

        if (!m_working_folder_exists)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(237, 67, 55, 255)); // Blue color
        }

        /// @begin Text
        ImGui::TextUnformatted("Working Folder:");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The folder that is used by the app to keep temporary files and database. Needs both read and write access.");
        /// @end Text

        /// @begin Input
        ImGui::SetNextItemWidth(work_size.x - style.ItemSpacing.x* 2.0);
        ImGui::InputText("##working_folder_path", m_working_folder_path, 512, ImGuiInputTextFlags_EnterReturnsTrue);
        if (!m_working_folder_exists)
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The folder that is used by the app to keep temporary files and database. Needs both read and write access.");
        /// @end Input

        /// @begin Separator
        ImGui::Separator();
        /// @end Separator

        if (!m_working_folder_exists)
        {
            ImGui::BeginDisabled(true);
        }

        std::string setting_markets_str = "Setting up Markets";
        ImVec2 setting_markets_size = ImGui::CalcTextSize(setting_markets_str.c_str());
        prev_cursor_pos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(work_size.x / 2.0f - setting_markets_size.x / 2.0f, prev_cursor_pos.y));
        ImGui::TextUnformatted(setting_markets_str.c_str());

        /// @begin Separator
        ImGui::Separator();
        /// @end Separator

        std::string nse_securities_file_name = std::format("{}/NSE/EQUITY_L.csv", m_working_folder_path);
        m_nse_market_security_list_exist = TradinatorCoreSpace::Utils::DoesFileExist(nse_securities_file_name);

        /// @begin Text
        ImGui::TextUnformatted("NSE (National Stock Exchange, India) : ");
        /// @end Text

        /// @begin CheckBox
        ImGui::Checkbox("##IsNSEMarketValid", &m_nse_market_security_list_exist);
        /// @end CheckBox

        /// @begin Text
        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
        ImGui::PushTextWrapPos(0);
        ImGui::TextUnformatted(std::format("Download from security list for NSE market from 'https://nsearchives.nseindia.com/content/equities/EQUITY_L.csv' and place the file at {}.", nse_securities_file_name).c_str());
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
            TradinatorAppSpace::Utils::OpenURL("https://nsearchives.nseindia.com/content/equities/EQUITY_L.csv");
        }
        ImGui::PopTextWrapPos();
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(7);
        /// @end Text

        if (!m_working_folder_exists)
        {
            ImGui::EndDisabled();
        }

        /// @begin Separator
        ImGui::Separator();
        /// @end Separator



        m_all_settings_valid = m_working_folder_exists && m_nse_market_security_list_exist;
        
        /// @begin Button
        if (!m_all_settings_valid)
        {
            ImGui::BeginDisabled(true);
        }
        
        ImGui::SetCursorPos(ImVec2(work_size.x / 2.0f - m_save_settings_button_size.x / 2.0f, work_size.y - m_save_settings_button_size.y - style.FramePadding.y * 3.0));
        if (ImGui::Button("Save Settings", { 0, 0 }))
        {
            TradinatorSettings::Get().SetWorkingFolder(m_working_folder_path);
        }
        m_save_settings_button_size = ImGui::GetItemRectSize();
        if (!m_all_settings_valid)
        {
            ImGui::EndDisabled();
        }
        /// @end Button
    }

    ImGui::End();

    return TradinatorSettings::Get().IsValid();
}

void StartupWindow::Shutdown()
{

}