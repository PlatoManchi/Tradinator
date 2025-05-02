#include "Application/Windows/SettingsWindow.h"

#include <format>
#include "Utils/Utils.h"
#include "Application/TradinatorSettings.h"

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


            /// @begin Separator
            ImGui::Separator();
            /// @end Separator
            
            /// @begin Text
            ImGui::Text("Max Parallel Downloads:");
            ImGui::TextWrapped("(Max number of download threads at once. Depends on internet connection)");
            /// @end Text

            /// @begin Input
            //ImGui::SetNextItemWidth(work_size.x - style.ItemSpacing.x * 2.0);

            std::string curr_max_parallel_download_str = std::format("{}", TradinatorCoreSpace::Utils::GetMaxParallelDownloads());
            char max_parallel_download_str[10] = "";
            std::copy(curr_max_parallel_download_str.begin(), curr_max_parallel_download_str.end(), max_parallel_download_str);
            if (ImGui::InputText("##max_parallel_downloads", max_parallel_download_str, 10, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                int num = std::atoi(max_parallel_download_str);
                TradinatorCoreSpace::Utils::SetMaxParallelDownloads(num);
            }
            /// @end Input

            /// @begin Separator
            ImGui::Separator();
            /// @end Separator



            /// @begin Text
            ImGui::Text("Max Parallel Analysis:");
            ImGui::TextWrapped("(Max number of threads spawned that will run analysis on securities to determine patterns. This needs to load the candle data and also run analysis, higher numbers needs more RAM and CPU threads.)");
            /// @end Text

            /// @begin Input
            //ImGui::SetNextItemWidth(work_size.x - style.ItemSpacing.x * 2.0);

            std::string curr_max_parallel_analysis_str = std::format("{}", TradinatorCoreSpace::Utils::GetMaxParallelAnalysis());
            char max_parallel_analysis_str[10] = "";
            std::copy(curr_max_parallel_analysis_str.begin(), curr_max_parallel_analysis_str.end(), max_parallel_analysis_str);
            if (ImGui::InputText("##max_parallel_analysis", max_parallel_analysis_str, 10, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                int num = std::atoi(max_parallel_analysis_str);
                TradinatorCoreSpace::Utils::SetMaxParallelAnalysis(num);
            }
            /// @end Input

            /// @begin Separator
            ImGui::Separator();
            /// @end Separator


            /// @begin Text
            ImGui::Text("Read Write Batch Size:");
            ImGui::TextWrapped("(Max number of threads spawned that will read downloaded candle data and write into local database. Higher number needs more RAM, CPU and faster drive.)");
            /// @end Text

            /// @begin Input
            //ImGui::SetNextItemWidth(work_size.x - style.ItemSpacing.x * 2.0);

            std::string curr_read_write_batch_size_str = std::format("{}", TradinatorCoreSpace::Utils::GetReadWriteBatchSize());
            char read_write_batch_size_str[10] = "";
            std::copy(curr_read_write_batch_size_str.begin(), curr_read_write_batch_size_str.end(), read_write_batch_size_str);
            if (ImGui::InputText("##read_write_batch_size", read_write_batch_size_str, 10, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                int num = std::atoi(read_write_batch_size_str);
                TradinatorCoreSpace::Utils::SetReadWriteBatchSize(num);
            }
            /// @end Input

            /// @begin Separator
            ImGui::Separator();
            /// @end Separator

            std::string patterns_heading_str = "Pattern Visibility";
            ImVec2 patterns_heading_size = ImGui::CalcTextSize(patterns_heading_str.c_str());
            ImVec2 prev_pos = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(work_size.x / 2.0 - patterns_heading_size.x / 2.0, prev_pos.y));
            ImGui::Text(patterns_heading_str.c_str());

            std::vector<std::unique_ptr<Pattern>> patterns = TradinatorCoreSpace::Utils::GetAvailablePatterns();
            for (std::unique_ptr<Pattern>& pattern : patterns)
            {
                bool is_visible = TradinatorSettings::Get().GetPatternVisibility(pattern->PatternType());
                ImGui::Checkbox(TradinatorCoreSpace::Utils::GetPatternShortDescription(pattern->PatternType()).c_str(), &is_visible);
                TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), is_visible);
            }

            /// @begin Separator
            ImGui::Separator();
            /// @end Separator
            

            /// @separator
            ImGui::EndChild();
        }
        /// @end Child

        /// @begin Button
        ImGui::SetCursorPos(ImVec2(work_size.x / 2.0f - m_save_settings_button_size.x / 2.0f, work_size.y - m_save_settings_button_size.y - style.FramePadding.y * 3.0));
        if(ImGui::Button("Close"))
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