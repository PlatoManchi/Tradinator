#include "Components/IndicatorWrappers.h"

#include "Data/Counter.h"
#include "Indicators/BollingerBand.h"

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

    if (ImGui::InputText(std::format("##length{}", m_indicator->GetName()).c_str(), length_str, 4, ImGuiInputTextFlags_CharsDecimal))
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
    if (ImGui::Button(std::format(" + ##{}", m_indicator->GetName()).c_str(), { 0, 0 }))
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
        m_indicator->SetLength(length);

        m_points_list.clear();
        m_points_list.push_back(std::move(m_indicator->Calculate()));
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
    m_points_list.push_back(std::move(m_indicator->Calculate()));
}

void GenericIndicatorWrapper::PlotPreCandle()
{

}

void GenericIndicatorWrapper::PlotPostCandle()
{
    size_t count = m_points_list[0].size();

    if (count != 0)
    {
        ImPlot::SetNextLineStyle(m_colors_list[0], m_colors_list[0].w);
        ImPlot::PlotLineG("Indicator", indicator_plot_point_getter, (void*)&m_points_list[0], count);
    }
}

void GenericIndicatorWrapper::FromJson(Json::Value value)
{
    EIndicatorType type = TradinatorAppSpace::Utils::GetIndicatorType(value["Name"].asString());
    m_indicator = std::move(TradinatorAppSpace::Utils::GetIndicator(type));
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

void GenericChartIndicatorWrapper::PlotPreCandle()
{
    assert("Chart indicator will draw its own chart. This shouldn't be called. Call DrawCustomChart to draw the chart.");
}

void GenericChartIndicatorWrapper::PlotPostCandle()
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

void GenericChartIndicatorWrapper::DrawCustomChart(double chart_height, ImPlotAxisFlags x_axis_flags, ImPlotAxisFlags y_axis_flags, ImPlotRect& shared_limits)
{
    assert(m_counter);
    assert(!IsIndicatorOverlayable());
    
    
    if (ImPlot::BeginPlot(std::format("{}##{}_{}", m_indicator->GetName(), m_counter->ISIN_Number(), m_id).c_str(), ImVec2(-1, chart_height), ImPlotFlags_NoTitle))
    {
        ImPlot::SetupAxes(nullptr, nullptr, x_axis_flags, y_axis_flags);

        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
        if (m_is_hovered)
        {
            ImPlot::SetupAxisLimits(ImAxis_X1, m_x_axis_min, m_x_axis_max);
        }
        else
        {
            ImPlot::SetupAxisLimits(ImAxis_X1, shared_limits.X.Min, shared_limits.X.Max, ImGuiCond_Always);
        }
        //ImPlot::SetupAxisLimits(ImAxis_Y1, volume_axis_min, volume_axis_max);

        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, m_x_axis_min, m_x_axis_max);
        ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 60 * 60 * 24 * 14, m_x_axis_max - m_x_axis_min); // 14 days at min and full chat at max

        PlotItems();
        
        m_chart_limits = ImPlot::GetPlotLimits();

        if (ImPlot::IsPlotHovered())
        {
            shared_limits = ImPlot::GetPlotLimits();
            m_is_hovered = true;
        }
        else
        {
            m_is_hovered = false;
        }
        ImPlot::EndPlot();
    }
}

void GenericChartIndicatorWrapper::PlotItems()
{
    for (int i = 0; i < m_points_list.size(); ++i)
    {
        ImVec4 color = m_colors_list[0];
        if (i < m_colors_list.size())
        {
            color = m_colors_list[i];
        }

        ImPlot::SetNextLineStyle(color, color.w);
        ImPlot::PlotLineG(std::format("{}##Chart{}_{}", m_indicator->GetName(), m_counter->ISIN_Number(), m_id).c_str()
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

    if (ImGui::InputText(std::format("##length{}", m_indicator->GetName()).c_str(), length_str, 4, ImGuiInputTextFlags_CharsDecimal))
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

    if (ImGui::InputText(std::format("##standard deviation multiplier{}", m_indicator->GetName()).c_str(), multiplier_str, 5, ImGuiInputTextFlags_CharsDecimal))
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
    if (ImGui::Button(std::format(" + ##{}", m_indicator->GetName()).c_str(), { 0, 0 }))
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
        m_indicator->SetLength(length);
        m_points_list = std::move(bollinger_band->CalculateEnvelope());
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
        m_points_list = std::move(bollinger_band->CalculateEnvelope());
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
    m_points_list = std::move(bollinger_band->CalculateEnvelope());
}

void BollingerBandIndicatorWrapper::PlotPreCandle()
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

void BollingerBandIndicatorWrapper::PlotPostCandle()
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
    if (ImGui::Button(std::format(" + ##{}", m_indicator->GetName()).c_str(), { 0, 0 }))
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
