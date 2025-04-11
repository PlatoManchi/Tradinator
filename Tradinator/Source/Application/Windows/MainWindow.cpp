#include "MainWindow.h"

#include "imgui.h"

void MainWindow::Init()
{
}

void MainWindow::Begin()
{
}

void MainWindow::Show()
{
    ImGuiWindowFlags no_decoration =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove
        //| ImGuiWindowFlags_AlwaysVerticalScrollbar;
        //ImGuiWindowFlags_NoSavedSettings
        ;
            
    //ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    if (ImGui::Begin("Main Window", nullptr, no_decoration))
    {
        

        ImGui::TextWrapped("This is awesome dashboard. Show new formed IPO details, some market stats like top 10 during last update and show the delta from last update.");


        m_autocomplete.Draw("##value4", "Tes", { "Test1", "Test2", "2 times", "time is bad" });

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
