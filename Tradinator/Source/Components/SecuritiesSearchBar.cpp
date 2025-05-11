#include  "SecuritiesSearchBar.h"

#include <string>
#include <algorithm>
#include <iostream>

#include "imgui.h"
#include "imspinner/imspinner.h"

#include "TradinatorCore.h"
#include "Market/Market.h"



SecurityAutoCompleteItem::SecurityAutoCompleteItem(std::shared_ptr<Security> security)
    : m_security(security)
{
    m_cached_description = std::format("{}\n     Symbol: {}\n     ISIN Number: {}", m_security->Name(), m_security->Symbol(), m_security->ISIN_Number());
}

bool SecurityAutoCompleteItem::CanPassFilter(const char* search_string)
{
    // case insensitive search
    std::string desc = std::format("{} {} {}", m_security->Symbol(), m_security->Name(), m_security->ISIN_Number());
    std::string search = std::string(search_string);

    std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);

    size_t search_index = desc.find(search);
    if (search_index != -1)
        return true;

    return false;
}

const std::string& SecurityAutoCompleteItem::GetItemDescription()
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

std::shared_ptr<Security> SecuritiesSearchBar::Show()
{
    std::shared_ptr<Security> result = nullptr;

    ImGuiWindowFlags no_decoration =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking;
    
    
    if (ImGui::Begin("SearchBar", nullptr, no_decoration))
    {

        if (!m_autocomplete.IsInitialized())
        {
            std::vector<std::shared_ptr<Market>> markets = m_tradinator_core->GetAllMarkets();
            bool are_all_markets_ready = true;
            for (const std::shared_ptr<Market>& market : markets)
            {
                if (!market->IsSecurityDataAvailable())
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
                    if (market->IsSecurityDataAvailable())
                    {
                        const AsyncData<std::map<std::string, std::shared_ptr<Security>>>& securities_async_data = market->GetSecurityAsyncData();
                        const std::map<std::string, std::shared_ptr<Security>>& securities = securities_async_data.GetData();

                        for (std::pair<std::string, std::shared_ptr<Security>> pair : securities)
                        {
                            std::shared_ptr<SecurityAutoCompleteItem> item = std::make_shared<SecurityAutoCompleteItem>(pair.second);
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
            if (std::shared_ptr<IAutoCommpleteItem> item = m_autocomplete.Show("##SecuritySearchBar", "##SecuritySearchBar", "Search by company name, symbol or keywords ..."))
            {
                std::shared_ptr<SecurityAutoCompleteItem> security_item = std::dynamic_pointer_cast<SecurityAutoCompleteItem> (item);
                result = security_item->GetSecurity();
                std::cout << "Selected item: " << security_item->GetItemDescription() << std::endl;
            }
        }
    }
    ImGui::End();

    return result;
}

void SecuritiesSearchBar::Shutdown()
{
}
