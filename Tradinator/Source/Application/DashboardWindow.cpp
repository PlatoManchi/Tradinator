#include "DashboardWindow.h"

#include "imgui.h"

DashboardWindow::DashboardWindow()
{
}

void DashboardWindow::Init()
{
}

void DashboardWindow::Begin()
{
}

void DashboardWindow::Show()
{
    if(ImGui::Begin("Dashboard"))
    {
        ImGui::TextWrapped("This is awesome dashboard. Show new formed IPO details, some market stats like top 10 during last update and show the delta from last update.");
    }
    ImGui::End();
}

void DashboardWindow::Shutdown()
{
}
