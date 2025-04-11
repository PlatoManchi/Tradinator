#include "StatusBar.h"

#include "imgui.h"
#include "imspinner/imspinner.h"

void StatusBar::Init()
{
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

        ImSpinner::SpinnerScaleDots("Status", 15, 5); ImGui::SameLine();
        ImGui::TextWrapped("Status: 100/100"); ImGui::SameLine();
        ImGui::ProgressBar(0.5f, { 200, 0 }, nullptr);


        
    }
    ImGui::End();
}

void StatusBar::Shutdown()
{
}
