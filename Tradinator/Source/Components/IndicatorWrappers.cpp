#include "Components/IndicatorWrappers.h"

#include "implot.h"
#include "implot_internal.h"

#include "Data/Counter.h"
#include "Indicators/BollingerBand.h"
#include "Indicators/MACD.h"
#include "Indicators/TrendAnalysisDebug.h"

#include "Utils.h"

size_t IIndicatorWrapper::_INCREMENTAL_WRAPPER_ID_ = 0;

// Getter for IndicatorPoint to draw the plot
ImPlotPoint indicator_plot_point_getter(int idx, void* data) {
    std::vector<IndicatorPoint>* point_data = (std::vector<IndicatorPoint>*)(data);
    const IndicatorPoint& point = (*point_data)[idx];

    return ImPlotPoint(std::chrono::duration_cast<std::chrono::seconds>(point.date.time_since_epoch()).count(), point.value);
}

IIndicatorWrapper::IIndicatorWrapper()
{
    m_id = _INCREMENTAL_WRAPPER_ID_++;
}

IIndicatorWrapper::IIndicatorWrapper(std::unique_ptr<Indicator> indicator)
    : m_indicator(std::move(indicator)), m_counter(nullptr)
{
    m_id = _INCREMENTAL_WRAPPER_ID_++;

    m_colors_list.push_back(TradinatorAppSpace::Utils::GetIndicatorColor(m_indicator->IndicatorType()));
}

IIndicatorWrapper::IIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Counter> counter)
    : m_indicator(std::move(indicator)), m_counter(counter)
{
    m_id = _INCREMENTAL_WRAPPER_ID_++;

    m_colors_list.push_back(TradinatorAppSpace::Utils::GetIndicatorColor(m_indicator->IndicatorType()));
}

IIndicatorWrapper::IIndicatorWrapper(const IIndicatorWrapper& other)
    : m_indicator(std::move(other.m_indicator->Clone()))
    , m_counter(other.m_counter)
    , m_points_list(other.m_points_list)
    , m_colors_list(other.m_colors_list)
    , m_show(other.m_show)
    , m_is_hovered(other.m_is_hovered)
{
    m_id = _INCREMENTAL_WRAPPER_ID_++;
}

IIndicatorWrapper& IIndicatorWrapper::operator=(const IIndicatorWrapper& other)
{
    m_indicator = std::move(other.m_indicator->Clone());
    m_counter = other.m_counter;

    m_points_list = other.m_points_list;
    m_colors_list = other.m_colors_list;

    m_show = other.m_show;
    m_is_hovered = m_is_hovered;
    m_id = _INCREMENTAL_WRAPPER_ID_++;

    return *this;
}

void IIndicatorWrapper::SetIndicator(std::unique_ptr<Indicator> indicator)
{
    m_indicator = std::move(indicator);

    m_colors_list.clear();
    m_colors_list.push_back(TradinatorAppSpace::Utils::GetIndicatorColor(m_indicator->IndicatorType()));
}

void IIndicatorWrapper::SetCounter(std::shared_ptr<Counter> counter)
{
    m_counter = counter;
    if (m_indicator)
    {
        m_indicator->SetCounter(counter);
    }
}


/*********************************************************************************
*                                Generic Wrapper
**********************************************************************************/

bool GenericIndicatorWrapper::DrawAsAvailableIndicator()
{
    bool is_pressed = false;

    /// @begin Text
    ImGui::SetNextItemWidth(350);
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text

    /// @begin Input
    ImGui::SetNextItemWidth(70);
    std::string indicator_length = std::format("{}", m_indicator->GetLength());
    char length_str[4] = "";
    std::copy(indicator_length.begin(), indicator_length.end(), length_str);

    if (ImGui::InputText(std::format("##length{}", m_id).c_str(), length_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int length = std::atoi(length_str);
        m_indicator->SetLength(length);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Length");
    }
    ImGui::SameLine();
    /// @end Input

    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" + ##{}", m_id).c_str(), { 0, 0 }))
    {
        std::shared_ptr<Indicator> new_indicator = m_indicator->Clone();
        new_indicator->SetCounter(m_counter);

        is_pressed = true;
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Apply {}", m_indicator->GetName()).c_str());
    }

    return is_pressed;
}

bool GenericIndicatorWrapper::DrawAsAppliedIndicator()
{
    bool is_pressed = false;

    ImGui::Checkbox(std::format("##show/hide{}", m_id).c_str(), &m_show); ImGui::SameLine();
    /// @begin Text
    ImGui::SetNextItemWidth(250);
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text

    /// @begin Input
    ImGui::SetNextItemWidth(70);
    std::string indicator_length = std::format("{}", m_indicator->GetLength());
    char length_str[4] = "";
    std::copy(indicator_length.begin(), indicator_length.end(), length_str);

    if (ImGui::InputText(std::format("##indicator length{}", m_id).c_str(), length_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int length = std::atoi(length_str);
        if (m_indicator->GetLength() != length)
        {
            m_indicator->SetLength(length);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Length");
    }
    ImGui::SameLine();
    /// @end Input



    ImGui::ColorEdit4(std::format("##plot color{}", m_id).c_str(), &m_colors_list[0].x, ImGuiColorEditFlags_NoInputs);

    ::ImGui::SameLine();
    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" x ##remove indicator{}", m_id).c_str(), { 0, 0 }))
    {
        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Remove {}", m_indicator->GetName()).c_str());
    }

    return is_pressed;
}

void GenericIndicatorWrapper::Calculate()
{
    assert(m_counter);

    m_points_list.clear();
    m_points_list = std::move(m_indicator->Calculate());
}

void GenericIndicatorWrapper::PlotPreCandle(ImVec4 bull_color, ImVec4 bear_color)
{

}

void GenericIndicatorWrapper::PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color)
{
    if (m_points_list.size() == 0) return;

    size_t count = m_points_list[0].size();
    if (count == 0) return;

    ImPlot::SetNextLineStyle(m_colors_list[0], m_colors_list[0].w);
    ImPlot::PlotLineG(std::format("{}", m_indicator->GetName()).c_str(), indicator_plot_point_getter, (void*)&m_points_list[0], count);
}

std::string GenericIndicatorWrapper::GetHumanReadableValueAt(size_t index) const
{
    if (m_points_list.size() >= 1)
    {
        return std::format("{}({}) :   {}",
            TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType()), index,
            m_points_list[0][index].value);
    }

    return std::format("{}({}) :   Invalid Input",
        TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType()), index);
}

void GenericIndicatorWrapper::FromJson(Json::Value value)
{
    EIndicatorType type = TradinatorAppSpace::Utils::GetIndicatorType(value["Name"].asString());
    if (!m_indicator)
    {
        m_indicator = std::move(TradinatorAppSpace::Utils::GetIndicator(type));
    }
    m_indicator->SetLength(value["Length"].asUInt64());
    m_show = value["Show"].asBool();

    ImVec4 color;
    color.x = value["Color"]["R"].asFloat();
    color.y = value["Color"]["G"].asFloat();
    color.z = value["Color"]["B"].asFloat();
    color.w = value["Color"]["A"].asFloat();

    m_colors_list.clear();
    m_colors_list.push_back(color);
}

Json::Value GenericIndicatorWrapper::ToJson() const
{
    Json::Value json_indicator;
    json_indicator["Name"] = TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType());
    json_indicator["Length"] = m_indicator->GetLength();
    json_indicator["Show"] = m_show;

    Json::Value json_color;
    json_color["R"] = m_colors_list[0].x;
    json_color["G"] = m_colors_list[0].y;
    json_color["B"] = m_colors_list[0].z;
    json_color["A"] = m_colors_list[0].w;

    json_indicator["Color"] = json_color;

    return json_indicator;
}


/*********************************************************************************
*                                Generic Chart Wrapper
**********************************************************************************/

void GenericChartIndicatorWrapper::PlotPreCandle(ImVec4 bull_color, ImVec4 bear_color)
{
    assert("Chart indicator will draw its own chart. This shouldn't be called. Call DrawCustomChart to draw the chart.");
}

void GenericChartIndicatorWrapper::PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color)
{
    assert("Chart indicator will draw its own chart. This shouldn't be called. Call DrawCustomChart to draw the chart.");
}

void GenericChartIndicatorWrapper::Calculate()
{
    assert(m_counter);

    GenericIndicatorWrapper::Calculate();

    for (const std::vector<IndicatorPoint>& points : m_points_list)
    {
        if (points.size() > 0)
        {
            std::chrono::system_clock::rep latest_date = std::chrono::duration_cast<std::chrono::seconds>(points[0].date.time_since_epoch()).count();
            std::chrono::system_clock::rep oldest_date = std::chrono::duration_cast<std::chrono::seconds>(points[points.size() - 1].date.time_since_epoch()).count();

            if (m_x_axis_min > oldest_date)
                m_x_axis_min = oldest_date;
            if (m_x_axis_max < latest_date)
                m_x_axis_max = latest_date;
        }
    }
}

void GenericChartIndicatorWrapper::CalculateLabelWidth()
{
    float max_range_width = ImGui::CalcTextSize(std::format("${:.0f}", m_chart_limits.Y.Max).c_str()).x;
    float min_range_width = ImGui::CalcTextSize(std::format("${:.0f}", m_chart_limits.Y.Min).c_str()).x;
    m_label_width = std::max(max_range_width, min_range_width);
}

void GenericChartIndicatorWrapper::DrawCustomChart(double chart_height, ImPlotAxisFlags x_axis_flags, ImPlotAxisFlags y_axis_flags, ImPlotRect& shared_limits, bool& is_any_plot_hovered, bool show_highlight, ImPlotPoint& hovered_mouse_point, float hover_highlight_l, float hover_highlight_r, ImVec4 bull_color, ImVec4 bear_color)
{
    assert(m_counter);
    assert(!IsIndicatorOverlayable());
    
    

    if (ImPlot::BeginPlot(std::format("{}##{}_{}", m_indicator->GetName(), m_counter->ISIN_Number(), m_id).c_str(), ImVec2(-1, chart_height), ImPlotFlags_NoTitle))
    {
        ImPlot::SetupAxes(nullptr, nullptr, x_axis_flags | ImPlotAxisFlags_NoGridLines, y_axis_flags);

        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
        if (m_is_hovered)
        {
            ImPlot::SetupAxisLimits(ImAxis_X1, m_x_axis_min, m_x_axis_max);
        }
        else
        {
            ImPlot::SetupAxisLimits(ImAxis_X1, shared_limits.X.Min, shared_limits.X.Max, ImGuiCond_Always);
        }
        //ImPlot::SetupAxisLimits(ImAxis_Y1, y_max - padding, y_max + padding, ImGuiCond_Always);

        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, m_x_axis_min, m_x_axis_max);
        ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 60 * 60 * 24 * 14, m_x_axis_max - m_x_axis_min); // 14 days at min and full chat at max

        m_chart_limits = ImPlot::GetPlotLimits();

        is_any_plot_hovered |= ImPlot::IsPlotHovered();
        if (ImPlot::IsPlotHovered())
        {
            shared_limits = ImPlot::GetPlotLimits();
            m_is_hovered = true;

            hovered_mouse_point = ImPlot::GetPlotMousePos();
            hovered_mouse_point.x = ImPlot::RoundTime(ImPlotTime::FromDouble(hovered_mouse_point.x), ImPlotTimeUnit_Day).ToDouble();
        }
        else
        {
            m_is_hovered = false;
        }

        if (show_highlight)
        {
            ImDrawList* draw_list = ImPlot::GetPlotDrawList();

            float highlight_l = ImPlot::PlotToPixels(hover_highlight_l, hovered_mouse_point.y).x;
            float highlight_r = ImPlot::PlotToPixels(hover_highlight_r, hovered_mouse_point.y).x;
            float  highlight_t = ImPlot::GetPlotPos().y;
            float  highlight_b = highlight_t + ImPlot::GetPlotSize().y;
            ImPlot::PushPlotClipRect();
            draw_list->AddRectFilled(ImVec2(highlight_l, highlight_t), ImVec2(highlight_r, highlight_b), IM_COL32(128, 128, 128, 64));
            ImPlot::PopPlotClipRect();
        }

        PlotItems(bull_color, bear_color);

        ImPlot::EndPlot();
    }
}

void GenericChartIndicatorWrapper::PlotItems(ImVec4 bull_color, ImVec4 bear_color)
{
    for (int i = 0; i < m_points_list.size(); ++i)
    {
        ImVec4 color = m_colors_list[0];
        if (i < m_colors_list.size())
        {
            color = m_colors_list[i];
        }

        ImPlot::SetNextLineStyle(color, color.w);
        ImPlot::PlotLineG(std::format("{}##Chart{}_{}{}", m_indicator->GetName(), m_counter->ISIN_Number(), m_id, i).c_str()
            , indicator_plot_point_getter
            , (void*)&m_points_list[i]
            , m_points_list[i].size());
    }
}

/*********************************************************************************
*                         Bollinger Band
**********************************************************************************/

bool BollingerBandIndicatorWrapper::DrawAsAvailableIndicator()
{
    bool is_pressed = false;

    BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(m_indicator.get());
    assert(bollinger_band);

    /// @begin Text
    ImGui::SetNextItemWidth(350);
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text

    /// @begin Input
    ImGui::SetNextItemWidth(70);
    std::string indicator_length = std::format("{}", m_indicator->GetLength());
    char length_str[4] = "";
    std::copy(indicator_length.begin(), indicator_length.end(), length_str);

    if (ImGui::InputText(std::format("##length{}", m_id).c_str(), length_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int length = std::atoi(length_str);
        m_indicator->SetLength(length);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Length");
    }
    ImGui::SameLine();

    

    ImGui::SetNextItemWidth(70);
    std::string indicator_sd_multiplier = std::format("{:.1f}", bollinger_band->GetStandardDeviationMultiplier());
    char multiplier_str[5] = "";
    std::copy(indicator_sd_multiplier.begin(), indicator_sd_multiplier.end(), multiplier_str);

    if (ImGui::InputText(std::format("##standard deviation multiplier{}", m_id).c_str(), multiplier_str, 5, ImGuiInputTextFlags_CharsDecimal))
    {
        double multiplier = std::atof(multiplier_str);
        bollinger_band->SetStandardDeviationMultiplier(multiplier);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Standard Deviation Multiplier");
    }
    ImGui::SameLine();
    /// @end Input




    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" + ##{}", m_id).c_str(), { 0, 0 }))
    {
        std::shared_ptr<Indicator> new_indicator = m_indicator->Clone();
        new_indicator->SetCounter(m_counter);

        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Apply {}", m_indicator->GetName()).c_str());
    }

    return is_pressed;
}

bool BollingerBandIndicatorWrapper::DrawAsAppliedIndicator()
{
    BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(m_indicator.get());
    assert(bollinger_band);

    bool is_pressed = false;

    ImGui::Checkbox(std::format("##show/hide{}", m_id).c_str(), &m_show); ImGui::SameLine();
    
    /// @begin Text
    ImGui::SetNextItemWidth(250);
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text


    /// @begin Input
    ImGui::SetNextItemWidth(70);
    std::string indicator_length = std::format("{}", m_indicator->GetLength());
    char length_str[4] = "";
    std::copy(indicator_length.begin(), indicator_length.end(), length_str);

    if (ImGui::InputText(std::format("##indicator length{}", m_id).c_str(), length_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int length = std::atoi(length_str);
        if (m_indicator->GetLength() != length)
        {
            m_indicator->SetLength(length);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Length");
    }
    ImGui::SameLine();


    ImGui::SetNextItemWidth(70);
    std::string indicator_sd_multiplier = std::format("{:.1f}", bollinger_band->GetStandardDeviationMultiplier());
    char multiplier_str[5] = "";
    std::copy(indicator_sd_multiplier.begin(), indicator_sd_multiplier.end(), multiplier_str);

    if (ImGui::InputText(std::format("##standard deviation multiplier{}", m_id).c_str(), multiplier_str, 5, ImGuiInputTextFlags_CharsDecimal))
    {
        double multiplier = std::atof(multiplier_str);
        if (fabs(bollinger_band->GetStandardDeviationMultiplier() - multiplier) > LDBL_EPSILON)
        {
            bollinger_band->SetStandardDeviationMultiplier(multiplier);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Standard Deviation Multiplier");
    }
    ImGui::SameLine();
    /// @end Input


    ImGui::ColorEdit4(std::format("##plot color{}", m_id).c_str(), &m_colors_list[0].x, ImGuiColorEditFlags_NoInputs);

    
    /// @begin Button
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" x ##remove indicator{}", m_id).c_str(), { 0, 0 }))
    {
        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Remove {}", m_indicator->GetName()).c_str());
    }
    /// @end Button

    return is_pressed;
}

void BollingerBandIndicatorWrapper::Calculate()
{
    assert(m_counter);

    BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(m_indicator.get());
    assert(bollinger_band);

    m_points_list.clear();
    m_points_list = std::move(bollinger_band->Calculate());
}

void BollingerBandIndicatorWrapper::PlotPreCandle(ImVec4 bull_color, ImVec4 bear_color)
{
    size_t count = m_points_list[0].size();

    if (count != 0)
    {
        // 10% of alpha of original color for filling
        ImPlot::SetNextFillStyle(m_colors_list[0], m_colors_list[0].w * 0.1f);

        ImPlot::PlotShadedG("BollingerBand", indicator_plot_point_getter, (void*)&m_points_list[0],
            indicator_plot_point_getter, (void*)&m_points_list[2], count);
    }
}

void BollingerBandIndicatorWrapper::PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color)
{
    size_t count = m_points_list[0].size();

    if (count != 0)
    {
        ImPlot::SetNextLineStyle(m_colors_list[0], m_colors_list[0].w);
        ImPlot::PlotLineG("Top", indicator_plot_point_getter, (void*)&m_points_list[0], m_points_list[0].size());

        ImPlot::SetNextLineStyle(m_colors_list[0], m_colors_list[0].w);
        ImPlot::PlotLineG("SMA", indicator_plot_point_getter, (void*)&m_points_list[1], m_points_list[1].size());

        ImPlot::SetNextLineStyle(m_colors_list[0], m_colors_list[0].w);
        ImPlot::PlotLineG("Bottom", indicator_plot_point_getter, (void*)&m_points_list[2], m_points_list[2].size());
    }
}

std::string BollingerBandIndicatorWrapper::GetHumanReadableValueAt(size_t index) const
{
    if (m_points_list.size() >= 3)
    {
        return std::format("{}({}) :   {}, {}, {}",
            TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType()), index,
            m_points_list[0][index].value,
            m_points_list[1][index].value,
            m_points_list[2][index].value);
    }

    return std::format("{}({}) :   Invalid Input",
        TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType()), index);
}

void BollingerBandIndicatorWrapper::FromJson(Json::Value value)
{
    GenericIndicatorWrapper::FromJson(value);

    BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(m_indicator.get());
    assert(bollinger_band);

    bollinger_band->SetStandardDeviationMultiplier(value["Multiplier"].asDouble());
}

Json::Value BollingerBandIndicatorWrapper::ToJson() const
{
    BollingerBand* bollinger_band = dynamic_cast<BollingerBand*>(m_indicator.get());
    assert(bollinger_band);

    Json::Value json_indicator = GenericIndicatorWrapper::ToJson();
    json_indicator["Multiplier"] = bollinger_band->GetStandardDeviationMultiplier();

    return json_indicator;
}


/*********************************************************************************
*                                ROC
**********************************************************************************/
void ROCIndicatorWrapper::PlotItems(ImVec4 bull_color, ImVec4 bear_color)
{
    if (m_points_list.size() > 0 && m_points_list[0].size() > 0)
    {
        if (ImPlot::BeginItem("Zero Line"))
        {
            ImDrawList* draw_list = ImPlot::GetPlotDrawList();    
            ImU32 color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
            double left = std::chrono::duration_cast<std::chrono::seconds>(m_points_list[0][m_points_list[0].size() - 1].date.time_since_epoch()).count();
            double right = std::chrono::duration_cast<std::chrono::seconds>(m_points_list[0][0].date.time_since_epoch()).count();
            
            ImVec2 left_point = ImPlot::PlotToPixels(left, 0);
            ImVec2 right_point = ImPlot::PlotToPixels(right, 0);

            draw_list->AddLine(left_point, right_point, color);

            ImPlot::EndItem();
        }
    }
    
    GenericChartIndicatorWrapper::PlotItems(bull_color, bear_color);
}

/*********************************************************************************
*                                RSI
**********************************************************************************/
void RSIIndicatorWrapper::PlotItems(ImVec4 bull_color, ImVec4 bear_color)
{
    if (m_points_list.size() > 0 && m_points_list[0].size() > 0)
    {
        if (ImPlot::BeginItem("50 Line and Zones"))
        {
            ImDrawList* draw_list = ImPlot::GetPlotDrawList();
            ImU32 color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
            double left = std::chrono::duration_cast<std::chrono::seconds>(m_points_list[0][m_points_list[0].size() - 1].date.time_since_epoch()).count();
            double right = std::chrono::duration_cast<std::chrono::seconds>(m_points_list[0][0].date.time_since_epoch()).count();

            ImVec2 left_point = ImPlot::PlotToPixels(left, 50);
            ImVec2 right_point = ImPlot::PlotToPixels(right, 50);

            // 50 line
            draw_list->AddLine(left_point, right_point, color);

            // Over Bought Zone
            ImVec2 over_bought_top_left = ImPlot::PlotToPixels(left, 100);
            ImVec2 over_bought_bottom_right = ImPlot::PlotToPixels(right, 70);

            ImVec4 tmp_bear_color = bear_color;
            tmp_bear_color.w = 0.1;
            draw_list->AddRectFilled(over_bought_top_left, over_bought_bottom_right, ImGui::GetColorU32(tmp_bear_color));

            // Over Sold Zone
            ImVec2 over_sold_top_left = ImPlot::PlotToPixels(left, 30);
            ImVec2 over_sold_bottom_right = ImPlot::PlotToPixels(right, 0);

            ImVec4 tmp_bull_color = bull_color;
            tmp_bull_color.w = 0.1;
            draw_list->AddRectFilled(over_sold_top_left, over_sold_bottom_right, ImGui::GetColorU32(tmp_bull_color));

            ImPlot::EndItem();
        }
    }

    GenericChartIndicatorWrapper::PlotItems(bull_color, bear_color);
}

/*********************************************************************************
*                                     OBV
**********************************************************************************/

bool OBVIndicatorWrapper::DrawAsAvailableIndicator()
{
    bool is_pressed = false;

    /// @begin Text
    ImGui::SetNextItemWidth(350);
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text


    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" + ##{}", m_id).c_str(), { 0, 0 }))
    {
        std::shared_ptr<Indicator> new_indicator = m_indicator->Clone();
        new_indicator->SetCounter(m_counter);

        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Apply {}", m_indicator->GetName()).c_str());
    }
    /// @end Button

    return is_pressed;
}

bool OBVIndicatorWrapper::DrawAsAppliedIndicator()
{
    bool is_pressed = false;

    /// @begin Text
    ImGui::SetNextItemWidth(250);
    ImGui::Checkbox(std::format("##show/hide{}", m_id).c_str(), &m_show); ImGui::SameLine();
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text


    ImGui::ColorEdit4(std::format("##plot color{}", m_id).c_str(), &m_colors_list[0].x, ImGuiColorEditFlags_NoInputs);

    ::ImGui::SameLine();
    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" x ##remove indicator{}", m_id).c_str(), { 0, 0 }))
    {
        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Remove {}", m_indicator->GetName()).c_str());
    }

    return is_pressed;
}




/*********************************************************************************
*                                     MACD
**********************************************************************************/

void MACDIndicatorWrapper::SetIndicator(std::unique_ptr<Indicator> indicator)
{
    GenericChartIndicatorWrapper::SetIndicator(std::move(indicator));

    m_colors_list.clear();
    m_colors_list.push_back(TradinatorAppSpace::Utils::GetIndicatorColor(EIndicatorType::E_MACD, 0));
    m_colors_list.push_back(TradinatorAppSpace::Utils::GetIndicatorColor(EIndicatorType::E_MACD, 1));
    m_colors_list.push_back(TradinatorAppSpace::Utils::GetIndicatorColor(EIndicatorType::E_MACD, 2));
}

bool MACDIndicatorWrapper::DrawAsAvailableIndicator()
{
    bool is_pressed = false;

    MACD* macd = dynamic_cast<MACD*>(m_indicator.get());
    assert(macd);

    /// @begin Text
    ImGui::SetNextItemWidth(280);
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text

    /// @begin Input
    /// @ period 1
    ImGui::SetNextItemWidth(70);
    std::string period_1_str = std::format("{}", macd->GetPeriod_1());
    char period_1_input_str[4] = "";
    std::copy(period_1_str.begin(), period_1_str.end(), period_1_input_str);

    if (ImGui::InputText(std::format("##period 1{}", m_id).c_str(), period_1_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int period_1 = std::atoi(period_1_input_str);
        macd->SetPeriod_1(period_1);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Period 1");
    }
    ImGui::SameLine();


    /// @ period 2
    ImGui::SetNextItemWidth(70);
    std::string period_2_str = std::format("{}", macd->GetPeriod_2());
    char period_2_input_str[4] = "";
    std::copy(period_2_str.begin(), period_2_str.end(), period_2_input_str);

    if (ImGui::InputText(std::format("##period 2{}", m_id).c_str(), period_2_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int period_2 = std::atoi(period_2_input_str);
        macd->SetPeriod_2(period_2);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Period 2");
    }
    ImGui::SameLine();

    

    /// @ signal period
    ImGui::SetNextItemWidth(70);
    std::string signal_period_str = std::format("{}", macd->GetSignalPeriod());
    char signal_period_input_str[4] = "";
    std::copy(signal_period_str.begin(), signal_period_str.end(), signal_period_input_str);

    if (ImGui::InputText(std::format("##signal period{}", m_id).c_str(), signal_period_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int signal_period = std::atoi(signal_period_input_str);
        macd->SetSignalPeriod(signal_period);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Signal Period");
    }
    ImGui::SameLine();
    /// @end Input




    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" + ##{}", m_id).c_str(), { 0, 0 }))
    {
        std::shared_ptr<Indicator> new_indicator = m_indicator->Clone();
        new_indicator->SetCounter(m_counter);

        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Apply {}", m_indicator->GetName()).c_str());
    }

    return is_pressed;
}

bool MACDIndicatorWrapper::DrawAsAppliedIndicator()
{
    bool is_pressed = false;

    MACD* macd = dynamic_cast<MACD*>(m_indicator.get());
    assert(macd);

    /// @begin Text
    ImGui::SetNextItemWidth(250);
    ImGui::Checkbox(std::format("##show/hide{}", m_id).c_str(), &m_show); ImGui::SameLine();
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text



    /// @begin Input
    /// @ period 1
    ImGui::SetNextItemWidth(70);
    std::string period_1_str = std::format("{}", macd->GetPeriod_1());
    char period_1_input_str[4] = "";
    std::copy(period_1_str.begin(), period_1_str.end(), period_1_input_str);

    if (ImGui::InputText(std::format("##period 1{}", m_id).c_str(), period_1_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int period_1 = std::atoi(period_1_input_str);
        if (macd->GetPeriod_1() != period_1)
        {
            macd->SetPeriod_1(period_1);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Period 1");
    }
    ImGui::SameLine();


    /// @ period 2
    ImGui::SetNextItemWidth(70);
    std::string period_2_str = std::format("{}", macd->GetPeriod_2());
    char period_2_input_str[4] = "";
    std::copy(period_2_str.begin(), period_2_str.end(), period_2_input_str);

    if (ImGui::InputText(std::format("##period 2{}", m_id).c_str(), period_2_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int period_2 = std::atoi(period_2_input_str);
        if (macd->GetPeriod_2() != period_2)
        {
            macd->SetPeriod_2(period_2);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Period 2");
    }
    ImGui::SameLine();



    /// @ signal period
    ImGui::SetNextItemWidth(70);
    std::string signal_period_str = std::format("{}", macd->GetSignalPeriod());
    char signal_period_input_str[4] = "";
    std::copy(signal_period_str.begin(), signal_period_str.end(), signal_period_input_str);

    if (ImGui::InputText(std::format("##signal period{}", m_id).c_str(), signal_period_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int signal_period = std::atoi(signal_period_input_str);
        if (macd->GetSignalPeriod() != signal_period)
        {
            macd->SetSignalPeriod(signal_period);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Signal Period");
    }
    ImGui::SameLine();
    /// @end Input


    /// @begin Colors
    ImGui::ColorEdit4(std::format("##macd{}", m_id).c_str(), &m_colors_list[0].x, ImGuiColorEditFlags_NoInputs); ImGui::SameLine();
    ImGui::ColorEdit4(std::format("##signal{}", m_id).c_str(), &m_colors_list[1].x, ImGuiColorEditFlags_NoInputs); ImGui::SameLine();
    ImGui::ColorEdit4(std::format("##histogram{}", m_id).c_str(), &m_colors_list[2].x, ImGuiColorEditFlags_NoInputs); ImGui::SameLine();
    /// @end Colors



    ::ImGui::SameLine();
    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" x ##remove indicator{}", m_id).c_str(), { 0, 0 }))
    {
        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Remove {}", m_indicator->GetName()).c_str());
    }

    return is_pressed;
}

void MACDIndicatorWrapper::PlotItems(ImVec4 bull_color, ImVec4 bear_color)
{
    if (m_points_list.size() == 3)
    {
        ImVec4 macd_color = m_colors_list[0];
        ImPlot::SetNextLineStyle(macd_color, macd_color.w);
        ImPlot::PlotLineG(std::format("{}##Chart{}_{}", m_indicator->GetName(), m_counter->ISIN_Number(), m_id).c_str()
            , indicator_plot_point_getter
            , (void*)&m_points_list[0]
            , m_points_list[0].size());

        ImVec4 signal_color = m_colors_list[1];
        ImPlot::SetNextLineStyle(signal_color, signal_color.w);
        ImPlot::PlotLineG(std::format("Signal##Chart{}_{}", m_counter->ISIN_Number(), m_id).c_str()
            , indicator_plot_point_getter
            , (void*)&m_points_list[1]
            , m_points_list[1].size());

        ImVec4 histogram_color = m_colors_list[2];
        ImPlot::SetNextLineStyle(histogram_color, histogram_color.w);
        ImPlot::SetNextFillStyle(histogram_color, histogram_color.w);
        ImPlot::PlotBarsG(std::format("Histogram##Chart{}_{}", m_counter->ISIN_Number(), m_id).c_str()
            , indicator_plot_point_getter
            , (void*)&m_points_list[2]
            , m_points_list[2].size()
            , 60 * 60 * 12);
    }
}

std::string MACDIndicatorWrapper::GetHumanReadableValueAt(size_t index) const
{
    if (m_points_list.size() >= 3)
    {
        return std::format("{}({}) :   {}, {}, {}",
            TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType()), 
            index,
            m_points_list[0][index].value,
            m_points_list[1][index].value,
            m_points_list[2][index].value);
    }
    
    return std::format("{}({}) :   Invalid Input",
        TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType()), index);
}

void MACDIndicatorWrapper::FromJson(Json::Value value)
{
    MACD* macd = dynamic_cast<MACD*>(m_indicator.get());
    assert(macd);

    GenericChartIndicatorWrapper::FromJson(value);

    ImVec4 macd_color;
    macd_color.x = value["Colors"]["MACD"]["R"].asFloat();
    macd_color.y = value["Colors"]["MACD"]["G"].asFloat();
    macd_color.z = value["Colors"]["MACD"]["B"].asFloat();
    macd_color.w = value["Colors"]["MACD"]["A"].asFloat();

    ImVec4 signal_color;
    signal_color.x = value["Colors"]["Signal"]["R"].asFloat();
    signal_color.y = value["Colors"]["Signal"]["G"].asFloat();
    signal_color.z = value["Colors"]["Signal"]["B"].asFloat();
    signal_color.w = value["Colors"]["Signal"]["A"].asFloat();

    ImVec4 histogram_color;
    histogram_color.x = value["Colors"]["Histogram"]["R"].asFloat();
    histogram_color.y = value["Colors"]["Histogram"]["G"].asFloat();
    histogram_color.z = value["Colors"]["Histogram"]["B"].asFloat();
    histogram_color.w = value["Colors"]["Histogram"]["A"].asFloat();

    m_colors_list.clear();
    m_colors_list.push_back(macd_color);
    m_colors_list.push_back(signal_color);
    m_colors_list.push_back(histogram_color);

    macd->SetPeriod_1(value["Period1"].asInt64());
    macd->SetPeriod_2(value["Period2"].asInt64());
    macd->SetSignalPeriod(value["SignalPeriod"].asInt64());
}

Json::Value MACDIndicatorWrapper::ToJson() const
{
    MACD* macd = dynamic_cast<MACD*>(m_indicator.get());
    assert(macd);

    Json::Value json_indicator = GenericChartIndicatorWrapper::ToJson();
    json_indicator.removeMember("Color");
    json_indicator.removeMember("Length");
    
    Json::Value json_macd_color;
    json_macd_color["R"] = m_colors_list[0].x;
    json_macd_color["G"] = m_colors_list[0].y;
    json_macd_color["B"] = m_colors_list[0].z;
    json_macd_color["A"] = m_colors_list[0].w;

    Json::Value json_signal_color;
    json_signal_color["R"] = m_colors_list[1].x;
    json_signal_color["G"] = m_colors_list[1].y;
    json_signal_color["B"] = m_colors_list[1].z;
    json_signal_color["A"] = m_colors_list[1].w;

    Json::Value json_histogram_color;
    json_histogram_color["R"] = m_colors_list[2].x;
    json_histogram_color["G"] = m_colors_list[2].y;
    json_histogram_color["B"] = m_colors_list[2].z;
    json_histogram_color["A"] = m_colors_list[2].w;

    Json::Value json_colors;
    json_colors["MACD"] = json_macd_color;
    json_colors["Signal"] = json_signal_color;
    json_colors["Histogram"] = json_histogram_color;

    json_indicator["Colors"] = json_colors;

    json_indicator["Period1"] = macd->GetPeriod_1();
    json_indicator["Period2"] = macd->GetPeriod_2();
    json_indicator["SignalPeriod"] = macd->GetSignalPeriod();
    
    return json_indicator;
}


/*********************************************************************************
*                                Savitzky Golay Filter
**********************************************************************************/

bool TrendAnalysisDebugWrapper::DrawAsAvailableIndicator()
{
    bool is_pressed = false;

    TrendAnalysisDebug* trend_analysis_debug = dynamic_cast<TrendAnalysisDebug*>(m_indicator.get());
    assert(trend_analysis_debug);

    /// @begin Text
    ImGui::SetNextItemWidth(280);
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text

    /// @begin Input
    /// @ period 1
    ImGui::SetNextItemWidth(70);
    std::string window_size_str = std::format("{}", trend_analysis_debug->GetLength());
    char window_size_input_str[4] = "";
    std::copy(window_size_str.begin(), window_size_str.end(), window_size_input_str);

    if (ImGui::InputText(std::format("##window_size{}", m_id).c_str(), window_size_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int window_size = std::atoi(window_size_input_str);
        trend_analysis_debug->SetLength(window_size);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Window Size");
    }
    ImGui::SameLine();


    /// @ period 2
    ImGui::SetNextItemWidth(70);
    std::string polynomial_order_str = std::format("{}", trend_analysis_debug->GetPolynomialOrder());
    char polynomial_order_input_str[4] = "";
    std::copy(polynomial_order_str.begin(), polynomial_order_str.end(), polynomial_order_input_str);

    if (ImGui::InputText(std::format("##period 2{}", m_id).c_str(), polynomial_order_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int polynomial_order = std::atoi(polynomial_order_input_str);
        trend_analysis_debug->SetPolynomialOrder(polynomial_order);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Polynomial Order (Must be odd number)");
    }
    ImGui::SameLine();


    /// @ Distance
    ImGui::SetNextItemWidth(70);
    std::string distance_str = std::format("{}", trend_analysis_debug->GetDistance());
    char distance_input_str[4] = "";
    std::copy(distance_str.begin(), distance_str.end(), distance_input_str);

    if (ImGui::InputText(std::format("##distance{}", m_id).c_str(), distance_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int distance = std::atoi(distance_input_str);
        trend_analysis_debug->SetDistance(distance);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Min distance between each peak.");
    }
    ImGui::SameLine();


    /// @ Width
    ImGui::SetNextItemWidth(70);
    std::string width_str = std::format("{}", trend_analysis_debug->GetWidth());
    char width_input_str[4] = "";
    std::copy(width_str.begin(), width_str.end(), width_input_str);

    if (ImGui::InputText(std::format("##width{}", m_id).c_str(), width_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int width = std::atoi(width_input_str);
        trend_analysis_debug->SetWidth(width);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Width for finding peaks.");
    }
    ImGui::SameLine();


    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" + ##{}", m_id).c_str(), { 0, 0 }))
    {
        std::shared_ptr<Indicator> new_indicator = m_indicator->Clone();
        new_indicator->SetCounter(m_counter);

        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Apply {}", m_indicator->GetName()).c_str());
    }

    return is_pressed;
}

bool TrendAnalysisDebugWrapper::DrawAsAppliedIndicator()
{
    bool is_pressed = false;

    TrendAnalysisDebug* trend_analysis_debug = dynamic_cast<TrendAnalysisDebug*>(m_indicator.get());
    assert(trend_analysis_debug);


    /// @begin Text
    ImGui::Checkbox(std::format("##show/hide{}", m_id).c_str(), &m_show); ImGui::SameLine();
    ImGui::SetNextItemWidth(250);
    ImGui::TextUnformatted(m_indicator->GetName().c_str()); ImGui::SameLine();
    /// @end Text



    /// @begin Input
    /// @ period 1
    ImGui::SetNextItemWidth(70);

    std::string window_size_str = std::format("{}", trend_analysis_debug->GetLength());
    char window_size_input_str[4] = "";
    std::copy(window_size_str.begin(), window_size_str.end(), window_size_input_str);

    if (ImGui::InputText(std::format("##window_size{}", m_id).c_str(), window_size_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int window_size = std::atoi(window_size_input_str);
        if (trend_analysis_debug->GetLength() != window_size)
        {
            trend_analysis_debug->SetLength(window_size);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Window Size");
    }
    ImGui::SameLine();


    /// @ Polynomial order
    ImGui::SetNextItemWidth(70);
    std::string polynomial_order_str = std::format("{}", trend_analysis_debug->GetPolynomialOrder());
    char polynomial_order_input_str[4] = "";
    std::copy(polynomial_order_str.begin(), polynomial_order_str.end(), polynomial_order_input_str);

    bool is_polynomial_order_valid = trend_analysis_debug->GetPolynomialOrder() % 2 != 0 &&
        trend_analysis_debug->GetPolynomialOrder() >= 5;
    if (!is_polynomial_order_valid)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(237, 67, 55, 255));
    }
    if (ImGui::InputText(std::format("##polynomial_order{}", m_id).c_str(), polynomial_order_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int polynomial_order = std::atoi(polynomial_order_input_str);
        if (trend_analysis_debug->GetPolynomialOrder() != polynomial_order)
        {
            trend_analysis_debug->SetPolynomialOrder(polynomial_order);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Polynomial Order (Must be odd number and greater than or equal to 5)");
    }
    if (!is_polynomial_order_valid)
    {
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();



    /// @ Distance
    ImGui::SetNextItemWidth(70);
    std::string distance_str = std::format("{}", trend_analysis_debug->GetDistance());
    char distance_input_str[4] = "";
    std::copy(distance_str.begin(), distance_str.end(), distance_input_str);

    if (ImGui::InputText(std::format("##distance{}", m_id).c_str(), distance_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int distance = std::atoi(distance_input_str);
        if (trend_analysis_debug->GetDistance() != distance)
        {
            trend_analysis_debug->SetDistance(distance);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Min distance between each peak.");
    }
    ImGui::SameLine();


    /// @ Width
    ImGui::SetNextItemWidth(70);
    std::string width_str = std::format("{}", trend_analysis_debug->GetWidth());
    char width_input_str[4] = "";
    std::copy(width_str.begin(), width_str.end(), width_input_str);

    if (ImGui::InputText(std::format("##width{}", m_id).c_str(), width_input_str, 4, ImGuiInputTextFlags_CharsDecimal))
    {
        int width = std::atoi(width_input_str);
        if (trend_analysis_debug->GetWidth() != width)
        {
            trend_analysis_debug->SetWidth(width);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Width for finding peaks.");
    }
    ImGui::SameLine();


    /// @ rel_width
    ImGui::SetNextItemWidth(70);
    std::string rel_width_str = std::format("{:.2f}", trend_analysis_debug->GetRelativeWidth());
    char rel_width_input_str[10] = "";
    std::copy(rel_width_str.begin(), rel_width_str.end(), rel_width_input_str);

    if (ImGui::InputText(std::format("##rel_width{}", m_id).c_str(), rel_width_input_str, 10, ImGuiInputTextFlags_CharsDecimal))
    {
        double rel_width = std::atof(rel_width_input_str);
        if (fabs(trend_analysis_debug->GetRelativeWidth() - rel_width) > LDBL_EPSILON)
        {
            trend_analysis_debug->SetRelativeWidth(rel_width);

            Calculate();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("rel_width for finding peaks.");
    }
    ImGui::SameLine();


    /// @begin Colors
    ImGui::ColorEdit4(std::format("##trend_analysis_debug_color{}", m_id).c_str(), &m_colors_list[0].x, ImGuiColorEditFlags_NoInputs); ImGui::SameLine();
    /// @end Colors


    /// @begin Button
    ImGui::SetNextItemWidth(50);
    if (ImGui::Button(std::format(" + ##{}", m_id).c_str(), { 0, 0 }))
    {
        is_pressed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(std::format("Apply {}", m_indicator->GetName()).c_str());
    }

    return is_pressed;
}

void TrendAnalysisDebugWrapper::PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color)
{
    GenericIndicatorWrapper::PlotPostCandle(bull_color, bear_color);

    if (m_points_list.size() < 1) return;

    size_t count = m_points_list[1].size();
    if (count == 0) return;

    ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 10.0f, bear_color, 1.0f, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImPlot::PlotScatterG(std::format("{}##peaks", m_indicator->GetName()).c_str(), indicator_plot_point_getter, (void*)&m_points_list[1], count);




    if (m_points_list.size() < 2) return;

    count = m_points_list[2].size();
    if (count == 0) return;

    ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 10.0f, bull_color, 1.0f, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImPlot::PlotScatterG(std::format("{}##trough", m_indicator->GetName()).c_str(), indicator_plot_point_getter, (void*)&m_points_list[2], count);
}

std::string TrendAnalysisDebugWrapper::GetHumanReadableValueAt(size_t index) const
{
    if (m_points_list.size() >= 4)
    {
        double trend = m_points_list[3][index].value;
        std::string trend_str = "None";
        if (trend > 0.9)
        {
            trend_str = "Up";
        }
        else if(trend < -0.9)
        {
            trend_str = "Down";
        }
        return std::format("{}({}) :   {}",
            TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType()),
            index,
            trend_str);
    }

    return std::format("{}({}) :   Invalid Input",
        TradinatorAppSpace::Utils::GetIndicatorTypeStr(m_indicator->IndicatorType()), index);
}

void TrendAnalysisDebugWrapper::FromJson(Json::Value value)
{
    TrendAnalysisDebug* trend_analysis_debug = dynamic_cast<TrendAnalysisDebug*>(m_indicator.get());
    assert(trend_analysis_debug);

    GenericIndicatorWrapper::FromJson(value);
    trend_analysis_debug->SetPolynomialOrder(value["PolynomialOrder"].asUInt());
    trend_analysis_debug->SetDistance(value["Distance"].asUInt());
    trend_analysis_debug->SetWidth(value["Width"].asUInt());
}

Json::Value TrendAnalysisDebugWrapper::ToJson() const
{
    TrendAnalysisDebug* trend_analysis_debug = dynamic_cast<TrendAnalysisDebug*>(m_indicator.get());
    assert(trend_analysis_debug);

    Json::Value json_indicator = GenericIndicatorWrapper::ToJson();
    json_indicator["PolynomialOrder"] = trend_analysis_debug->GetPolynomialOrder();
    json_indicator["Distance"] = trend_analysis_debug->GetDistance();
    json_indicator["Width"] = trend_analysis_debug->GetWidth();

    return json_indicator;
}