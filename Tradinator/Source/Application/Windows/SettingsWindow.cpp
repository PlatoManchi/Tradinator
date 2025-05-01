#include "Application/Windows/SettingsWindow.h"

#include <format>


SettingsWindow::SettingsWindow()
    : m_max_parallel_downloads(100)
    , m_max_parallel_process(100)
{

}


void SettingsWindow::Init()
{

}

void SettingsWindow::Begin()
{

}

bool SettingsWindow::Show()
{
    bool should_show = true;

    ImVec2 work_pos = ImGui::GetMainViewport()->WorkPos;
    ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;

    ImGui::SetNextWindowPos(work_pos);
    ImGui::SetNextWindowSize(work_size);

    ImGuiStyle& style = ImGui::GetStyle();

    ImVec2 settings_size = ImVec2(work_size.x - style.FramePadding.x * 4.0, work_size.y - m_save_settings_button_size.y - style.FramePadding.y * 8.0);

    if (ImGui::Begin("TradinatorSettings", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
    {
        /// @begin Child
        ImGui::BeginChild("child1", settings_size, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings);
        {
            /// @separator

            /// @begin Button
            ImGui::Button("Delete Temporary Files", { 0, 0 });
            /// @end Button

            /// @begin Button
            ImGui::Button("Delete Log Files", { 0, 0 });
            /// @end Button

            /// @begin Input
            ImGui::SetNextItemWidth(200);
            std::string max_parallel_downloads_str = std::format("{}", m_max_parallel_downloads);
            char max_parallel_downloads[10] = "";
            std::copy(max_parallel_downloads_str.begin(), max_parallel_downloads_str.end(), max_parallel_downloads);
            if (ImGui::InputText("Max Parallel Downloads", max_parallel_downloads, 10, ImGuiInputTextFlags_CharsDecimal))
            {
                m_max_parallel_downloads = std::atoi(max_parallel_downloads);
            }
            /// @end Input

            /// @begin Input
            ImGui::SetNextItemWidth(200);
            std::string max_parallel_process_str = std::format("{}", m_max_parallel_process);
            char max_parallel_process[10] = "";
            std::copy(max_parallel_process_str.begin(), max_parallel_process_str.end(), max_parallel_process);
            if (ImGui::InputText("Max Parallel Process", max_parallel_process, 10, ImGuiInputTextFlags_CharsDecimal))
            {
                m_max_parallel_process = std::atoi(max_parallel_process);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                ImGui::SetTooltip("While looking up for stratiges and patterns how many parallel processes. More number = Higher ram and CPU usage.");
            /// @end Input

            /// @separator
            ImGui::EndChild();
        }
        /// @end Child

        /// @begin Button
        ImGui::SetCursorPos(ImVec2(work_size.x / 2.0f - m_save_settings_button_size.x / 2.0f, work_size.y - m_save_settings_button_size.y - style.FramePadding.y * 3.0));
        if(ImGui::Button("Save Settings"))
        {
            should_show = false;
        }
        m_save_settings_button_size = ImGui::GetItemRectSize();
        /// @end Button
    }
    ImGui::End();

    return should_show;
}

void SettingsWindow::Shutdown()
{

}