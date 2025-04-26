#include "CounterWindow.h"

#include <float.h>
#include <iostream>

#include  "json/json.h"

#include "Data/Counter.h"
#include "Indicators/BollingerBand.h"

#include "Utils.h"
#include "Utils/Utils.h"

size_t CounterWindow::_INCREMENTAL_ID_ = 0;

// Getter for IndicatorPoint to draw the plot
ImPlotPoint indicator_plot_point_getter(int idx, void* data) {
    std::vector<IndicatorPoint>* point_data = (std::vector<IndicatorPoint>*)(data);
    const IndicatorPoint& point = (*point_data)[idx];

    return ImPlotPoint(std::chrono::duration_cast<std::chrono::seconds>(point.date.time_since_epoch()).count(), point.value);
}

CounterWindow::CounterWindow(std::shared_ptr<Counter> counter)
	: m_counter(counter)
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
{
	m_cached_label_id = m_counter->Name() + "##" +m_counter->ISIN_Number();

    m_available_indicators = TradinatorCoreSpace::Utils::GetAvailableIndicators();

    m_counter->LoadCandleDataToMemory();
}

CounterWindow::~CounterWindow()
{
    m_counter->UnloadCandleDataFromMemory();
}

void CounterWindow::Show()
{
    ImGui::GetFocusID();
    if (ImGui::Begin(m_cached_label_id.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
    {
        ShowTitle();

        const std::shared_ptr<const AsyncData<AsyncCandleData>>& candle_data = m_counter->GetCandleData();


        if (candle_data->IsDataReady() && m_is_dirty)
        {
            m_is_dirty = false;
            RebuildCachedPlotPoints();
        }

        if (!candle_data->IsDataReady())
        {
            m_is_dirty = true;

            ImGuiStyle& style = ImGui::GetStyle();
            std::string label = "Candle data loading";
            ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
            ImVec2 avail = ImGui::GetContentRegionAvail();

            ImGui::SetCursorPos(ImVec2(avail.x / 2.0f - label_size.x / 2.0f, avail.y / 2.0f - label_size.y / 2.0f));

            ImGui::Text(label.c_str()); ImGui::SameLine();
            ImSpinner::SpinnerScaleDots(m_counter->ISIN_Number().c_str(), 15, 5);
        }
        else if (candle_data->GetData().size() == 0)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            std::string label = "No candle data available for this counter.";
            ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
            ImVec2 avail = ImGui::GetContentRegionAvail();

            ImGui::SetCursorPos(ImVec2(avail.x / 2.0f - label_size.x / 2.0f, avail.y / 2.0f - label_size.y / 2.0f));

            ImGui::Text(label.c_str());
        }
        else
        {
            ImGuiStyle& style = ImGui::GetStyle();
            // close indicators that are marked for closing
            for (std::shared_ptr<Indicator> indicator_to_remove : m_remove_applied_indicators)
            {
                m_applied_indicators_data.erase(indicator_to_remove);
            }
            m_remove_applied_indicators.clear();


            const float PRICE_CHART_MIN_HEIGHT = 400.0f;
            const float VOLUME_CHART_HEIGHT = 250.0f;
            const float SEPERATE_CHARTS_HEIGHT = 250.0f;

            m_price_chart_height = ImGui::GetWindowHeight();
            
            float volume_label_width = ImGui::CalcTextSize(std::format("{:.0f}", m_volume_chart_limits.Y.Max).c_str()).x;
            float price_label_width = ImGui::CalcTextSize(std::format("${:.0f}", m_price_chart_limits.Y.Max).c_str()).x;
            float largest_label_width = std::max(volume_label_width, price_label_width);

            size_t seperate_charts_count = 0;

            for (auto& pair : m_applied_indicators_data)
            {
                if (pair.second.m_show && !TradinatorAppSpace::Utils::IsIndicatorOverlayable(pair.first->IndicatorType()))
                {
                    ++seperate_charts_count;

                    float max_range_width = ImGui::CalcTextSize(std::format("${:.0f}", pair.second.m_chart_limits.Y.Max).c_str()).x;
                    float min_range_width = ImGui::CalcTextSize(std::format("${:.0f}", pair.second.m_chart_limits.Y.Min).c_str()).x;

                    pair.second.m_label_width = std::max(max_range_width, min_range_width);
                    if (pair.second.m_label_width > largest_label_width)
                    {
                        largest_label_width = pair.second.m_label_width;
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
            


            size_t count = candle_data->GetData().size();

            ShowIndicatorsList();

            //ImGui::BulletText("You can create custom plotters or extend ImPlot using implot_internal.h.");
            static bool tooltip = true;
            ImGui::Checkbox("Show Tooltip", &tooltip);
            ImGui::SameLine();
            static ImVec4 bullCol = ImVec4(0.031f, 0.600f, 0.505f, 1.000f);
            static ImVec4 bearCol = ImVec4(0.949f, 0.211f, 0.270f, 1.000f);
            ImGui::SameLine(); ImGui::ColorEdit4("##Bull", &bullCol.x, ImGuiColorEditFlags_NoInputs);
            ImGui::SameLine(); ImGui::ColorEdit4("##Bear", &bearCol.x, ImGuiColorEditFlags_NoInputs);
            ImPlot::GetStyle().UseLocalTime = false;

            m_price_chart_height -= 50; // for checkbox and color selector
            m_price_chart_height -= 90; // miscellaneous


            if (m_price_chart_height < PRICE_CHART_MIN_HEIGHT)
            {
                // Minimum height for the price chart
                m_price_chart_height = PRICE_CHART_MIN_HEIGHT;
            }

            
            if (largest_label_width > (price_label_width + 5.0f))
            {
                ImVec2 plot_padding(largest_label_width - price_label_width + 5.0f, 0); // x = left, y = top
                ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, plot_padding);
            }
            
            if (ImPlot::BeginPlot(std::format("Price Chart##{}", m_counter->ISIN_Number()).c_str(), ImVec2(-1, m_price_chart_height), ImPlotFlags_NoLegend)) {
                ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
                if (m_is_price_chart_hovered)
                {
                    ImPlot::SetupAxisLimits(ImAxis_X1, date_axis_min, date_axis_max);
                }
                else
                {
                    ImPlot::SetupAxisLimits(ImAxis_X1, m_shared_limits.X.Min, m_shared_limits.X.Max, ImGuiCond_Always);
                }
                //ImPlot::SetupAxesLimits(date_axis_min, date_axis_max, price_axis_min, price_axis_max);
                
                ImPlot::SetupAxisLimits(ImAxis_Y1, price_axis_min, price_axis_max);
                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, date_axis_min, date_axis_max);
                ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 60 * 60 * 24 * 14, date_axis_max - date_axis_min); // 14 days at min and full chat at max
                ImPlot::SetupAxisFormat(ImAxis_Y1, "$%.0f");
                
                PlotCandlestick(m_counter->Name().c_str(), m_dates.data(), m_opens.data(), m_closes.data(), m_lows.data(), m_highes.data(), m_dates.size(), tooltip, 0.25f, bullCol, bearCol);

                m_price_chart_limits = ImPlot::GetPlotLimits();

                // sharing plot limits so they are all in sync
                if (ImPlot::IsPlotHovered())
                {
                    m_shared_limits = ImPlot::GetPlotLimits();
                    m_is_price_chart_hovered = true;
                }
                else
                {
                    m_is_price_chart_hovered = false;
                }

                ImPlot::EndPlot();
            }
            if (largest_label_width > price_label_width)
            {
                ImPlot::PopStyleVar();
            }

            //ImPlot::SetNextAxesLimits(ImAxis_X1, m_shared_limits.X.Min, m_shared_limits.X.Max, ImGuiCond_Always);

            if (largest_label_width > (volume_label_width + 5.0f))
            {
                ImVec2 plot_padding(largest_label_width - volume_label_width + 5.0f, 0); // x = left, y = top
                ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, plot_padding);
            }
            if (ImPlot::BeginPlot(std::format("Volume Chart##{}", m_counter->ISIN_Number()).c_str(), ImVec2(-1, volume_chart_height), ImPlotFlags_NoLegend | ImPlotFlags_CanvasOnly)) {
                ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
                //seperate_charts_count
                ImPlotAxisFlags x_axis_flags = 0;
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

                double x_axis_interval = 60 * 60 * 24; // one day in sec
                ImPlot::PlotBars("Volume", m_dates.data(), m_volumes.data(), m_volumes.size(), x_axis_interval * 0.5);

                m_volume_chart_limits = ImPlot::GetPlotLimits();

                // sharing plot limits so they are all in sync
                if (ImPlot::IsPlotHovered())
                {
                    m_shared_limits = ImPlot::GetPlotLimits();
                    m_is_volume_chart_hovered = true;
                }
                else
                {
                    m_is_volume_chart_hovered = false;
                }

                ImPlot::EndPlot();
            }
            if (largest_label_width > volume_label_width)
            {
                ImPlot::PopStyleVar();
            }

            size_t applied_seperate_charts_index = 0;
            for (auto& pair : m_applied_indicators_data)
            {
                if (pair.second.m_show && !TradinatorAppSpace::Utils::IsIndicatorOverlayable(pair.first->IndicatorType()))
                {
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
                    if (largest_label_width > (pair.second.m_label_width + 20.0f))
                    {
                        ImVec2 plot_padding(largest_label_width - pair.second.m_label_width + 20.0f, 0); // x = left, y = top
                        ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, plot_padding);
                    }
                    if (ImPlot::BeginPlot(std::format("{}##{}_{}", pair.first->GetName(), m_counter->ISIN_Number(), pair.second.m_id).c_str(), ImVec2(-1, chart_height), ImPlotFlags_NoLegend | ImPlotFlags_CanvasOnly))
                    {
                        ImPlot::SetupAxes(nullptr, nullptr, x_axis_flags, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);

                        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
                        if (pair.second.m_is_hovered)
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

                        ImPlot::SetNextLineStyle(pair.second.m_color, pair.second.m_color.w);
                        ImPlot::PlotLineG(std::format("{}##Chart{}_{}", pair.first->GetName(), m_counter->ISIN_Number(), pair.second.m_id).c_str()
                            , indicator_plot_point_getter
                            , (void*)&pair.second.m_points
                            , pair.second.m_points.size());
                        
                        pair.second.m_chart_limits = ImPlot::GetPlotLimits();

                        if (ImPlot::IsPlotHovered())
                        {
                            m_shared_limits = ImPlot::GetPlotLimits();
                            pair.second.m_is_hovered = true;
                        }
                        else
                        {
                            pair.second.m_is_hovered = false;
                        }
                        ImPlot::EndPlot();
                    }
                    if (largest_label_width > (pair.second.m_label_width + 5.0))
                    {
                        ImPlot::PopStyleVar();
                    }
                    ++applied_seperate_charts_index;
                }
            }
        }
    }
    ImGui::End();
}

void CounterWindow::ShowIndicatorsList()
{
    float indicator_height = 55.0f;
    float indicator_width = 600.0f;
    
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 item_spacing = style.ItemSpacing;

    float window_width = ImGui::GetWindowWidth();
    float table_width = (window_width - item_spacing.x * 3.0f) / 2.0f;
    int  column_count = std::max((int)(table_width / indicator_width), 1);

    float max_height = 300.0f;

    int total_applied_indicators = m_applied_indicators_data.size();
    int total_available_indicators = m_available_indicators.size();
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
            int i = 0;
            int row = -1;
            for (auto& pair : m_applied_indicators_data)
            {
                int tmp_row = i / column_count;
                if (tmp_row != row)
                {
                    row = tmp_row;
                    ImGui::TableNextRow(0, 0);
                }

                ImGui::TableSetColumnIndex(i % column_count);

                ShowAppliedIndicator(pair.first);

                ++i;
            }

            /// @separator


            /// @separator
            
            ImGui::EndTable();
        }
        /// @end Table

        /// @separator
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

                ShowAvailableIndicator(m_available_indicators[i]);
            }
            /// @separator


            /// @separator
            ImGui::EndTable();
        }
        /// @end Table

        /// @separator
        ImGui::EndChild();
    }
    /// @end Child
}

void CounterWindow::AddIndicator(std::shared_ptr<Indicator> indicator)
{
    AddIndicator(indicator, TradinatorAppSpace::Utils::GetIndicatorColor(indicator->IndicatorType()));
}

void CounterWindow::AddIndicator(std::shared_ptr<Indicator> indicator, ImVec4 color)
{
    IndicatorData indicator_data;
    indicator_data.m_color = color;
    if (TradinatorAppSpace::Utils::IsIndicatorEnvelopeType(indicator->IndicatorType()))
    {
        BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(indicator.get());

        std::vector<std::vector<IndicatorPoint>> envelope_points = std::move(bollinger_band->CalculateEnvelope());
        indicator_data.m_top_points = std::move(envelope_points[0]);
        indicator_data.m_points = std::move(envelope_points[1]);
        indicator_data.m_bottom_points = std::move(envelope_points[2]);
    }
    else
    {
        indicator_data.m_points = std::move(indicator->Calculate());
    }
    
    indicator_data.m_show = true;
    indicator_data.m_id = _INCREMENTAL_ID_;

    m_applied_indicators_data[indicator] = indicator_data;

    _INCREMENTAL_ID_++;
}

void CounterWindow::ShowAvailableIndicator(const std::unique_ptr<Indicator>& indicator)
{
    /// @begin Text
    ImGui::SetNextItemWidth(350);
    bool is_selected = false;
    
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.CellPadding.y * 2)); // Fix
    //ImGui::Selectable(std::format("{}##{}", indicator->GetName(), indicator->GetName()).c_str(), &is_selected); ImGui::SameLine();
    //ImGui::PopStyleVar();
    ImGui::TextUnformatted(indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text

    /// @begin Input
    ImGui::SetNextItemWidth(70);
    std::string indicator_length = std::format("{}", indicator->GetLength());
    char length_str[4] = "";
    std::copy(indicator_length.begin(), indicator_length.end(), length_str);
    
    if (ImGui::InputText(std::format("##length{}", indicator->GetName()).c_str(), length_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int length = std::atoi(length_str);
        indicator->SetLength(length);
    }
    ImGui::SameLine();
    
    if (indicator->IndicatorType() == EIndicatorType::E_BOLLINGER_BAND)
    {
        BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(indicator.get());

        ImGui::SetNextItemWidth(70);
        std::string indicator_length = std::format("{:.1f}", bollinger_band->GetStandardDeviationMultiplier());
        char multiplier_str[5] = "";
        std::copy(indicator_length.begin(), indicator_length.end(), multiplier_str);

        if (ImGui::InputText(std::format("##standard deviation multiplier{}", indicator->GetName()).c_str(), multiplier_str, 5, ImGuiInputTextFlags_CharsDecimal))
        {
            double multiplier = std::atof(multiplier_str);
            bollinger_band->SetStandardDeviationMultiplier(multiplier);
        }
        ImGui::SameLine();
    }

    /// @end Input

    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" + ##{}", indicator->GetName()).c_str(), { 0, 0 }))
    {
        std::shared_ptr<Indicator> new_indicator = indicator->Clone();
        new_indicator->SetCounter(m_counter);

        AddIndicator(new_indicator);
        
    }
}

void CounterWindow::ShowAppliedIndicator(const std::shared_ptr<Indicator>& indicator) 
{
    IndicatorData& indicator_data = m_applied_indicators_data[indicator];
    /// @begin Text
    ImGui::SetNextItemWidth(250);
    bool is_selected = false;

    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.CellPadding.y * 2)); // Fix
    //ImGui::Selectable(std::format("{}##{}", indicator->GetName(), indicator->GetName()).c_str(), &is_selected); ImGui::SameLine();
    //ImGui::PopStyleVar();

    ImGui::Checkbox(std::format("##show/hide{}", indicator_data.m_id).c_str(), &indicator_data.m_show); ImGui::SameLine();
    ImGui::TextUnformatted(indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text

    /// @begin Input
    ImGui::SetNextItemWidth(70);
    std::string indicator_length = std::format("{}", indicator->GetLength());
    char length_str[4] = "";
    std::copy(indicator_length.begin(), indicator_length.end(), length_str);

    if (ImGui::InputText(std::format("##indicator length{}", indicator_data.m_id).c_str(), length_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int length = std::atoi(length_str);
        indicator->SetLength(length);

        if (TradinatorAppSpace::Utils::IsIndicatorEnvelopeType(indicator->IndicatorType()))
        {
            BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(indicator.get());

            std::vector<std::vector<IndicatorPoint>> envelope_points = std::move(bollinger_band->CalculateEnvelope());
            indicator_data.m_top_points = std::move(envelope_points[0]);
            indicator_data.m_points = std::move(envelope_points[1]);
            indicator_data.m_bottom_points = std::move(envelope_points[2]);
        }
        else
        {
            indicator_data.m_points = std::move(indicator->Calculate());
        }
    }
    ImGui::SameLine();

    if (indicator->IndicatorType() == EIndicatorType::E_BOLLINGER_BAND)
    {
        BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(indicator.get());

        ImGui::SetNextItemWidth(70);
        std::string indicator_length = std::format("{:.1f}", bollinger_band->GetStandardDeviationMultiplier());
        char multiplier_str[5] = "";
        std::copy(indicator_length.begin(), indicator_length.end(), multiplier_str);

        if (ImGui::InputText(std::format("##standard deviation multiplier{}", indicator_data.m_id).c_str(), multiplier_str, 5, ImGuiInputTextFlags_CharsDecimal))
        {
            double multiplier = std::atof(multiplier_str);
            bollinger_band->SetStandardDeviationMultiplier(multiplier);

            std::vector<std::vector<IndicatorPoint>> envelope_points = std::move(bollinger_band->CalculateEnvelope());
            indicator_data.m_top_points = std::move(envelope_points[0]);
            indicator_data.m_points = std::move(envelope_points[1]);
            indicator_data.m_bottom_points = std::move(envelope_points[2]);
        }
        ImGui::SameLine();
    }

    /// @end Input
    ImGui::ColorEdit4(std::format("##plot color{}", indicator_data.m_id).c_str(), &indicator_data.m_color.x, ImGuiColorEditFlags_NoInputs);

    ::ImGui::SameLine();
    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" x ##remove indicator{}", indicator_data.m_id).c_str(), { 0, 0 }))
    {
        m_remove_applied_indicators.push_back(indicator);
    }
}

void CounterWindow::ShowTitle()
{
    ImVec2 title_size = ImGui::CalcTextSize(m_cached_label_id.c_str());
    ImVec2 close_text_size = ImGui::CalcTextSize(" X ");

    ImGuiID id = ImGui::GetID(m_cached_label_id.c_str());
    float top = ImGui::GetCursorPosY();
    ImGui::Text(m_counter->Name().c_str());



    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 window_pos = ImGui::GetWindowPos();

    ImVec2 button_size = ImVec2(70, 57);

    ImVec2 available_space = ImGui::GetContentRegionAvail();
    //ImGui::SetCursorPosX(available_space.x - button_size.x);
    ImVec2 prev_cursor_pos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(window_size.x - button_size.x, top - 16));

    // remove padding after x button so that there is no scrolling
    // but this isn't working. 
    // TODO: Figure out how to remove padding after x button so that page won't scroll horizontally
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); 
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

void CounterWindow::RebuildCachedPlotPoints()
{
    const std::shared_ptr<const AsyncData<AsyncCandleData>>& candle_data = m_counter->GetCandleData();
    size_t count = candle_data->GetData().size();

    // don't care about previous data stored in cache
    m_dates.reserve(count);
    m_opens.reserve(count);
    m_highes.reserve(count);
    m_lows.reserve(count);
    m_closes.reserve(count);
    m_volumes.reserve(count);
    m_open_interests.reserve(count);

    date_axis_min = SIZE_MAX;
    date_axis_max = 0;
    price_axis_min = DBL_MAX;
    price_axis_max = -DBL_MAX;
    volume_axis_min = SIZE_MAX;
    volume_axis_max = 0;
    const auto& candle_data_map = candle_data->GetData();

    // processing in reverse direction because plot needs data to be in ascending order of dates
    for (auto iter = candle_data_map.rbegin(); iter != candle_data_map.rend(); ++iter) {

        std::chrono::system_clock::rep date = std::chrono::duration_cast<std::chrono::seconds>(iter->second.m_date.time_since_epoch()).count();

        m_dates.push_back(date);
        m_opens.push_back(iter->second.m_open);
        m_highes.push_back(iter->second.m_high);
        m_lows.push_back(iter->second.m_low);
        m_closes.push_back(iter->second.m_close);
        m_volumes.push_back(iter->second.m_volume);
        m_open_interests.push_back(iter->second.m_open_interest);

        if (date_axis_min > date)
            date_axis_min = date;
        if (date_axis_max < date)
            date_axis_max = date;

        double max = std::max({ iter->second.m_open, iter->second.m_high, iter->second.m_close, iter->second.m_low });
        double min = std::max({ iter->second.m_open, iter->second.m_high, iter->second.m_close, iter->second.m_low });

        if (price_axis_min > min)
            price_axis_min = min;
        if (price_axis_max < max)
            price_axis_max = max;

        if (volume_axis_min > iter->second.m_volume)
            volume_axis_min = iter->second.m_volume;
        if (volume_axis_max < iter->second.m_volume)
            volume_axis_max = iter->second.m_volume;

        count--;
        //if(count <=0 )
            //break;
    }

    // Adding one day padding at start and end of graph
    date_axis_min -= 60 * 60 * 24;
    date_axis_max += 60 * 60 * 24;

    m_shared_limits.X.Min = date_axis_min;
    m_shared_limits.X.Max = date_axis_max;
}

void CounterWindow::PlotCandlestick(const char* label_id, const size_t* xs, const double* opens, const double* closes, const double* lows, const double* highs, int count, bool tooltip, float width_percent, ImVec4 bullCol, ImVec4 bearCol) {

    // get ImGui window DrawList
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    // calc real value width
    double x_axis_interval = 60 * 60 * 24; // one day in sec
    double half_width = count > 1 ? x_axis_interval * width_percent : width_percent;

    // custom tool
    if (ImPlot::IsPlotHovered() && tooltip) 
    {
        ImPlotPoint mouse = ImPlot::GetPlotMousePos();
        mouse.x = ImPlot::RoundTime(ImPlotTime::FromDouble(mouse.x), ImPlotTimeUnit_Day).ToDouble();
        float  tool_l = ImPlot::PlotToPixels(mouse.x - half_width * 1.5, mouse.y).x;
        float  tool_r = ImPlot::PlotToPixels(mouse.x + half_width * 1.5, mouse.y).x;
        float  tool_t = ImPlot::GetPlotPos().y;
        float  tool_b = tool_t + ImPlot::GetPlotSize().y;
        ImPlot::PushPlotClipRect();
        draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
        ImPlot::PopPlotClipRect();
        
        // find mouse location index
        int idx = BinarySearch(xs, 0, count - 1, mouse.x);
        // render tool tip (won't be affected by plot clip rect)
        if (idx != -1) {
            ImGui::BeginTooltip();
            char buff[32];
            ImPlot::FormatDate(ImPlotTime::FromDouble(xs[idx]), buff, 32, ImPlotDateFmt_DayMoYr, ImPlot::GetStyle().UseISO8601);

            std::string text = std::format(
                "Day:    {}\n\n"
                "Open:   ${}\n"
                "Close:  ${}\n"
                "Low:    ${}\n"
                "High:   ${}\n\n"
                "Volume: {}", buff, opens[idx], closes[idx], lows[idx], highs[idx], m_volumes[idx]);

            /*ImGui::Text("Day:    %s", buff);
            ImGui::Text("Open:   $%.2f", opens[idx]);
            ImGui::Text("Close:  $%.2f", closes[idx]);
            ImGui::Text("Low:    $%.2f", lows[idx]);
            ImGui::Text("High:   $%.2f", highs[idx]);
            ImGui::Text("\nVolume: %.0f", m_volumes[idx]);*/
            ImGui::Text(text.c_str());

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

        // Draw filling behind the candles otherwise it will cover up the candles
        for (auto& pair : m_applied_indicators_data)
        {
            if (pair.second.m_show &&
                TradinatorAppSpace::Utils::IsIndicatorOverlayable(pair.first->IndicatorType()))
            {
                if (TradinatorAppSpace::Utils::IsIndicatorEnvelopeType(pair.first->IndicatorType()))
                {
                    PlotEnvelopeIndicatorFill(pair.first);
                }
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
            draw_list->AddRectFilled(open_pos, close_pos, color);
        }

        for (auto& pair : m_applied_indicators_data)
        {
            if (pair.second.m_show && 
                TradinatorAppSpace::Utils::IsIndicatorOverlayable(pair.first->IndicatorType()))
            {
                if (TradinatorAppSpace::Utils::IsIndicatorEnvelopeType(pair.first->IndicatorType()))
                {
                    PlotEnvelopeIndicatorLines(pair.first);
                }
                else
                {
                    PlotIndicator(pair.first);
                }
            }
        }

        // end plot item
        ImPlot::EndItem();
    }
}

void CounterWindow::PlotIndicator(const std::shared_ptr<Indicator>& indicator)
{
    const CounterWindow::IndicatorData& indicator_data = m_applied_indicators_data[indicator];
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    size_t count = indicator_data.m_points.size();
    
    if (count != 0)
    {
        ImPlot::SetNextLineStyle(indicator_data.m_color, indicator_data.m_color.w);
        ImPlot::PlotLineG("Indicators", indicator_plot_point_getter, (void*)&indicator_data.m_points, count);
    }
}

void CounterWindow::PlotEnvelopeIndicatorFill(const std::shared_ptr<Indicator>& indicator)
{
    const CounterWindow::IndicatorData& indicator_data = m_applied_indicators_data[indicator];
    size_t count = indicator_data.m_points.size();

    if (count != 0)
    {
        // 10% of alpha of original color for filling
        ImPlot::SetNextFillStyle(indicator_data.m_color, indicator_data.m_color.w * 0.1);

        ImPlot::PlotShadedG("BollingerBand", indicator_plot_point_getter, (void*)& indicator_data.m_top_points,
            indicator_plot_point_getter, (void*)&indicator_data.m_bottom_points, indicator_data.m_bottom_points.size());
    }
}

void CounterWindow::PlotEnvelopeIndicatorLines(const std::shared_ptr<Indicator>& indicator)
{
    const CounterWindow::IndicatorData& indicator_data = m_applied_indicators_data[indicator];
    size_t count = indicator_data.m_points.size();

    if (count != 0)
    {
        ImPlot::SetNextLineStyle(indicator_data.m_color, indicator_data.m_color.w);
        ImPlot::PlotLineG("Top", indicator_plot_point_getter, (void*)&indicator_data.m_top_points, indicator_data.m_top_points.size());

        ImPlot::SetNextLineStyle(indicator_data.m_color, indicator_data.m_color.w);
        ImPlot::PlotLineG("Top", indicator_plot_point_getter, (void*)&indicator_data.m_points, indicator_data.m_points.size());

        ImPlot::SetNextLineStyle(indicator_data.m_color, indicator_data.m_color.w);
        ImPlot::PlotLineG("Bottom", indicator_plot_point_getter, (void*)&indicator_data.m_bottom_points, indicator_data.m_bottom_points.size());
    }
}

//template <typename T>
int CounterWindow::BinarySearch(const size_t* arr, int l, int r, double x) {
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



Json::Value CounterWindow::GetCounterStatus()
{
    Json::Value result;

    result["Symbol"] = m_counter->Symbol();
    result["ISIN"] = m_counter->ISIN_Number();

    Json::Value applied_indicators(Json::arrayValue);
    for (auto& indicator : m_applied_indicators_data)
    {
        Json::Value json_indicator;
        json_indicator["Name"] = TradinatorAppSpace::Utils::GetIndicatorTypeStr(indicator.first->IndicatorType());
        json_indicator["Length"] = indicator.first->GetLength();

        if (indicator.first->IndicatorType() == EIndicatorType::E_BOLLINGER_BAND)
        {
            BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(indicator.first.get());
            json_indicator["Multiplier"] = bollinger_band->GetStandardDeviationMultiplier();
        }

        Json::Value color;

        color["R"] = indicator.second.m_color.x;
        color["G"] = indicator.second.m_color.y;
        color["B"] = indicator.second.m_color.z;
        color["A"] = indicator.second.m_color.w;

        json_indicator["Color"] = color;

        applied_indicators.append(json_indicator);
    }

    result["Applied_Indicators"] = applied_indicators;

    return result;
}

void CounterWindow::SetCounterStatus(Json::Value status)
{
    Json::Value applied_indicators = status["Applied_Indicators"];

    Json::Value::ArrayIndex count = applied_indicators.size();

    for (Json::Value::ArrayIndex i = 0; i < count; ++i)
    {
        Json::Value json_indicator = applied_indicators[i];
        EIndicatorType type = TradinatorAppSpace::Utils::GetIndicatorType(json_indicator["Name"].asString());
        std::shared_ptr<Indicator> indicator = TradinatorAppSpace::Utils::GetIndicator(type);
        if (indicator)
        {
            indicator->SetCounter(m_counter);
            indicator->SetLength(json_indicator["Length"].asUInt64());

            if (indicator->IndicatorType() == EIndicatorType::E_BOLLINGER_BAND)
            {
                BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(indicator.get());
                bollinger_band->SetStandardDeviationMultiplier(json_indicator["Multiplier"].asDouble());
            }

            ImVec4 color;
            color.x = json_indicator["Color"]["R"].asFloat();
            color.y = json_indicator["Color"]["G"].asFloat();
            color.z = json_indicator["Color"]["B"].asFloat();
            color.w = json_indicator["Color"]["A"].asFloat();

            AddIndicator(indicator, color);
        }
    }
}