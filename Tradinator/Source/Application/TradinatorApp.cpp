#include "TradinatorApp.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"

#include "Utils.h"

#include "Market/NSE_Market.h"
#include "TradinatorSettings.h"

#include "Windows/SecurityWindow.h"


TradinatorApp::TradinatorApp()
    : m_tradinator_core(nullptr)
    , m_main_windows(*this)
{
    if (TradinatorSettings::Get().IsValid())
    {
        m_tradinator_core = std::make_shared<TradinatorCore>(TradinatorSettings::Get().GetWorkingFolder());
    }
}

void TradinatorApp::Init()
{
    if (TradinatorSettings::Get().IsValid())
    {
        m_main_windows.Init(m_tradinator_core);
    }
    else
    {
        m_startup_window.Init();
    }
}

void TradinatorApp::Begin()
{
    if (TradinatorSettings::Get().IsValid())
    {
        // Creating markets that we want to follow
        m_tradinator_core->AddMarket(std::make_shared<NSE_Market>());

        m_tradinator_core->Init();

        m_main_windows.Begin();

        LoadWindowsState();
    }
    else
    {
        m_startup_window.Begin();
    }
}




bool TradinatorApp::ShowApp()
{
    bool should_app_run = true;
    if (TradinatorSettings::Get().IsValid())
    {
        should_app_run = ShowMainWindow();
    }
    else
    {
        m_startup_window.Show();
        if (TradinatorSettings::Get().IsValid())
        {
            m_tradinator_core = std::make_shared<TradinatorCore>(TradinatorSettings::Get().GetWorkingFolder());

            Init();
            Begin();
        }
    }

    return should_app_run;
}

bool TradinatorApp::ShowMainWindow()
{
    bool should_app_run = m_main_windows.Show();

    for (std::pair<std::string, std::shared_ptr<SecurityWindow>> pair : m_security_windows)
    {
        ImGui::SetNextWindowSize(ImVec2(1280, 1368), ImGuiCond_FirstUseEver);
        if (pair.second->m_maximize)
        {
            pair.second->m_maximize = false;

            ImGuiPlatformMonitor monitor = ImGui::GetPlatformIO().Monitors[0];
            ImGui::SetNextWindowPos(monitor.WorkPos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(monitor.WorkSize, ImGuiCond_Always);
        }

        pair.second->Show();
    }

    std::erase_if(m_security_windows, [](const std::pair<std::string, std::shared_ptr<SecurityWindow>>& item) {
        return item.second->m_close;
        });

    return should_app_run;
}


std::shared_ptr<SecurityWindow> TradinatorApp::ShowSecurityWindow(std::shared_ptr<Security> security)
{
    if (!m_security_windows.contains(security->ISIN_Number()))
    {
        m_security_windows[security->ISIN_Number()] = std::make_shared<SecurityWindow>(security);
    }
    else
    {
        //TODO: Bring window to foreground
    }

    return m_security_windows[security->ISIN_Number()];
}


void TradinatorApp::Shutdown()
{
    SaveWindowsState();
    TradinatorSettings::Get().SaveSettings();

    if (m_tradinator_core)
    {
        m_tradinator_core->Shutdown();
    }
    
    m_main_windows.Shutdown();
}





void TradinatorApp::LoadWindowsState()
{
    const std::vector<std::shared_ptr<Market>>& markets = m_tradinator_core->GetAllMarkets();
    bool are_all_markets_ready = false;
    while (!are_all_markets_ready)
    {
        are_all_markets_ready = true;
        for (const std::shared_ptr<Market>& market : markets)
        {
            are_all_markets_ready = are_all_markets_ready && market->IsSecurityDataAvailable();
            if (!are_all_markets_ready)
                break;
        }

        // wait till the security data for all markets is available
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // read from file
    Json::Value status = TradinatorSettings::Get().GetAllOpenedWindowsStatus();

    Json::Value::ArrayIndex count = status.size();
    for (Json::Value::ArrayIndex i = 0; i < count; ++i)
    {
        Json::Value window_details = status[i];
        std::shared_ptr<Security> security = nullptr;

        std::string symbol_str = window_details["Symbol"].asString();
        for (const std::shared_ptr<Market>& market : markets)
        {
            const std::map<std::string, std::shared_ptr<Security>>& data = market->GetSecurityAsyncData().GetData();
            if (data.find(symbol_str) != data.end())
            {
                const std::shared_ptr<Security>& tmp_security = data.at(symbol_str);
                if (tmp_security && tmp_security->ISIN_Number() == window_details["ISIN"].asString())
                {
                    security = tmp_security;
                    break;
                }
            }
        }

        if (security)
        {
            ShowSecurityWindow(security);
            m_security_windows[security->ISIN_Number()]->SetSecurityStatus(window_details);
        }
    }
}

void TradinatorApp::SaveWindowsState()
{
    Json::Value result(Json::arrayValue);

    for (auto& pair : m_security_windows)
    {
        result.append(pair.second->GetSecurityStatus());
    }
    
    TradinatorSettings::Get().SetAllOpenedWindowsStatus(result);
}