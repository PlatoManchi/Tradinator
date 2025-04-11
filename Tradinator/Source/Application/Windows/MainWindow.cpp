#include "MainWindow.h"

#include "imgui.h"
#include "imspinner/imspinner.h"

#include "Utils.h"

void MainWindow::Init(std::shared_ptr<TradinatorCore> tradinator_core)
{
    m_tradinator_core = tradinator_core;
}

void MainWindow::Begin()
{
}

void MainWindow::Show()
{
    ImGuiWindowFlags no_decoration =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoDocking
        //| ImGuiWindowFlags_AlwaysVerticalScrollbar;
        //ImGuiWindowFlags_NoSavedSettings
        ;
            
    //ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    if (ImGui::Begin("Main Window", nullptr, no_decoration))
    {
        
        //ImGui::PushStyleVar(ImGuiStyleVar_font)
        //ImGui::TextWrapped("This is awesome dashboard. Show new formed IPO details, some market stats like top 10 during last update and show the delta from last update.");
        //Utils::PushHeadingFont();
        ImGui::SeparatorText("Market Status");
        ImSpinner::SpinnerScaleDots("NSE", 15, 5);
        //Utils::PopFont();

        //ImGui::SetNextWindowPos(ImVec2(0, 0));
        /*ImGui::BeginChild("statusbar", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f));
        ImGui::Text("Samples:%d", 100); ImGui::SameLine();
        ImGui::Text("/%d", 100);
        ImGui::EndChild();
        */
        
    }
    ImGui::End();
}

void MainWindow::Shutdown()
{
}
