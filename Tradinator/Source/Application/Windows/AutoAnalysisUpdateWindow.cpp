#include "AutoAnalysisUpdateWindow.h"

#include "imgui.h"
#include "imspinner/imspinner.h"

#include "Data/Security.h"
#include "Utils/Utils.h"

#include "Application/TradinatorApp.h"
#include "Application/TradinatorSettings.h"


AutoAnalysisUpdateWindow::AutoAnalysisUpdateWindow(TradinatorApp& tradinator_app)
    : m_tradinator_app(tradinator_app)
{
}

void AutoAnalysisUpdateWindow::Init(std::shared_ptr<AsyncData<NewsPointVectorType>> news_points)
{
    m_news_points = news_points;
}

void AutoAnalysisUpdateWindow::Begin()
{
}

bool SelectableWrapped(const char* label, bool selected = false)
{
    ImVec2 work_size = ImGui::GetWindowSize();
    ImGuiStyle style = ImGui::GetStyle();

    ImVec2 wrapped_size = ImGui::CalcTextSize(label, nullptr, false, work_size.x - style.ItemSpacing.x * 3.0f);

    ImVec2 cusor_pos = ImGui::GetCursorPos();
    

    ImGui::PushID(label); // Unique ID
    ImGui::BeginGroup();

    // Draw a selectable dummy to capture the click
    ImGui::SetCursorPosX(cusor_pos.x);
    bool clicked = ImGui::Selectable("##hidden_selectable", selected, 0, ImVec2(work_size.x, wrapped_size.y));

    // Overlay the wrapped text
    ImGui::SameLine(0, 0);
    //ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + size.x);
    ImGui::SetCursorPosX(cusor_pos.x);
    ImGui::TextWrapped("%s", label);
    //ImGui::PopTextWrapPos();

    ImGui::EndGroup();
    ImGui::PopID();

    return clicked;
}

int64_t AutoAnalysisUpdateWindow::Show()
{
    int64_t selected_index = -1;
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoDocking
        //| ImGuiWindowFlags_AlwaysVerticalScrollbar;
        //ImGuiWindowFlags_NoSavedSettings
        ;

    if (ImGui::Begin("Auto Analysis Update", nullptr, flags))
    {
        ImGui::SeparatorText("Analysis");

        if (!m_news_points->WasEverReadyBefore() || !m_news_points->IsDataReady())
        {
            ImSpinner::SpinnerIncDots("PinnedSecuritiesWait", 18, 3, ImColor{ 1.f, 1.f, 1.f, 1.f }, 10.0f);
        }
        else
        {
            //ImGui::TextWrapped("This is awesome window where new patterns recognized in new data is shown.");
            NewsPointVectorType& news_points = m_news_points->GetData();
            for (int64_t i = 0; i < news_points.size(); ++i)
            {
                NewsPoint& news = news_points[i];
                if (news.m_pattern != EPattern::None)
                {
                    EPattern pattern = TradinatorCoreSpace::Utils::GetPatternFrom(news.m_pattern);
                    if (TradinatorSettings::Get().GetPatternVisibility(pattern))
                    {
                        std::string text = std::format("{} has {} at {:%d-%b-%Y}", news.m_security->Name(), TradinatorCoreSpace::Utils::GetPatternShortDescription(pattern), news.m_date);

                        if (SelectableWrapped(text.c_str()))
                        {
                            selected_index = i;
                        }
                    }
                }
            }
        }
        
    }

    ImGui::End();

    return selected_index;
}

void AutoAnalysisUpdateWindow::Shutdown()
{
}
