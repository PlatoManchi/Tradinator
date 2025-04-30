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
    if (ImGui::Begin("Auto Analysis Update"))
    {
        ImGui::TextWrapped("This is awesome window where new patterns recognized in new data is shown.");
    }
    ImGui::End();
}

void AutoAnalysisUpdateWindow::Shutdown()
{
}
