#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <vector>
#include <map>

#include "imgui.h"
#include "implot.h"
#include "implot_internal.h"
#include  "imspinner/imspinner.h"

#include "json/json.h"

#include "Indicators/Indicator.h"
#include "Application/Windows/AutoAnalysisUpdateWindow.h"
#include "TradinatorTypes.h"

class Security;
class IIndicatorWrapper;


class SecurityWindow
{
public:
	SecurityWindow(std::shared_ptr<Security> security);
	~SecurityWindow();

	void Show();

	void HighlightDateIndex(NewsPoint news);

	Json::Value GetSecurityStatus();
	void SetSecurityStatus(Json::Value status);

	bool m_close;
	bool m_maximize;


private:
	void ShowTitle();
	void ShowIndicatorsList();
	bool CanApplyIndicatorOfType(EIndicatorType type);
	
	void ShowPatterns(float chart_width, float chart_height,  ImPlotRect chart_limits);
	void ShowHilights(ImPlotRect limit);
	void RebuildCachedPlotPoints();
	void PlotCandlestick(const char* label_id, const size_t* xs, const double* opens, const double* closes, const double* lows, const double* highs, const uint64_t* volumes, int count, bool tooltip = true, float width_percent = 0.25f, ImVec4 bullCol = ImVec4(0, 1, 0, 1), ImVec4 bearCol = ImVec4(1, 0, 0, 1));
	
	//template <typename T>
	int BinarySearch(const size_t* arr, int l, int r, double x);


	std::vector<uint64_t> m_dates;

	std::vector<std::unique_ptr<IIndicatorWrapper>> m_available_indicator_wrappers;
	std::vector<std::unique_ptr<IIndicatorWrapper>> m_applied_indicator_wrappers;
	std::vector<size_t> m_remove_applied_indicator_ids;

	size_t date_axis_min, date_axis_max;
	double price_axis_min, price_axis_max;
	size_t volume_axis_min, volume_axis_max;

	bool m_is_price_chart_hovered;
	bool m_is_volume_chart_hovered;

	float m_price_chart_height;

	std::string m_cached_label_id;
	std::shared_ptr<Security> m_security;

	bool m_is_dirty;

	ImPlotRect m_shared_limits;

	ImPlotRect m_price_chart_limits;
	ImPlotRect m_volume_chart_limits;

	bool m_show_tool_tip;
	bool m_show_patterns;
	bool m_tooltip_override;
	bool m_is_any_plot_hovered;
	ImPlotPoint m_current_hovered_plot_mouse_location;
	float m_hovered_highlight_l, m_hovered_highlight_r;

	ImVec4 m_bull_color = ImVec4(0.031f, 0.600f, 0.505f, 1.000f);
	ImVec4 m_bear_color = ImVec4(0.949f, 0.211f, 0.270f, 1.000f);

	float m_first_time_chart_limit_x_min;
	float m_first_time_chart_limit_x_max;
	bool m_is_first_time_limit_set;

	bool m_is_highlight_date_index = false;
	std::vector<uint64_t> m_highlight_date_index;

	std::chrono::steady_clock::time_point m_last_load_request_time;

	AutoAnalysisUpdateWindow m_auto_analysis_update_window;
	std::vector<HilightsAnimationData> m_hilights_animation_data;
};

