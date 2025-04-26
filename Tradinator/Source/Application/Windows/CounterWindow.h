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

class Counter;

class CounterWindow
{
public:
	CounterWindow(std::shared_ptr<Counter> counter);
	~CounterWindow();

	void Show();

	Json::Value GetCounterStatus();
	void SetCounterStatus(Json::Value status);

	bool m_close;
	bool m_maximize;


private:
	struct IndicatorData
	{
		std::vector<IndicatorPoint> m_top_points;
		std::vector<IndicatorPoint> m_points;
		std::vector<IndicatorPoint> m_bottom_points;

		ImVec4 m_color;
		bool m_show = true;
		bool m_is_hovered = false;
		size_t m_id = 0;

		ImPlotRect m_chart_limits;
		float m_label_width;
	};


	void ShowTitle();
	void ShowIndicatorsList();
	void ShowAvailableIndicator(const std::unique_ptr<Indicator>& indicator);
	void ShowAppliedIndicator(const std::shared_ptr<Indicator>& indicator);
	void AddIndicator(std::shared_ptr<Indicator> indicator);
	void AddIndicator(std::shared_ptr<Indicator> indicator, ImVec4 color);

	void RebuildCachedPlotPoints();
	void PlotCandlestick(const char* label_id, const size_t* xs, const double* opens, const double* closes, const double* lows, const double* highs, int count, bool tooltip = true, float width_percent = 0.25f, ImVec4 bullCol = ImVec4(0, 1, 0, 1), ImVec4 bearCol = ImVec4(1, 0, 0, 1));
	void PlotIndicator(const std::shared_ptr<Indicator>& indicator);
	void PlotEnvelopeIndicatorFill(const std::shared_ptr<Indicator>& indicator);
	void PlotEnvelopeIndicatorLines(const std::shared_ptr<Indicator>& indicator);

	//template <typename T>
	int BinarySearch(const size_t* arr, int l, int r, double x);


	std::vector<size_t> m_dates;
	std::vector<double> m_opens;
	std::vector<double> m_highes;
	std::vector<double> m_lows;
	std::vector<double> m_closes;
	std::vector<size_t> m_volumes;
	std::vector<size_t> m_open_interests;

	std::vector<std::unique_ptr<Indicator>> m_available_indicators;
	std::map<std::shared_ptr<Indicator>, IndicatorData> m_applied_indicators_data;
	std::vector<std::shared_ptr<Indicator>> m_remove_applied_indicators;
	
	size_t date_axis_min, date_axis_max;
	double price_axis_min, price_axis_max;
	size_t volume_axis_min, volume_axis_max;

	bool m_is_price_chart_hovered;
	bool m_is_volume_chart_hovered;

	float m_price_chart_height;

	std::string m_cached_label_id;
	std::shared_ptr<Counter> m_counter;

	bool m_is_dirty;

	ImPlotRect m_shared_limits;

	ImPlotRect m_price_chart_limits;
	ImPlotRect m_volume_chart_limits;

	static size_t _INCREMENTAL_ID_;
};

