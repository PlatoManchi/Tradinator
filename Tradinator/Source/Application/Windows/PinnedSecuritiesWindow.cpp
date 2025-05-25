#include "PinnedSecuritiesWindow.h"

#include <format>

#include "imgui.h"
#include "imspinner/imspinner.h"

#include "TradinatorCore.h"
#include "Market/Market.h"

#include "Application/TradinatorSettings.h"
#include "Application/TradinatorApp.h"

PinnedSecuritiesWindow::PinnedSecuritiesWindow(TradinatorApp& tradinator_app)
    : m_tradinator_app(tradinator_app)
{

}

void PinnedSecuritiesWindow::Init(std::shared_ptr<TradinatorCore> tradinator_core)
{
    m_tradinator_core = tradinator_core;
}

void PinnedSecuritiesWindow::Begin()
{

}

void PinnedSecuritiesWindow::Show()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoDocking
        //| ImGuiWindowFlags_AlwaysVerticalScrollbar;
        //ImGuiWindowFlags_NoSavedSettings
        ;

    if (ImGui::Begin("Pinned Securities", nullptr, flags))
    {
        ImGui::SeparatorText("Pinned Securities");
        

        std::vector<TradinatorSettings::PinnedSecurity> pinned_securities = TradinatorSettings::Get().GetPinnedSecuritiesIsinNumbers();
        size_t count = pinned_securities.size();

        if (count == 0)
        {
            ImGui::TextWrapped("Pin any security through security window.");
        }
        else
        {
            bool is_security_data_available = true;
            const std::vector<std::shared_ptr<Market>>& markets = m_tradinator_core->GetAllMarkets();
            for (const std::shared_ptr<Market>& market : markets)
            {
                is_security_data_available &= market->IsSecurityDataAvailable();
            }

            if (is_security_data_available)
            {
                for (TradinatorSettings::PinnedSecurity pinned_security : pinned_securities)
                {
                    std::shared_ptr<Security> security = nullptr;
                    for (const std::shared_ptr<Market>& market : markets)
                    {
                        std::shared_ptr<Security> tmp = market->GetSecurity(pinned_security.symbol);
                        if (tmp && tmp->ISIN_Number() == pinned_security.isin_number)
                        {
                            security = tmp;
                            break;
                        }
                    }
                    if (security)
                    {
                        std::shared_ptr<Market> owning_market = security->GetOwningMarket().lock();
                        if (owning_market)
                        {
                            std::string text = std::format("{}\n{}\n{}", security->Name(), security->Symbol(), owning_market->GetMarketCode());
                            if (ImGui::Selectable(text.c_str()))
                            {
                                m_tradinator_app.ShowSecurityWindow(security);
                            }
                        }
                    }
                }
            }
            else
            {
                ImSpinner::SpinnerIncDots("PinnedSecuritiesWait", 18, 3, ImColor{ 1.f, 1.f, 1.f, 1.f }, 10.0f);
            }
        }
    }
    ImGui::End();
}

void PinnedSecuritiesWindow::Shutdown()
{

}