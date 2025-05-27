#include "SecurityWindow.h"

#include <float.h>
#include <iostream>
#include <algorithm>
#include <initializer_list>

#include  "json/json.h"

#include "Data/Security.h"
#include "Indicators/BollingerBand.h"

#include "Utils.h"
#include "Utils/Utils.h"
#include "Components/IndicatorWrappers.h"
#include "Application/TradinatorSettings.h"

// Getter for IndicatorPoint to draw the plot
ImPlotPoint indicator_plot_point_getter2(int idx, void* data) {
    std::vector<IndicatorPoint>* point_data = (std::vector<IndicatorPoint>*)(data);
    const IndicatorPoint& point = (*point_data)[idx];

    return ImPlotPoint(std::chrono::duration_cast<std::chrono::seconds>(point.date.time_since_epoch()).count(), point.value);
}

SecurityWindow::SecurityWindow(std::shared_ptr<Security> security)
	: m_security(security)
    , m_close(false)
    , m_maximize(false)
    , m_is_dirty(true)
    , date_axis_min(SIZE_MAX)
    , date_axis_max(0)
    , price_axis_min(DBL_MAX)
    , price_axis_max(-DBL_MAX)
    , volume_axis_min(SIZE_MAX)
    , volume_axis_max(0)
    , m_is_price_chart_hovered(false)
    , m_is_volume_chart_hovered(false)
    , m_first_time_chart_limit_x_min(0.0f)
    , m_first_time_chart_limit_x_max(0.0f)
    , m_is_first_time_limit_set(false)
    , m_tooltip_override(false)
    , m_bull_color(0.031f, 0.600f, 0.505f, 1.000f)
    , m_bear_color(0.949f, 0.211f, 0.270f, 1.000f)
{
	m_cached_label_id = m_security->Name() + "##" +m_security->ISIN_Number();

    std::vector<std::unique_ptr<Indicator>> indicators = TradinatorCoreSpace::Utils::GetAvailableIndicators();
    for (std::unique_ptr<Indicator>& indicator : indicators)
    {
        m_available_indicator_wrappers.push_back(TradinatorAppSpace::Utils::GetIndicatorWrapper(std::move(indicator)));
    }

    m_security->SetLockInMemory(true);
    m_security->LoadCandleDataToMemoryAsync();
}

SecurityWindow::~SecurityWindow()
{
    m_security->SetLockInMemory(false);
    m_security->UnloadCandleDataFromMemory();
}

void SecurityWindow::Show()
{
    if (ImGui::Begin(m_cached_label_id.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
    {
        ShowTitle();

        const std::shared_ptr<const AsyncData<CandlesData>>& async_candles_data = m_security->GetCandlesData();


        if (async_candles_data->IsDataReady())
        {
            if (!m_security->IsMemoryInSync())
            {
                // Restrict requsting data from memory
                std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                size_t seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_last_load_request_time).count();
                if (seconds_elapsed > 2)
                {
                    m_last_load_request_time = now;
                    m_security->LoadCandleDataToMemoryAsync();
                }
            }

            if (m_is_dirty)
            {
                m_is_dirty = false;
                RebuildCachedPlotPoints();
            }
        }

        if (!async_candles_data->IsDataReady())
        {
            m_is_dirty = true;

            ImGuiStyle& style = ImGui::GetStyle();
            std::string label = "Candle data loading";
            ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
            ImVec2 avail = ImGui::GetContentRegionAvail();

            ImGui::SetCursorPos(ImVec2(avail.x / 2.0f - label_size.x / 2.0f, avail.y / 2.0f - label_size.y / 2.0f));

            ImGui::Text(label.c_str()); ImGui::SameLine();
            ImSpinner::SpinnerScaleDots(m_security->ISIN_Number().c_str(), 15, 5);
        }
        else if (async_candles_data->GetData().m_dates.size() == 0)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            std::string label = "No candle data available for this security.";
            ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
            ImVec2 avail = ImGui::GetContentRegionAvail();

            ImGui::SetCursorPos(ImVec2(avail.x / 2.0f - label_size.x / 2.0f, avail.y / 2.0f - label_size.y / 2.0f));

            ImGui::Text(label.c_str());
        }
        else
        {
            ImGuiStyle& style = ImGui::GetStyle();
            // close indicators that are marked for closing
            for (size_t id : m_remove_applied_indicator_ids)
            {
                std::erase_if(m_applied_indicator_wrappers, 
                    [id](const std::unique_ptr<IIndicatorWrapper>& other) -> bool
                    {
                        return other->GetID() == id;
                    });
            }
            m_remove_applied_indicator_ids.clear();

            const float PRICE_CHART_MIN_HEIGHT = 400.0f;
            const float VOLUME_CHART_HEIGHT = 250.0f;
            const float SEPERATE_CHARTS_HEIGHT = 250.0f;

            m_price_chart_height = ImGui::GetWindowHeight();
            
            float volume_label_width = ImGui::CalcTextSize(std::format("{:.0f}", m_volume_chart_limits.Y.Max).c_str()).x;
            float price_label_width = ImGui::CalcTextSize(std::format("${:.0f}", m_price_chart_limits.Y.Max).c_str()).x;
            float largest_label_width = std::max(volume_label_width, price_label_width);

            size_t seperate_charts_count = 0;

            for (std::unique_ptr<IIndicatorWrapper>& wrapper : m_applied_indicator_wrappers)
            {
                if (wrapper->ShouldShow() && !wrapper->IsIndicatorOverlayable())
                {
                    GenericChartIndicatorWrapper* chart_wrapper = dynamic_cast<GenericChartIndicatorWrapper*>(wrapper.get());
                    assert(chart_wrapper);

                    ++seperate_charts_count;
                    
                    chart_wrapper->CalculateLabelWidth();
                    if (chart_wrapper->GetLabelWidth() > largest_label_width)
                    {
                        largest_label_width = chart_wrapper->GetLabelWidth();
                    }
                }
            }

            std::string chart_x_axis_label = "Jan\n2018";
            ImVec2 chart_x_axis_label_size = ImGui::CalcTextSize(chart_x_axis_label.c_str());

            double volume_chart_height = VOLUME_CHART_HEIGHT;
            if (seperate_charts_count > 0)
            {
                // since the label won't be shown remove it from height
                volume_chart_height -= chart_x_axis_label_size.y;

                m_price_chart_height -= seperate_charts_count * (SEPERATE_CHARTS_HEIGHT + style.ItemSpacing.y);
                m_price_chart_height -= chart_x_axis_label_size.y; // Last chart should have the label
            }

            m_price_chart_height -= volume_chart_height; // remove volume chart
            

            const CandlesData& candles_data = async_candles_data->GetData();
            size_t count = candles_data.m_dates.size();

            ShowIndicatorsList();

            //ImGui::BulletText("You can create custom plotters or extend ImPlot using implot_internal.h.");
            ImGui::Checkbox("Show Tooltip", &m_show_tool_tip);
            ImGui::SameLine();
            ImGui::Checkbox("Show Patterns", &m_show_patterns);
            ImGui::SameLine();

            ImGui::SameLine(); ImGui::ColorEdit4("##Bull", &m_bull_color.x, ImGuiColorEditFlags_NoInputs);
            ImGui::SameLine(); ImGui::ColorEdit4("##Bear", &m_bear_color.x, ImGuiColorEditFlags_NoInputs);
            ImPlot::GetStyle().UseLocalTime = false;

            m_price_chart_height -= 50; // for checkbox and color selector
            m_price_chart_height -= 90; // miscellaneous


            if (m_price_chart_height < PRICE_CHART_MIN_HEIGHT)
            {
                // Minimum height for the price chart
                m_price_chart_height = PRICE_CHART_MIN_HEIGHT;
            }

            bool is_any_plot_hovered = false;
            if (largest_label_width > (price_label_width + 5.0f))
            {
                ImVec2 plot_padding(largest_label_width - price_label_width + 5.0f, 0); // x = left, y = top
                ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, plot_padding);
            }
            
            if (ImPlot::BeginPlot(std::format("Price Chart##{}", m_security->ISIN_Number()).c_str(), ImVec2(-1, m_price_chart_height), ImPlotFlags_NoLegend)) {
                ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoGridLines, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
                if (!m_is_first_time_limit_set || m_is_highlight_date_index)
                {
                    if (m_first_time_chart_limit_x_min < 1.0f || m_first_time_chart_limit_x_max < 1.0f)
                    {
                        // bad ranges
                        // default range to last 60 days
                        m_first_time_chart_limit_x_max = date_axis_max;
                        m_first_time_chart_limit_x_min = m_first_time_chart_limit_x_max - 60 * 60 * 24 * 60;
                    }
                    // Zoom to the region that was left at
                    ImPlot::SetupAxisLimits(ImAxis_X1, m_first_time_chart_limit_x_min, m_first_time_chart_limit_x_max, ImGuiCond_Always);
                    
                    m_shared_limits.X.Min = m_first_time_chart_limit_x_min;
                    m_shared_limits.X.Max = m_first_time_chart_limit_x_max;

                    m_is_first_time_limit_set = true;
                    m_is_highlight_date_index = false;
                }
                else
                {
                    if (m_is_price_chart_hovered)
                    {
                        ImPlot::SetupAxisLimits(ImAxis_X1, date_axis_min, date_axis_max);
                    }
                    else
                    {
                        ImPlot::SetupAxisLimits(ImAxis_X1, m_shared_limits.X.Min, m_shared_limits.X.Max, ImGuiCond_Always);
                    }
                }
                
                //ImPlot::SetupAxesLimits(date_axis_min, date_axis_max, price_axis_min, price_axis_max);
                
                ImPlot::SetupAxisLimits(ImAxis_Y1, price_axis_min, price_axis_max);
                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, date_axis_min, date_axis_max);
                ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 60 * 60 * 24 * 14, date_axis_max - date_axis_min); // 14 days at min and full chat at max
                ImPlot::SetupAxisFormat(ImAxis_Y1, "$%.0f");
                
                if (m_show_patterns)
                {
                    ShowPatterns(ImGui::GetWindowWidth() - largest_label_width, m_price_chart_height, m_price_chart_limits);
                }

                if (m_is_any_plot_hovered)
                {
                    ImDrawList* draw_list = ImPlot::GetPlotDrawList();

                    float tool_l = ImPlot::PlotToPixels(m_hovered_highlight_l, m_current_hovered_plot_mouse_location.y).x;
                    float tool_r = ImPlot::PlotToPixels(m_hovered_highlight_r, m_current_hovered_plot_mouse_location.y).x;
                    float  tool_t = ImPlot::GetPlotPos().y;
                    float  tool_b = tool_t + ImPlot::GetPlotSize().y;
                    ImPlot::PushPlotClipRect();
                    draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
                    ImPlot::PopPlotClipRect();
                }

                PlotCandlestick(m_security->Name().c_str(), 
                    m_dates.data(), 
                    candles_data.m_opens.data(),
                    candles_data.m_closes.data(),
                    candles_data.m_lows.data(),
                    candles_data.m_highs.data(),
                    candles_data.m_volumes.data(),
                    m_dates.size(), 
                    m_show_tool_tip, 
                    0.25f, 
                    m_bull_color,
                    m_bear_color);

                m_price_chart_limits = ImPlot::GetPlotLimits();

                // sharing plot limits so they are all in sync
                is_any_plot_hovered |= ImPlot::IsPlotHovered();
                if (ImPlot::IsPlotHovered())
                {
                    m_shared_limits = ImPlot::GetPlotLimits();
                    m_is_price_chart_hovered = true;


                    m_current_hovered_plot_mouse_location = ImPlot::GetPlotMousePos();
                    m_current_hovered_plot_mouse_location.x = ImPlot::RoundTime(ImPlotTime::FromDouble(m_current_hovered_plot_mouse_location.x), ImPlotTimeUnit_Day).ToDouble();
                }
                else
                {
                    m_is_price_chart_hovered = false;
                }
                
                ImPlot::EndPlot();
            }
            if (largest_label_width > (price_label_width + 5.0f))
            {
                ImPlot::PopStyleVar();
            }

            //ImPlot::SetNextAxesLimits(ImAxis_X1, m_shared_limits.X.Min, m_shared_limits.X.Max, ImGuiCond_Always);

            if (largest_label_width > (volume_label_width + 5.0f))
            {
                ImVec2 plot_padding(largest_label_width - volume_label_width + 5.0f, 0); // x = left, y = top
                ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, plot_padding);
            }
            if (ImPlot::BeginPlot(std::format("Volume Chart##{}", m_security->ISIN_Number()).c_str(), ImVec2(-1, volume_chart_height), ImPlotFlags_NoLegend | ImPlotFlags_NoTitle)) {
                ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
                //seperate_charts_count
                ImPlotAxisFlags x_axis_flags = ImPlotAxisFlags_NoGridLines;
                if (seperate_charts_count > 0)
                {
                    x_axis_flags |= ImPlotAxisFlags_NoTickLabels;
                }
                ImPlot::SetupAxes(nullptr, nullptr, x_axis_flags, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
                
                if (m_is_volume_chart_hovered)
                {
                    ImPlot::SetupAxisLimits(ImAxis_X1, date_axis_min, date_axis_max);
                }
                else
                {
                    ImPlot::SetupAxisLimits(ImAxis_X1, m_shared_limits.X.Min, m_shared_limits.X.Max, ImGuiCond_Always);
                }
                ImPlot::SetupAxisLimits(ImAxis_Y1, volume_axis_min, volume_axis_max);

                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, date_axis_min, date_axis_max);
                ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 60 * 60 * 24 * 14, date_axis_max - date_axis_min); // 14 days at min and full chat at max

                if (m_is_any_plot_hovered)
                {
                    ImDrawList* draw_list = ImPlot::GetPlotDrawList();

                    float tool_l = ImPlot::PlotToPixels(m_hovered_highlight_l, m_current_hovered_plot_mouse_location.y).x;
                    float tool_r = ImPlot::PlotToPixels(m_hovered_highlight_r, m_current_hovered_plot_mouse_location.y).x;
                    float  tool_t = ImPlot::GetPlotPos().y;
                    float  tool_b = tool_t + ImPlot::GetPlotSize().y;
                    ImPlot::PushPlotClipRect();
                    draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
                    ImPlot::PopPlotClipRect();
                }

                double x_axis_interval = 60 * 60 * 24; // one day in sec
                ImPlot::PlotBars("Volume", m_dates.data(), candles_data.m_volumes.data(), candles_data.m_volumes.size(), x_axis_interval * 0.5);

                m_volume_chart_limits = ImPlot::GetPlotLimits();

                // sharing plot limits so they are all in sync
                is_any_plot_hovered |= ImPlot::IsPlotHovered();
                if (ImPlot::IsPlotHovered())
                {
                    m_shared_limits = ImPlot::GetPlotLimits();
                    m_is_volume_chart_hovered = true;

                    m_current_hovered_plot_mouse_location = ImPlot::GetPlotMousePos();
                    m_current_hovered_plot_mouse_location.x = ImPlot::RoundTime(ImPlotTime::FromDouble(m_current_hovered_plot_mouse_location.x), ImPlotTimeUnit_Day).ToDouble();
                }
                else
                {
                    m_is_volume_chart_hovered = false;
                }

                ImPlot::EndPlot();
            }
            if (largest_label_width > (volume_label_width + 5.0f))
            {
                ImPlot::PopStyleVar();
            }

            size_t applied_seperate_charts_index = 0;
            for (std::unique_ptr<IIndicatorWrapper>& wrapper : m_applied_indicator_wrappers)
            {
                if (wrapper->ShouldShow() && !wrapper->IsIndicatorOverlayable())
                {
                    GenericChartIndicatorWrapper* chart_wrapper = dynamic_cast<GenericChartIndicatorWrapper*>(wrapper.get());
                    assert(chart_wrapper);

                    double chart_height = SEPERATE_CHARTS_HEIGHT;
                    ImPlotAxisFlags x_axis_flags = 0;
                    if (applied_seperate_charts_index == seperate_charts_count - 1)
                    {
                        chart_height += chart_x_axis_label_size.y;
                    }
                    else
                    {
                        // Don't draw xaxis labels unless its the last chart
                        x_axis_flags |= ImPlotAxisFlags_NoTickLabels;
                    }

                    // 20 is magic number to get the graphs aligned lol
                    // its coming from imgui item padding etc
                    // but why is it not 5.0f like for other graphs?
                    if (largest_label_width > (chart_wrapper->GetLabelWidth() + 20.0f))
                    {
                        ImVec2 plot_padding(largest_label_width - chart_wrapper->GetLabelWidth() + 20.0f, 0); // x = left, y = top
                        ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, plot_padding);
                    }

                    chart_wrapper->DrawCustomChart(chart_height, 
                        x_axis_flags, 
                        ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit, 
                        m_shared_limits, 
                        is_any_plot_hovered,
                        m_is_any_plot_hovered,
                        m_current_hovered_plot_mouse_location,
                        m_hovered_highlight_l,
                        m_hovered_highlight_r,
                        m_bull_color,
                        m_bear_color);

                    if (largest_label_width > (chart_wrapper->GetLabelWidth() + 20.0))
                    {
                        ImPlot::PopStyleVar();
                    }
                    ++applied_seperate_charts_index;
                }
            }
            
            if (is_any_plot_hovered)
            {
                float width_percent = 0.25f;
                double x_axis_interval = 60 * 60 * 24; // one day in sec
                double half_width = count > 1 ? x_axis_interval * width_percent : width_percent;

                m_hovered_highlight_l = m_current_hovered_plot_mouse_location.x - half_width * 1.5;
                m_hovered_highlight_r = m_current_hovered_plot_mouse_location.x + half_width * 1.5;
            }
            
            m_is_any_plot_hovered = is_any_plot_hovered;
        }
    }
    ImGui::End();
}

void SecurityWindow::ShowIndicatorsList()
{
    float indicator_height = 55.0f;
    float indicator_width = 600.0f;
    
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 item_spacing = style.ItemSpacing;

    float window_width = ImGui::GetWindowWidth();
    float table_width = (window_width - item_spacing.x * 3.0f) / 2.0f;
    int  column_count = std::max((int)(table_width / indicator_width), 1);

    float max_height = 300.0f;

    int total_applied_indicators = m_applied_indicator_wrappers.size();
    int total_available_indicators = m_available_indicator_wrappers.size();
    int max_indicators = std::max(total_applied_indicators, total_available_indicators);
    
    int num_of_rows = (max_indicators % column_count) == 0 ? (max_indicators / column_count) : (max_indicators / column_count) + 1;

    float child_window_height = num_of_rows * indicator_height > 200.0f ? 200.0f : num_of_rows * indicator_height;
    child_window_height += 50; // adding the height for the heading text

    m_price_chart_height -= child_window_height; // remove this height from available space for price chart

    /// @begin Child
    ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
    ImGui::BeginChild("child1", { table_width, child_window_height }, ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings);
    {
        /// @separator
        ImGui::SeparatorText("Applied Indicators");
        /// @begin Table
        if (ImGui::BeginTable("AppliedIndicators", column_count, ImGuiTableFlags_ScrollY, { -1, 0 }))
        {
            int row = -1;
            for (int i = 0; i < total_applied_indicators; ++i)
            {
                int tmp_row = i / column_count;
                if (tmp_row != row)
                {
                    row = tmp_row;
                    ImGui::TableNextRow(0, 0);
                }

                ImGui::TableSetColumnIndex(i % column_count);

                if (m_applied_indicator_wrappers[i]->DrawAsAppliedIndicator())
                {
                    m_remove_applied_indicator_ids.push_back(m_applied_indicator_wrappers[i]->GetID());
                }
            }
            ImGui::EndTable();
        }
        /// @end Table

        ImGui::EndChild();
    }
    /// @end Child

    /// @begin Child
    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
    ImGui::BeginChild("child3", { table_width, child_window_height }, ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings);
    {
        /// @separator
        ImGui::SeparatorText("Available Indicators");
        /// @begin Table
        if (ImGui::BeginTable("AvailableIndicators", column_count, ImGuiTableFlags_ScrollY, { -1, 0 }))
        {
            int row = -1;
            for (int i = 0; i < total_available_indicators; ++i)
            {
                int tmp_row = i / column_count;
                if (tmp_row != row)
                {
                    row = tmp_row;
                    ImGui::TableNextRow(0, 0);
                }

                ImGui::TableSetColumnIndex(i % column_count);

                bool can_apply = CanApplyIndicatorOfType(m_available_indicator_wrappers[i]->IndicatorType());
                if (!can_apply)
                {
                    ImGui::BeginDisabled();
                }
                if (m_available_indicator_wrappers[i]->DrawAsAvailableIndicator())
                {
                    std::unique_ptr<IIndicatorWrapper> clone_wrapper = m_available_indicator_wrappers[i]->Clone();
                    clone_wrapper->SetSecurity(m_security);
                    clone_wrapper->Calculate();

                    m_applied_indicator_wrappers.push_back(std::move(clone_wrapper));
                }
                if (!can_apply)
                {
                    ImGui::EndDisabled();
                }
            }
            ImGui::EndTable();
        }
        /// @end Table

        ImGui::EndChild();
    }
    /// @end Child
}

bool SecurityWindow::CanApplyIndicatorOfType(EIndicatorType type)
{
    auto applied_itr = std::find_if(m_applied_indicator_wrappers.begin(), m_applied_indicator_wrappers.end(),
        [type](const std::unique_ptr<IIndicatorWrapper>& other) -> bool
        {
            return other->IndicatorType() == type;
        });
    
    if (applied_itr != m_applied_indicator_wrappers.end())
    {
        if ((*applied_itr)->IsSingleInstanceType())
        {
            return false;
        }
    }

    return true;
}

void SecurityWindow::ShowTitle()
{
    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImGuiStyle& style = ImGui::GetStyle();
    float item_spacing = style.ItemSpacing.x;

    ImVec2 title_size = ImGui::CalcTextSize(m_security->Name().c_str());
    ImVec2 close_text_size = ImGui::CalcTextSize(" X ");

    ImGuiID id = ImGui::GetID(m_cached_label_id.c_str());
    float top = ImGui::GetCursorPosY();
    ImGui::Text(m_security->Name().c_str());


    const std::vector<TradinatorSettings::PinnedSecurity>& pinned = TradinatorSettings::Get().GetPinnedSecuritiesIsinNumbers();
    std::string button_txt = "Pin";
    bool is_pinned = false;
    for (const TradinatorSettings::PinnedSecurity& security : pinned)
    {
        if (security.isin_number == m_security->ISIN_Number())
        {
            is_pinned = true;
            button_txt = "Unpin";
            break;
        }
    }


    ImVec2 pin_text_size = ImGui::CalcTextSize(button_txt.c_str());
    
    
    ImVec2 button_size = ImVec2(70, 57);

    ImVec2 available_space = ImGui::GetContentRegionAvail();
    //ImGui::SetCursorPosX(available_space.x - button_size.x);
    ImVec2 prev_cursor_pos = ImGui::GetCursorPos();
    

    // remove padding after x button so that there is no scrolling
    // but this isn't working. 
    // TODO: Figure out how to remove padding after x button so that page won't scroll horizontally
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); 

    ImGui::SetCursorPos(ImVec2(title_size.x + item_spacing * 2.0f, top - 16));
    if (ImGui::Button(button_txt.c_str(), ImVec2(pin_text_size.x + item_spacing * 2.0f, 57.0f)))
    {
        if (is_pinned)
        {
            TradinatorSettings::Get().RemovePinnedSecuritiesIsinNumber({ m_security->Symbol(), m_security->ISIN_Number() });
        }
        else
        {
            TradinatorSettings::Get().AddPinnedSecuritiesIsinNumber({ m_security->Symbol(), m_security->ISIN_Number() });
        }
    }

    ImGui::SetCursorPos(ImVec2(window_size.x - button_size.x, top - 16));
    if (ImGui::Button(" X ", button_size))
    {
        m_close = true;
    }
    ImGui::SetCursorPos(ImVec2(window_size.x - button_size.x * 2.0f, top - 16));
    if (ImGui::Button(" [] ", button_size))
    {
        m_maximize = true;
    }
    ImGui::PopStyleVar();
    ImGui::SetCursorPos(prev_cursor_pos);

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

    m_price_chart_height -= (button_size.x + 20); // title height and sepertor
}



void SecurityWindow::RebuildCachedPlotPoints()
{
    const std::shared_ptr<const AsyncData<CandlesData>>& async_candles_data = m_security->GetCandlesData();
    const CandlesData& candles_data = async_candles_data->GetData();

    size_t count = candles_data.m_dates.size();

    // don't care about previous data stored in cache
    m_dates = std::vector<uint64_t>();
    m_dates.reserve(count);
    
    date_axis_min = SIZE_MAX;
    date_axis_max = 0;
    price_axis_min = DBL_MAX;
    price_axis_max = -DBL_MAX;
    volume_axis_min = SIZE_MAX;
    volume_axis_max = 0;

    for (size_t i = 0; i < count; ++i)
    {
        std::chrono::system_clock::rep date = std::chrono::duration_cast<std::chrono::seconds>(candles_data.m_dates[i].time_since_epoch()).count();
        m_dates.push_back(date);

        if (date_axis_min > date)
            date_axis_min = date;
        if (date_axis_max < date)
            date_axis_max = date;

        double max = std::max({ candles_data.m_opens[i], candles_data.m_highs[i], candles_data.m_closes[i], candles_data.m_lows[i]});
        double min = std::min({ candles_data.m_opens[i], candles_data.m_highs[i], candles_data.m_closes[i], candles_data.m_lows[i] });

        if (price_axis_min > min)
            price_axis_min = min;
        if (price_axis_max < max)
            price_axis_max = max;

        if (volume_axis_min > candles_data.m_volumes[i])
            volume_axis_min = candles_data.m_volumes[i];
        if (volume_axis_max < candles_data.m_volumes[i])
            volume_axis_max = candles_data.m_volumes[i];
    }

    // Adding one day padding at start and end of graph
    date_axis_min -= 60 * 60 * 24;
    date_axis_max += 60 * 60 * 24;

    m_shared_limits.X.Min = date_axis_min;
    m_shared_limits.X.Max = date_axis_max;

    for (std::unique_ptr<IIndicatorWrapper>& wrapper : m_applied_indicator_wrappers)
    {
        wrapper->Calculate();
    }

    if (m_is_highlight_date_index)
    {
        std::vector<uint64_t>::iterator min_itr = std::min_element(m_highlight_date_index.begin(), m_highlight_date_index.end());
        std::vector<uint64_t>::iterator max_itr = std::max_element(m_highlight_date_index.begin(), m_highlight_date_index.end());

        if (min_itr != m_highlight_date_index.end() && max_itr != m_highlight_date_index.end())
        {
            m_first_time_chart_limit_x_min = m_dates[*min_itr];
            m_first_time_chart_limit_x_max = m_dates[*max_itr];
        }
    }
}

void SecurityWindow::PlotCandlestick(const char* label_id, const size_t* xs, const double* opens, const double* closes, const double* lows, const double* highs, const uint64_t* volumes, int count, bool tooltip, float width_percent, ImVec4 bullCol, ImVec4 bearCol) {

    // get ImGui window DrawList
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    // calc real value width
    double x_axis_interval = 60 * 60 * 24; // one day in sec
    double half_width = count > 1 ? x_axis_interval * width_percent : width_percent;

    // custom tool
    if (ImPlot::IsPlotHovered() && tooltip && !m_tooltip_override)
    {
        ImPlotPoint mouse = ImPlot::GetPlotMousePos();
        mouse.x = ImPlot::RoundTime(ImPlotTime::FromDouble(mouse.x), ImPlotTimeUnit_Day).ToDouble();
        
        // find mouse location index
        int idx = BinarySearch(xs, 0, count - 1, mouse.x);
        // render tool tip (won't be affected by plot clip rect)
        if (idx != -1) {
            ImGui::BeginTooltip();
            char buff[32];
            ImPlot::FormatDate(ImPlotTime::FromDouble(xs[idx]), buff, 32, ImPlotDateFmt_DayMoYr, ImPlot::GetStyle().UseISO8601);

            std::stringstream string_stream;

            std::string text = std::format(
                "Day:    {}\n\n"
                "Open:   ${}\n"
                "Close:  ${}\n"
                "Low:    ${}\n"
                "High:   ${}\n\n"
                "Volume: {}", buff, opens[idx], closes[idx], lows[idx], highs[idx], volumes[idx]);

            string_stream << text;
            for (std::unique_ptr<IIndicatorWrapper>& wrapper : m_applied_indicator_wrappers)
            {
                if (wrapper->ShouldShow())
                {
                    string_stream << "\n\n";
                    string_stream << wrapper->GetHumanReadableValueAt(idx);
                }
            }

            std::string tool_tip = string_stream.str();
            
            /*ImGui::Text("Day:    %s", buff);
            ImGui::Text("Open:   $%.2f", opens[idx]);
            ImGui::Text("Close:  $%.2f", closes[idx]);
            ImGui::Text("Low:    $%.2f", lows[idx]);
            ImGui::Text("High:   $%.2f", highs[idx]);
            ImGui::Text("\nVolume: %.0f", m_volumes[idx]);*/
            ImGui::Text(tool_tip.c_str());

            ImGui::EndTooltip();
        }
    }

    // begin plot item
    if (ImPlot::BeginItem(label_id)) {
        // override legend icon color
        ImPlot::GetCurrentItem()->Color = IM_COL32(64, 64, 64, 255);
        // fit data if requested
        if (ImPlot::FitThisFrame()) {
            for (int i = 0; i < count; ++i) {
                ImPlot::FitPoint(ImPlotPoint(xs[i], lows[i]));
                ImPlot::FitPoint(ImPlotPoint(xs[i], highs[i]));
            }
        }

        for (std::unique_ptr<IIndicatorWrapper>& wrapper : m_applied_indicator_wrappers)
        {
            if (wrapper->ShouldShow() && wrapper->IsIndicatorOverlayable())
            {
                wrapper->PlotPreCandle(m_bull_color, m_bear_color);
            }
        }


        // render data
        for (int i = 0; i < count; ++i) {
            ImVec2 open_pos = ImPlot::PlotToPixels(xs[i] - half_width, opens[i]);
            ImVec2 close_pos = ImPlot::PlotToPixels(xs[i] + half_width, closes[i]);
            ImVec2 low_pos = ImPlot::PlotToPixels(xs[i], lows[i]);
            ImVec2 high_pos = ImPlot::PlotToPixels(xs[i], highs[i]);
            ImU32 color = ImGui::GetColorU32(opens[i] > closes[i] ? bearCol : bullCol);
            draw_list->AddLine(low_pos, high_pos, color);
            
            if (fabs(opens[i] - closes[i]) < LDBL_EPSILON)
            {
                draw_list->AddLine(open_pos, close_pos, color);
            }
            else
            {
                draw_list->AddRectFilled(open_pos, close_pos, color);
            }
        }

        for (std::unique_ptr<IIndicatorWrapper>& wrapper : m_applied_indicator_wrappers)
        {
            if (wrapper->ShouldShow() && wrapper->IsIndicatorOverlayable())
            {
                wrapper->PlotPostCandle(m_bull_color, m_bear_color);
            }
        }

        // end plot item
        ImPlot::EndItem();
    }

    
}

void SecurityWindow::ShowPatterns(float chart_width, float chart_height, ImPlotRect chart_limits)
{
    m_tooltip_override = false;

    if (m_security->GetNewsPointsData()->IsDataReady() && m_security->GetCandlesData()->IsDataReady())
    {
        if (ImPlot::BeginItem("Patterns")) {
            const CandlesData& candle_data = m_security->GetCandlesData()->GetData();
            const NewsPointVectorType& news_points = m_security->GetNewsPointsData()->GetData();

            ImDrawList* draw_list = ImPlot::GetPlotDrawList();

            uint64_t candles_count = candle_data.m_dates.size();
            uint64_t news_count = news_points.size();
            for (uint64_t i = 0; i < news_count; ++i)
            {
                if (!TradinatorSettings::Get().GetPatternVisibility(news_points[i].m_pattern))
                {
                    continue;
                }

                if (news_points[i].m_date_range.size() > 0)
                {
                    if (news_points[i].m_date_range[0] < candles_count)
                    {
                        //std::chrono::system_clock::time_point first_date = candle_data.m_dates[news_points[i].m_date_range[0]];
                        std::chrono::system_clock::rep cummulative_x_date = 0;
                        //std::chrono::system_clock::rep date = std::chrono::duration_cast<std::chrono::seconds>(first_date.time_since_epoch()).count();

                        double top = -DBL_MAX, bottom = DBL_MAX;
                        for (uint64_t index : news_points[i].m_date_range)
                        {
                            if (index < candles_count)
                            {
                                std::chrono::system_clock::time_point date = candle_data.m_dates[index];
                                cummulative_x_date += std::chrono::duration_cast<std::chrono::seconds>(date.time_since_epoch()).count();

                                top = std::max({ top, candle_data.m_highs[index], candle_data.m_opens[index], candle_data.m_closes[index], candle_data.m_lows[index] });
                                bottom = std::min({ bottom, candle_data.m_highs[index], candle_data.m_opens[index], candle_data.m_closes[index], candle_data.m_lows[index] });
                            }
                        }

                        std::chrono::system_clock::time_point left_date = candle_data.m_dates[news_points[i].m_date_range[0]];
                        std::chrono::system_clock::time_point right_date = candle_data.m_dates[news_points[i].m_date_range[news_points[i].m_date_range.size() - 1]];
                        std::chrono::system_clock::rep left = std::chrono::duration_cast<std::chrono::seconds>(left_date.time_since_epoch()).count() - 60 * 60 * 12;
                        std::chrono::system_clock::rep right = std::chrono::duration_cast<std::chrono::seconds>(right_date.time_since_epoch()).count() + 60 * 60 * 12;

                        ImVec2 upper_left = ImPlot::PlotToPixels(left, top);
                        ImVec2 lower_right = ImPlot::PlotToPixels(right, bottom);
                        ImVec2 upper_right = ImPlot::PlotToPixels(right, top);
                        ImVec2 lower_left = ImVec2(left, bottom);

                        upper_left.y -= 15.0f;
                        lower_right.y += 15.0f;

                        float offset = ((chart_limits.Y.Max - chart_limits.Y.Min) / chart_height) * 50.0f;

                        double top_tmp = top + offset;
                        double bottom_tmp = bottom - offset;

                        if (ImPlot::FitThisFrame())
                        {
                            ImVec2 upper_left_tmp(left, top_tmp);
                            ImVec2 bottom_right_tmp(right, bottom_tmp);

                            ImPlot::FitPoint(upper_left_tmp);
                            ImPlot::FitPoint(bottom_right_tmp);
                        }

                        TradinatorAppSpace::EPatternNatureType type = TradinatorAppSpace::Utils::GetPatternNatureType(news_points[i].m_pattern);
                        ImVec4 color = ImVec4(76.0 / 255.0, 144.0 / 255.0, 176.0 / 255.0, 1.0f); //  neutral color
                        if (type == TradinatorAppSpace::EPatternNatureType::BULL)
                        {
                            color = m_bull_color;
                        }
                        else if (type == TradinatorAppSpace::EPatternNatureType::BEAR)
                        {
                            color = m_bear_color;
                        }

                        bool is_annotation_hovered = false;

                        float annotation_x = cummulative_x_date / news_points[i].m_date_range.size();
                        float annotation_y = bottom - offset / 2.0f;
                        if (!m_tooltip_override)
                        {
                            ImPlotPoint mouse = ImPlot::GetPlotMousePos();

                            float ratio_x = (chart_limits.X.Max - chart_limits.X.Min) / chart_width;
                            float ratio_y = (chart_limits.Y.Max - chart_limits.Y.Min) / chart_height;

                            if (std::abs(mouse.x - annotation_x) < ratio_x * 30 &&
                                std::abs(mouse.y - annotation_y) < ratio_y * 20)
                            {
                                m_tooltip_override = true;
                                is_annotation_hovered = true;
                                ImGui::BeginTooltip();
                                ImGui::Text(TradinatorCoreSpace::Utils::GetPatternShortDescription(news_points[i].m_pattern).c_str());
                                ImGui::EndTooltip();
                            }
                            else
                            {
                                m_tooltip_override = false;
                            }
                        }

                        color.w = 0.5f; // alpha for border
                        draw_list->AddRect(upper_left, lower_right, ImGui::GetColorU32(color), 0.0f, ImDrawFlags_RoundCornersNone, 5.0f);
                        if (is_annotation_hovered)
                        {
                            color.w = 0.1f; // alpha for fill
                            draw_list->AddRectFilled(upper_left, lower_right, ImGui::GetColorU32(color), 0.0f, ImDrawFlags_RoundCornersNone);
                        }


                        color.w = 1.0f; // alpha for annotation
                        ImPlot::Annotation(annotation_x, annotation_y, color, ImVec2(0, 0), false, " P ");
                    }
                }
            }


            ImPlot::EndItem();
        }
    }
}

//template <typename T>
int SecurityWindow::BinarySearch(const size_t* arr, int l, int r, double x) {
    if (r >= l) {
        int mid = l + (r - l) / 2;
        if (arr[mid] == x)
        {
            return mid;
        }
        if (arr[mid] > x)
            return BinarySearch(arr, l, mid - 1, x);
        return BinarySearch(arr, mid + 1, r, x);
    }
    return -1;
}



void SecurityWindow::HighlightDateIndex(std::vector<uint64_t> dates_index_range)
{
    m_is_highlight_date_index = true;
    m_highlight_date_index = dates_index_range;
}


Json::Value SecurityWindow::GetSecurityStatus()
{
    Json::Value result;

    result["Symbol"] = m_security->Symbol();
    result["ISIN"] = m_security->ISIN_Number();
    result["Range"]["Min"] = m_price_chart_limits.X.Min;
    result["Range"]["Max"] = m_price_chart_limits.X.Max;
    result["ShowToolTip"] = m_show_tool_tip;
    result["ShowPatterns"] = m_show_patterns;

    result["BullColor"]["R"] = m_bull_color.x;
    result["BullColor"]["G"] = m_bull_color.y;
    result["BullColor"]["B"] = m_bull_color.z;
    result["BullColor"]["A"] = m_bull_color.w;

    result["BearColor"]["R"] = m_bear_color.x;
    result["BearColor"]["G"] = m_bear_color.y;
    result["BearColor"]["B"] = m_bear_color.z;
    result["BearColor"]["A"] = m_bear_color.w;

    Json::Value applied_indicators(Json::arrayValue);

    for (std::unique_ptr<IIndicatorWrapper>& wrapper : m_applied_indicator_wrappers)
    {
        applied_indicators.append(wrapper->ToJson());
    }

    result["Applied_Indicators"] = applied_indicators;

    return result;
}

void SecurityWindow::SetSecurityStatus(Json::Value status)
{
    m_first_time_chart_limit_x_min = status["Range"]["Min"].asFloat();
    m_first_time_chart_limit_x_max = status["Range"]["Max"].asFloat();
    m_show_tool_tip = status["ShowToolTip"].asBool();
    m_show_patterns = status.find("ShowPatterns") ? status["ShowPatterns"].asBool() : true;
    
    m_bull_color.x = status["BullColor"]["R"].asFloat();
    m_bull_color.y = status["BullColor"]["G"].asFloat();
    m_bull_color.z = status["BullColor"]["B"].asFloat();
    m_bull_color.w = status["BullColor"]["A"].asFloat();

    m_bear_color.x = status["BearColor"]["R"].asFloat();
    m_bear_color.y = status["BearColor"]["G"].asFloat();
    m_bear_color.z = status["BearColor"]["B"].asFloat();
    m_bear_color.w = status["BearColor"]["A"].asFloat();


    Json::Value applied_indicators = status["Applied_Indicators"];
    Json::Value::ArrayIndex count = applied_indicators.size();

    for (int i = 0; i < count; ++i)
    {
        EIndicatorType type = TradinatorAppSpace::Utils::GetIndicatorType(applied_indicators[i]["Name"].asString());
        std::unique_ptr<IIndicatorWrapper> wrapper = TradinatorAppSpace::Utils::GetIndicatorWrapper(type);
        if (wrapper)
        {
            wrapper->FromJson(applied_indicators[i]);
            wrapper->SetSecurity(m_security);

            m_applied_indicator_wrappers.push_back(std::move(wrapper));
        }
    }
}