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

        ImVec2 settings_size = ImVec2(work_size.x - style.FramePadding.x * 4.0, work_size.y - m_save_settings_button_size.y - style.FramePadding.y * 8.0);

        /// @begin Child
        ImGui::BeginChild("child1", settings_size, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings);
        {
            /// @begin Separator
            ImGui::Separator();
            /// @end Separator
        
            ImGui::TextWrapped("NOTE: If text looks blury, it is likely because of DPI scaling issues. Alt + Enter to full screen and Alt + Enter again to exit full screen. This should fix the DPI scaling issues.");

            /// @begin Separator
            ImGui::Separator();
            /// @end Separator
        


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
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(237, 67, 55, 255));
            }

            /// @begin Text
            ImGui::Text("Working Folder:");
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




            /// @begin Text
            ImGui::Text("Max Parallel Downloads:");
            ImGui::TextWrapped("(Max number of download threads at once. Depends on internet connection)");
            /// @end Text

            /// @begin Input
            ImGui::SetNextItemWidth(work_size.x - style.ItemSpacing.x * 2.0);

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
            ImGui::SetNextItemWidth(work_size.x - style.ItemSpacing.x * 2.0);

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
            ImGui::SetNextItemWidth(work_size.x - style.ItemSpacing.x * 2.0);

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

            std::string patterns_heading_str = "Pattern Visibility";
            ImVec2 patterns_heading_size = ImGui::CalcTextSize(patterns_heading_str.c_str());
            ImVec2 prev_pos = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(work_size.x / 2.0 - patterns_heading_size.x / 2.0, prev_pos.y));
            ImGui::Text(patterns_heading_str.c_str());

            if (ImGui::Button("Select All"))
            {
                std::vector<std::unique_ptr<Pattern>> patterns = TradinatorCoreSpace::Utils::GetAvailablePatterns();
                for (std::unique_ptr<Pattern>& pattern : patterns)
                {
                    TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), true);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Select None"))
            {
                std::vector<std::unique_ptr<Pattern>> patterns = TradinatorCoreSpace::Utils::GetAvailablePatterns();
                for (std::unique_ptr<Pattern>& pattern : patterns)
                {
                    TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), false);
                }
            }
            std::vector<std::unique_ptr<Pattern>> patterns = TradinatorCoreSpace::Utils::GetAvailablePatterns();


            ImGuiStyle& style = ImGui::GetStyle();
            ImVec2 item_spacing = style.ItemSpacing;

            float pattern_width = 180.0f;
            float window_width = ImGui::GetWindowWidth();
            float table_width = (window_width - item_spacing.x * 3.0f) / 2.0f;
            int  column_count = std::max((int)(table_width / pattern_width), 1);

            uint64_t pattern_count = patterns.size();

            ImGui::SeparatorText("Bullish Patterns : ");

            if (ImGui::BeginTable("BullishPatternList", column_count, ImGuiTableFlags_None, { -1, -1 }))
            {
                int row = -1;
                uint64_t index = 0;
                for (const std::unique_ptr<Pattern>& pattern : patterns)
                {
                    if ((pattern->PatternType() & Bullish_Pattern_Type) != EPatternType::None)
                    {
                        int tmp_row = index / column_count;
                        if (tmp_row != row)
                        {
                            row = tmp_row;
                            ImGui::TableNextRow(0, 0);
                        }

                        ImGui::TableSetColumnIndex(index % column_count);

                        bool is_visible = TradinatorSettings::Get().GetPatternVisibility(pattern->PatternType());
                        ImGui::Checkbox(pattern->Name().c_str(), &is_visible);
                        TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), is_visible);

                        ++index;
                    }
                }

                ImGui::EndTable();
            }

            ImGui::SeparatorText("Bearish Patterns : ");

            if (ImGui::BeginTable("BearishPatternList", column_count, ImGuiTableFlags_None, { -1, -1 }))
            {
                int row = -1;
                uint64_t index = 0;
                for (const std::unique_ptr<Pattern>& pattern : patterns)
                {
                    if ((pattern->PatternType() & Bearish_Pattern_Type) != EPatternType::None)
                    {
                        int tmp_row = index / column_count;
                        if (tmp_row != row)
                        {
                            row = tmp_row;
                            ImGui::TableNextRow(0, 0);
                        }

                        ImGui::TableSetColumnIndex(index % column_count);

                        bool is_visible = TradinatorSettings::Get().GetPatternVisibility(pattern->PatternType());
                        ImGui::Checkbox(pattern->Name().c_str(), &is_visible);
                        TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), is_visible);

                        ++index;
                    }
                }

                ImGui::EndTable();
            }

            ImGui::SeparatorText("Generic Patterns : ");

            if (ImGui::BeginTable("GenericPatternList", column_count, ImGuiTableFlags_None, { -1, -1 }))
            {
                int row = -1;
                uint64_t index = 0;
                for (const std::unique_ptr<Pattern>& pattern : patterns)
                {
                    if ((pattern->PatternType() & Bullish_Pattern_Type) == EPatternType::None &&
                        (pattern->PatternType() & Bearish_Pattern_Type) == EPatternType::None)
                    {
                        int tmp_row = index / column_count;
                        if (tmp_row != row)
                        {
                            row = tmp_row;
                            ImGui::TableNextRow(0, 0);
                        }

                        ImGui::TableSetColumnIndex(index % column_count);

                        bool is_visible = TradinatorSettings::Get().GetPatternVisibility(pattern->PatternType());
                        ImGui::Checkbox(pattern->Name().c_str(), &is_visible);
                        TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), is_visible);

                        ++index;
                    }
                }

                ImGui::EndTable();
            }

            /// @begin Separator
            ImGui::Separator();
            /// @end Separator

        
            /// @separator
            ImGui::EndChild();
        }



        ImGui::SetCursorPos(ImVec2(work_size.x / 2.0f - m_save_settings_button_size.x / 2.0f, work_size.y - m_save_settings_button_size.y - style.FramePadding.y * 3.0));

        m_all_settings_valid = m_working_folder_exists && m_nse_market_security_list_exist;

        /// @begin Button
        if (!m_all_settings_valid)
        {
            ImGui::BeginDisabled(true);
        }
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