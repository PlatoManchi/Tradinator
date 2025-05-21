#include "DashboardWindow.h"


#include <format>
#include <chrono>

#include "imgui.h"
#include "imspinner/imspinner.h"

#include "TradinatorCore.h"
#include "Data/AsyncData.h"
#include "Data/Security.h"

#include "Utils.h"
#include "Application/TradinatorApp.h"


DashboardWindow::DashboardWindow(TradinatorApp& tradinator_app)
    : m_tradinator_app(tradinator_app)
{
}

void DashboardWindow::Init(std::shared_ptr<TradinatorCore> tradinator_core)
{
    m_tradinator_core = tradinator_core;
}

void DashboardWindow::Begin()
{
}

void DashboardWindow::Show()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoDocking
        //| ImGuiWindowFlags_AlwaysVerticalScrollbar;
        //ImGuiWindowFlags_NoSavedSettings
        ;

    //ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    if (ImGui::Begin("Main Window", nullptr, flags))
    {

        ImGui::SeparatorText("Newest 10 Listings");

        const AsyncData<std::vector<std::weak_ptr<Security>>>& ten_newest = m_tradinator_core->GetTenNewestIPOs();

        if (ten_newest.IsDataReady())
        {
            if (ImGui::BeginTable("table1", 8, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersOuterV, { -1, 0 }))
            {
                ImGui::TableSetupColumn("Date Of Listing", ImGuiTableColumnFlags_WidthFixed, 0);
                ImGui::TableSetupColumn("Symbol", ImGuiTableColumnFlags_WidthFixed, 0);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 0);
                ImGui::TableSetupColumn("Series", ImGuiTableColumnFlags_WidthFixed, 0);
                ImGui::TableSetupColumn("Paid Up Value", ImGuiTableColumnFlags_WidthFixed, 0);
                ImGui::TableSetupColumn("Market Lot", ImGuiTableColumnFlags_WidthFixed, 0);
                ImGui::TableSetupColumn("ISIN Number", ImGuiTableColumnFlags_WidthFixed, 0);
                ImGui::TableSetupColumn("Face Value", ImGuiTableColumnFlags_WidthFixed, 0);
                ImGui::TableHeadersRow();

                const std::vector<std::weak_ptr<Security>>& list = ten_newest.GetData();
                for (std::weak_ptr<Security> security_weak : list)
                {
                    std::shared_ptr<Security> security = security_weak.lock();
                    if (security)
                    {
                        ImGui::TableNextRow(0, 0);

                        ImGui::TableSetColumnIndex(0);
                        //ImGui::Text(std::format("{:%d-%b-%y}", security->DateOfListing()).c_str());
                        bool is_selected = false;
                        ImGui::Selectable(std::format("{:%d-%b-%Y}##{}{}", security->DateOfListing(), security->ISIN_Number(), "TenNewest").c_str(), &is_selected, ImGuiSelectableFlags_SpanAllColumns);
                        if (ImGui::IsItemClicked())
                        {
                            m_tradinator_app.ShowSecurityWindow(security);
                        }


                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushID(std::format("{}##{}{}", security->Symbol(), security->ISIN_Number(), "TenNewest").c_str());
                        ImGui::Text(security->Symbol().c_str());
                        ImGui::PopID();

                        ImGui::TableSetColumnIndex(2);
                        ImGui::PushID(std::format("{}##{}{}", security->Name(), security->ISIN_Number(), "TenNewest").c_str());
                        ImGui::Text(security->Name().c_str());
                        ImGui::PopID();

                        ImGui::TableSetColumnIndex(3);
                        ImGui::PushID(std::format("{}##{}{}", security->Series(), security->ISIN_Number(), "TenNewest").c_str());
                        ImGui::Text(security->Series().c_str());
                        ImGui::PopID();

                        ImGui::TableSetColumnIndex(4);
                        ImGui::PushID(std::format("{}##{}{}", security->PaidUpValue(), security->ISIN_Number(), "PaidUpValue TenNewest").c_str());
                        ImGui::Text(std::format("{}", security->PaidUpValue()).c_str());
                        ImGui::PopID();

                        ImGui::TableSetColumnIndex(5);
                        ImGui::PushID(std::format("{}##{}{}", security->MarketLot(), security->ISIN_Number(), "MarkerLot TenNewest").c_str());
                        ImGui::Text(std::format("{}", security->MarketLot()).c_str());
                        ImGui::PopID();

                        ImGui::TableSetColumnIndex(6);
                        ImGui::PushID(std::format("{}##{}", security->ISIN_Number(), "TenNewest").c_str());
                        ImGui::Text(security->ISIN_Number().c_str());
                        ImGui::PopID();

                        ImGui::TableSetColumnIndex(7);
                        ImGui::PushID(std::format("{}##{}{}", security->FaceValue(), security->ISIN_Number(), "FaceValue TenNewest").c_str());
                        ImGui::Text(std::format("{}", security->FaceValue()).c_str());
                        ImGui::PopID();
                    }
                }

                ImGui::EndTable();
            }
        }
        else
        {
            ImSpinner::SpinnerIncDots("10 Newest IPOs", 18, 3, ImColor{ 1.f, 1.f, 1.f, 1.f }, 10.0f);
        }
    }
    ImGui::End();
}

void DashboardWindow::Shutdown()
{
}
