#include "TradinatorApp.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"

#include "Utils.h"

#include "Market/NSE_Market.h"
#include "TradinatorSettings.h"

#include "Windows/CounterWindow.h"


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
    bool should_exit = false;
    if (TradinatorSettings::Get().IsValid())
    {
        should_exit = !ShowMainWindow();
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

    return should_exit;
}

bool TradinatorApp::ShowMainWindow()
{
    bool should_exit = !m_main_windows.Show();

    for (std::pair<std::string, std::shared_ptr<CounterWindow>> pair : m_counter_windows)
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

    std::erase_if(m_counter_windows, [](const std::pair<std::string, std::shared_ptr<CounterWindow>>& item) {
        return item.second->m_close;
        });

    return should_exit;
}


void TradinatorApp::ShowCounterWindow(std::shared_ptr<Counter> counter)
{
    if (!m_counter_windows.contains(counter->ISIN_Number()))
    {
        m_counter_windows[counter->ISIN_Number()] = std::make_shared<CounterWindow>(counter);
    }
    else
    {
        //TODO: Bring window to foreground
    }
}


void TradinatorApp::Shutdown()
{
    SaveWindowsState();

    m_tradinator_core->Shutdown();

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
            are_all_markets_ready = are_all_markets_ready && market->IsCounterDataAvailable();
            if (!are_all_markets_ready)
                break;
        }

        // wait till the counter data for all markets is available
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // read from file
    Json::Value status = TradinatorSettings::Get().GetAllOpenedWindowsStatus();

    Json::Value::ArrayIndex count = status.size();
    for (Json::Value::ArrayIndex i = 0; i < count; ++i)
    {
        Json::Value window_details = status[i];
        std::shared_ptr<Counter> counter = nullptr;

        std::string symbol_str = window_details["Symbol"].asString();
        for (const std::shared_ptr<Market>& market : markets)
        {
            const std::map<std::string, std::shared_ptr<Counter>>& data = market->GetCounterAsyncData().GetData();
            if (data.find(symbol_str) != data.end())
            {
                const std::shared_ptr<Counter>& tmp_counter = data.at(symbol_str);
                if (tmp_counter && tmp_counter->ISIN_Number() == window_details["ISIN"].asString())
                {
                    counter = tmp_counter;
                    break;
                }
            }
        }

        if (counter)
        {
            ShowCounterWindow(counter);
            m_counter_windows[counter->ISIN_Number()]->SetCounterStatus(window_details);
        }
    }
}

void TradinatorApp::SaveWindowsState()
{
    Json::Value result(Json::arrayValue);

    for (auto& pair : m_counter_windows)
    {
        result.append(pair.second->GetCounterStatus());
    }
    
    TradinatorSettings::Get().SetAllOpenedWindowsStatus(result);
}