#include "MainWindow.h"

#include <format>
#include <chrono>

#include "imgui.h"
#include "imspinner/imspinner.h"

#include "TradinatorCore.h"
#include "Data/AsyncData.h"
#include "Data/Security.h"
#include "Patterns/Pattern.h"
#include "Strategy/Strategy.h"
#include "Utils/Utils.h"

#include "Utils.h"
#include "Application/TradinatorApp.h"
#include "Application/Windows/SecurityWindow.h"
#include "Application/TradinatorSettings.h"

MainWindow::MainWindow(TradinatorApp& tradinator_app)
    : m_dashboard_window(tradinator_app)
    , m_pinned_securities_window(tradinator_app)
    , m_auto_analysis_update_window()
    , m_show_settings_window(false)
    , m_should_exit(false)
    , m_tradinator_app(tradinator_app)
{

}

void MainWindow::Init(std::shared_ptr<TradinatorCore> tradinator_core)
{
    m_tradinator_core = tradinator_core;

    m_dashboard_window.Init(m_tradinator_core);
    m_pinned_securities_window.Init(m_tradinator_core);
    m_auto_analysis_update_window.Init(m_tradinator_core->GetGlobalNews());
    m_securities_search_bar.Init(m_tradinator_core);
    m_status_bar.Init(m_tradinator_core);
    m_settings_window.Init(m_tradinator_core);

    m_should_exit = false;
}

void MainWindow::Begin()
{
    m_dashboard_window.Begin();
    m_pinned_securities_window.Begin();
    m_auto_analysis_update_window.Begin();
    m_securities_search_bar.Begin();
    m_status_bar.Begin();
    m_settings_window.Begin();

    m_patterns = TradinatorCoreSpace::Utils::GetAvailablePatterns();
    m_strategies = TradinatorCoreSpace::Utils::GetAvailableStrategies();

    m_should_exit = false;
}

bool MainWindow::Show()
{
    ShowMainMenu();

    if (!m_show_settings_window)
    {
        const float search_bar_height = 80.0f;
        const float status_bar_height = 70.0f;

        ImVec2 work_pos = ImGui::GetMainViewport()->WorkPos;
        ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;




        ImGui::SetNextWindowPos(work_pos);
        ImGui::SetNextWindowSize(ImVec2(work_size.x, search_bar_height));
        if (std::shared_ptr<Security> security = m_securities_search_bar.Show())
        {
            m_tradinator_app.ShowSecurityWindow(security);
        }

        float pinned_securities_width = work_size.x * 0.2f > 400 ? 400 : work_size.x * 0.2f;
        float auto_analysis_width = work_size.x * 0.2f > 400 ? 400 : work_size.x * 0.2f;
        float dashboard_width = work_size.x - auto_analysis_width - pinned_securities_width;

        ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y + search_bar_height));
        ImGui::SetNextWindowSize(ImVec2(pinned_securities_width, work_size.y - (search_bar_height + status_bar_height)));
        m_pinned_securities_window.Show();

        ImGui::SetNextWindowPos(ImVec2(work_pos.x + pinned_securities_width, work_pos.y + search_bar_height));
        ImGui::SetNextWindowSize(ImVec2(dashboard_width, work_size.y - (search_bar_height + status_bar_height)));
        m_dashboard_window.Show();

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDocking
            //| ImGuiWindowFlags_AlwaysVerticalScrollbar;
            //ImGuiWindowFlags_NoSavedSettings
            ;
        ImGui::SetNextWindowPos(ImVec2(work_pos.x + pinned_securities_width + dashboard_width, work_pos.y + search_bar_height));
        ImGui::SetNextWindowSize(ImVec2(auto_analysis_width, work_size.y - (search_bar_height + status_bar_height)));
        if (ImGui::Begin("Auto", nullptr, flags))
        {
            int64_t selected_index = m_auto_analysis_update_window.Show();
            if (selected_index >= 0)
            {
                NewsPointVectorType& news_points = m_tradinator_core->GetGlobalNews()->GetData();
                if (selected_index < news_points.size())
                {
                    NewsPoint& news = news_points[selected_index];
                    std::shared_ptr<SecurityWindow> window = m_tradinator_app.ShowSecurityWindow(news.m_security);
                    window->HilightNews(news);
                }
            }
        }
        ImGui::End();
        

        ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y + work_size.y - status_bar_height));
        ImGui::SetNextWindowSize(ImVec2(work_size.x, status_bar_height));
        m_status_bar.Show();
    }
    else
    {
        m_show_settings_window = m_settings_window.Show();
    }

    return !m_should_exit;
}

void MainWindow::ShowMainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowMainMenu_File();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void MainWindow::ShowMainMenu_File()
{
    if (ImGui::BeginMenu("Strategy Visibility"))
    {
        if (ImGui::Button("Select All"))
        {
            for (std::unique_ptr<Strategy>& strategy : m_strategies)
            {
                TradinatorSettings::Get().SetStrategyVisibility(strategy->GetStrategyType(), true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Select None"))
        {
            for (std::unique_ptr<Strategy>& strategy : m_strategies)
            {
                TradinatorSettings::Get().SetStrategyVisibility(strategy->GetStrategyType(), false);
            }
        }

        for (std::unique_ptr<Strategy>& strategy : m_strategies)
        {
            bool is_visible = TradinatorSettings::Get().GetStrategyVisibility(strategy->GetStrategyType());
            ImGui::Checkbox(strategy->Name().c_str(), &is_visible);
            TradinatorSettings::Get().SetStrategyVisibility(strategy->GetStrategyType(), is_visible);
            if (ImGui::IsItemHovered()) 
            {
                ImGui::SetTooltip(TradinatorCoreSpace::Utils::GetStrategyDesc(strategy->GetStrategyType()).c_str());
            }
        }
        ImGui::EndMenu();
    }


    if (ImGui::BeginMenu("Pattern Visibility")) 
    {
        if (ImGui::Button("Select All"))
        {
            for (std::unique_ptr<Pattern>& pattern : m_patterns)
            {
                TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Select None"))
        {
            for (std::unique_ptr<Pattern>& pattern : m_patterns)
            {
                TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), false);
            }
        }

        ImGui::SeparatorText("Bullish Patterns : ");
        for (const std::unique_ptr<Pattern>& pattern : m_patterns)
        {
            if ((pattern->PatternType() & Bullish_Pattern_Type) != EPattern::None)
            {
                bool is_visible = TradinatorSettings::Get().GetPatternVisibility(pattern->PatternType());
                ImGui::Checkbox(pattern->Name().c_str(), &is_visible);
                TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), is_visible);
            }
        }
        ImGui::SeparatorText("Bearish Patterns : ");
        for (const std::unique_ptr<Pattern>& pattern : m_patterns)
        {
            if ((pattern->PatternType() & Bearish_Pattern_Type) != EPattern::None)
            {
                bool is_visible = TradinatorSettings::Get().GetPatternVisibility(pattern->PatternType());
                ImGui::Checkbox(pattern->Name().c_str(), &is_visible);
                TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), is_visible);
            }
        }
        ImGui::SeparatorText("Generic Patterns : ");
        for (const std::unique_ptr<Pattern>& pattern : m_patterns)
        {
            if ((pattern->PatternType() & Bullish_Pattern_Type) == EPattern::None &&
                (pattern->PatternType() & Bearish_Pattern_Type) == EPattern::None)
            {
                bool is_visible = TradinatorSettings::Get().GetPatternVisibility(pattern->PatternType());
                ImGui::Checkbox(pattern->Name().c_str(), &is_visible);
                TradinatorSettings::Get().SetPatternVisbility(pattern->PatternType(), is_visible);
            }
        }

        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Settings"))
    {
        m_show_settings_window = true;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) 
    {
        m_should_exit = true;
    }
}

void MainWindow::Shutdown()
{
    m_dashboard_window.Shutdown();
    m_pinned_securities_window.Shutdown();
    m_auto_analysis_update_window.Shutdown();
    m_securities_search_bar.Shutdown();
    m_status_bar.Shutdown();
    m_settings_window.Shutdown();
}
