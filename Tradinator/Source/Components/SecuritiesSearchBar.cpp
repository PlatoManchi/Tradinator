#include  "SecuritiesSearchBar.h"

#include <string>
#include <algorithm>
#include <iostream>

#include "imgui.h"
#include "imspinner/imspinner.h"

#include "TradinatorCore.h"
#include "Market/Market.h"



CounterAutoCompleteItem::CounterAutoCompleteItem(std::shared_ptr<Counter> counter)
    : m_counter(counter)
{
    m_cached_description = std::format("{}\n     Symbol: {}\n     ISIN Number: {}", m_counter->Name(), m_counter->Symbol(), m_counter->ISIN_Number());
}

bool CounterAutoCompleteItem::CanPassFilter(const char* search_string)
{
    // case insensitive search
    std::string desc = std::format("{} {} {}", m_counter->Symbol(), m_counter->Name(), m_counter->ISIN_Number());
    std::string search = std::string(search_string);

    std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);

    int search_index = desc.find(search);
    if (search_index != -1)
        return true;

    return false;
}

const std::string& CounterAutoCompleteItem::GetItemDescription()
{
    return m_cached_description;
}



void SecuritiesSearchBar::Init(std::shared_ptr<TradinatorCore> tradinator_core)
{
    m_tradinator_core = tradinator_core;
}

void SecuritiesSearchBar::Begin()
{
}

std::shared_ptr<Counter> SecuritiesSearchBar::Show()
{
    std::shared_ptr<Counter> result = nullptr;

    ImGuiWindowFlags no_decoration =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking;
    
    char* search_symbol = new char();

    if (ImGui::Begin("SearchBar", nullptr, no_decoration))
    {

        if (!m_autocomplete.IsInitialized())
        {
            std::vector<std::shared_ptr<Market>> markets = m_tradinator_core->GetAllMarkets();
            bool are_all_markets_ready = true;
            for (const std::shared_ptr<Market>& market : markets)
            {
                if (!market->IsCounterDataAvailable())
                {
                    are_all_markets_ready = false;
                    break;
                }
            }
            if (are_all_markets_ready)
            {
                std::vector<std::shared_ptr<IAutoCommpleteItem>> items;

                for (const std::shared_ptr<Market>& market : markets)
                {
                    if (market->IsCounterDataAvailable())
                    {
                        const AsyncData<std::map<std::string, std::shared_ptr<Counter>>>& securities_async_data = market->GetCounterAsyncData();
                        const std::map<std::string, std::shared_ptr<Counter>>& securities = securities_async_data.GetData();

                        for (std::pair<std::string, std::shared_ptr<Counter>> pair : securities)
                        {
                            std::shared_ptr<CounterAutoCompleteItem> item = std::make_shared<CounterAutoCompleteItem>(pair.second);
                            items.push_back(std::move(item));
                        }
                    }
                }

                m_autocomplete.Init(std::move(items));
            }

            ImGui::TextWrapped("Waiting for markets to load symbols data"); ImGui::SameLine();
            ImSpinner::SpinnerScaleDots("Status", 20, 5);
        }
        else
        {
            if (std::shared_ptr<IAutoCommpleteItem> item = m_autocomplete.Show("##CounterSearchBar", "##CounterSearchBar", "Search by company name, symbol or keywords ..."))
            {
                std::shared_ptr<CounterAutoCompleteItem> counter_item = std::dynamic_pointer_cast<CounterAutoCompleteItem> (item);
                result = counter_item->GetCounter();
                std::cout << "Selected item: " << counter_item->GetItemDescription() << std::endl;
            }
        }
    }
    ImGui::End();

    return result;
}

void SecuritiesSearchBar::Shutdown()
{
}
