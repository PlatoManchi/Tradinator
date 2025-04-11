#include "TradinatorApp.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "Utils.h"

#include "Market/NSE_Market.h"

TradinatorApp::TradinatorApp()
    : m_tradinator_core(std::make_shared<TradinatorCore>("D:/Stock Data"))
{
}

void TradinatorApp::Init()
{
    

    m_dashboard_window.Init();
    m_audo_analysis_update_window.Init();
    m_main_windows.Init(m_tradinator_core);
    m_securities_search_bar.Init(m_tradinator_core);
    m_status_bar.Init();
}

void TradinatorApp::Begin()
{

    // Creating markets that we want to follow
    m_tradinator_core->AddMarket(std::make_shared<NSE_Market>());



    m_dashboard_window.Begin();
    m_audo_analysis_update_window.Begin();
    m_main_windows.Begin();
    m_securities_search_bar.Begin();
    m_status_bar.Begin();
}




void TradinatorApp::ShowApp()
{
    ShowMainMenu();

    const float search_bar_height = 80.0f;
    const float status_bar_height = 70.0f;
    
    ImVec2 work_pos = ImGui::GetMainViewport()->WorkPos;
    ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
    



    ImGui::SetNextWindowPos(work_pos);
    ImGui::SetNextWindowSize(ImVec2(work_size.x, search_bar_height));
    if (std::shared_ptr<Security> security = m_securities_search_bar.Show())
    {
        if (!m_security_windows.contains(security->ISIN_Number()))
        {
            m_security_windows[security->ISIN_Number()] = std::make_shared<SecurityWindow>(security);
        }
        else
        {
            //TODO: Bring window to foreground
        }
    }

    ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y + search_bar_height));
    ImGui::SetNextWindowSize(ImVec2(work_size.x, work_size.y - (search_bar_height + status_bar_height)));
    m_main_windows.Show();


    ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y + work_size.y - status_bar_height));
    ImGui::SetNextWindowSize(ImVec2(work_size.x, status_bar_height));
    m_status_bar.Show();


    for (std::pair<std::string, std::shared_ptr<SecurityWindow>> pair : m_security_windows)
    {
        ImGui::SetNextWindowSize(ImVec2(1280, 768), ImGuiCond_Once);
        pair.second->Show();
    }
}




void TradinatorApp::Shutdown()
{
    m_tradinator_core->Shutdown();

    m_dashboard_window.Shutdown();
    m_audo_analysis_update_window.Shutdown();
    m_main_windows.Shutdown();
    m_securities_search_bar.Shutdown();
    m_status_bar.Shutdown();
}




void TradinatorApp::ShowMainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowMainMenu_File();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {} // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }

        ImGui::BeginChild("child", ImVec2(0, 40), ImGuiChildFlags_Borders);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();

        ImGui::EndMainMenuBar();
    }
}

void TradinatorApp::ShowMainMenu_File()
{
    //IMGUI_DEMO_MARKER("Examples/Menu");
    ImGui::MenuItem("(demo menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowMainMenu_File();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    //IMGUI_DEMO_MARKER("Examples/Menu/Options");
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), ImGuiChildFlags_Borders);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    //IMGUI_DEMO_MARKER("Examples/Menu/Colors");
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        //IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}
