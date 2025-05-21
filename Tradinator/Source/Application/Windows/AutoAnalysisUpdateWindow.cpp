#include "AutoAnalysisUpdateWindow.h"

#include "imgui.h"

AutoAnalysisUpdateWindow::AutoAnalysisUpdateWindow()
{
}

void AutoAnalysisUpdateWindow::Init()
{
}

void AutoAnalysisUpdateWindow::Begin()
{
}

void AutoAnalysisUpdateWindow::Show()
{
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
        ImGui::TextWrapped("This is awesome window where new patterns recognized in new data is shown.");
    }
    ImGui::End();
}

void AutoAnalysisUpdateWindow::Shutdown()
{
}
