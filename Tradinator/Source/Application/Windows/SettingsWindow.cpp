#include "Application/Windows/SettingsWindow.h"

#include <format>



#include "Utils/Utils.h"
#include "Patterns/Pattern.h"
#include "Strategy/Strategy.h"
#include "TradinatorCore.h"

#include "Application/TradinatorSettings.h"

SettingsWindow::SettingsWindow()
    : m_max_parallel_downloads(100)
    , m_max_parallel_process(100)
{

}


void SettingsWindow::Init(std::shared_ptr<TradinatorCore> tradinator_core)
{
    m_tradinator_core = tradinator_core;
}

void SettingsWindow::Begin()
{
    m_patterns = TradinatorCoreSpace::Utils::GetAvailablePatterns();
    m_strategies = TradinatorCoreSpace::Utils::GetAvailableStrategies();
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

            /// @begin Button
            bool is_processing = m_tradinator_core->IsProcessing();
            if (is_processing)
            {
                ImGui::BeginDisabled(true);
            }
            if (ImGui::Button("Redo Auto Analysis", { 0, 0 }))
            {
                m_tradinator_core->RedoAutoAnalysis();
            }
            if (ImGui::IsItemHovered()) {
                if (is_processing)
                {
                    ImGui::SetTooltip("Cannot do this while processing.");
                }
                else
                {
                    ImGui::SetTooltip("Delete previous auto analysis data and redo auto analysis for all securities in all markets.");
                }
            }
            if (is_processing)
            {
                ImGui::EndDisabled();
            }
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








            std::string strategy_heading_str = "Strategy Visibility";
            ImVec2 strategy_heading_size = ImGui::CalcTextSize(strategy_heading_str.c_str());
            ImVec2 prev_pos = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(work_size.x / 2.0 - strategy_heading_size.x / 2.0, prev_pos.y));
            ImGui::Text(strategy_heading_str.c_str());

            if (ImGui::Button("Select All##Strategy"))
            {
                for (std::unique_ptr<Strategy>& strategy : m_strategies)
                {
                    TradinatorSettings::Get().SetStrategyVisibility(strategy->GetStrategyType(), true);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Select None##Strategy"))
            {
                for (std::unique_ptr<Strategy>& strategy : m_strategies)
                {
                    TradinatorSettings::Get().SetStrategyVisibility(strategy->GetStrategyType(), false);
                }
            }


            ImGuiStyle& style = ImGui::GetStyle();
            ImVec2 item_spacing = style.ItemSpacing;

            float strategy_width = 180.0f;
            float window_width = ImGui::GetWindowWidth();
            float table_width = (window_width - item_spacing.x * 3.0f) / 2.0f;
            int  column_count = std::max((int)(table_width / strategy_width), 1);

            if (ImGui::BeginTable("StrategyList", column_count, ImGuiTableFlags_None, { -1, 0 }))
            {
                int row = -1;
                uint64_t index = 0;
                for (std::unique_ptr<Strategy>& strategy : m_strategies)
                {
                    int tmp_row = index / column_count;
                    if (tmp_row != row)
                    {
                        row = tmp_row;
                        ImGui::TableNextRow(0, 0);
                    }

                    ImGui::TableSetColumnIndex(index % column_count);

                    bool is_visible = TradinatorSettings::Get().GetStrategyVisibility(strategy->GetStrategyType());
                    ImGui::Checkbox(strategy->Name().c_str(), &is_visible);
                    TradinatorSettings::Get().SetStrategyVisibility(strategy->GetStrategyType(), is_visible);
                    if (ImGui::IsItemHovered()) 
                    {
                        ImGui::SetTooltip(TradinatorCoreSpace::Utils::GetStrategyDesc(strategy->GetStrategyType()).c_str());
                    }
                    ++index;
                }

                ImGui::EndTable();
            }




            /// @begin Separator
            ImGui::Separator();
            /// @end Separator

            std::string patterns_heading_str = "Pattern Visibility";
            ImVec2 patterns_heading_size = ImGui::CalcTextSize(patterns_heading_str.c_str());
            prev_pos = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(work_size.x / 2.0 - patterns_heading_size.x / 2.0, prev_pos.y));
            ImGui::Text(patterns_heading_str.c_str());

            if (ImGui::Button("Select All##Pattern"))
            {
                for (std::unique_ptr<Pattern>& pattern : m_patterns)
                {
                    TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), true);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Select None##Pattern"))
            {
                for (std::unique_ptr<Pattern>& pattern : m_patterns)
                {
                    TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), false);
                }
            }
            

            float pattern_width = 180.0f;
            table_width = (window_width - item_spacing.x * 3.0f) / 2.0f;
            column_count = std::max((int)(table_width / pattern_width), 1);

            uint64_t pattern_count = m_patterns.size();

            ImGui::SeparatorText("Bullish Patterns : ");

            if (ImGui::BeginTable("BullishPatternList", column_count, ImGuiTableFlags_None, { -1, 0 }))
            {
                int row = -1;
                uint64_t index = 0;
                for (const std::unique_ptr<Pattern>& pattern : m_patterns)
                {
                    if ((pattern->PatternType() & Bullish_Pattern_Type) != EPattern::None)
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
                for (const std::unique_ptr<Pattern>& pattern : m_patterns)
                {
                    if ((pattern->PatternType() & Bearish_Pattern_Type) != EPattern::None)
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
                for (const std::unique_ptr<Pattern>& pattern : m_patterns)
                {
                    if ((pattern->PatternType() & Bullish_Pattern_Type) == EPattern::None &&
                        (pattern->PatternType() & Bearish_Pattern_Type) == EPattern::None)
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