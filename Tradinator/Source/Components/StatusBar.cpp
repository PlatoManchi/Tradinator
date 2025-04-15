#include "StatusBar.h"

#include "imgui.h"
#include "imspinner/imspinner.h"

#include "TradinatorCore.h"

void StatusBar::Init(std::shared_ptr<TradinatorCore> tradinator_core)
{
    m_tradinator_core = tradinator_core;
}

void StatusBar::Begin()
{
}

void StatusBar::Show()
{
    ImGuiWindowFlags no_decoration =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking;
        

    if (ImGui::Begin("StatusBar", nullptr, no_decoration))
    {
        if (m_tradinator_core->IsProcessing())
        {
            ImSpinner::SpinnerIncDots("NSE", 18, 3, ImColor{ 1.f, 1.f, 1.f, 1.f }, 10.0f); ImGui::SameLine();
            ImGui::TextWrapped("Processing ...");
            //ImGui::ProgressBar(0.5f, { 200, 0 }, nullptr);
        }
        else
        {
            ImGui::TextWrapped("Ready"); ImGui::SameLine();
        }
    }
    ImGui::End();
}

void StatusBar::Shutdown()
{
}
