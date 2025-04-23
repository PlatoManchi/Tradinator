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
		std::vector<IndicatorPoint> m_points;
		ImVec4 m_color;
		bool m_show = true;
		size_t m_id = 0;
	};


	void ShowTitle();
	void ShowIndicatorsList();
	void ShowAvailableIndicator(const std::unique_ptr<Indicator>& indicator);
	void ShowAppliedIndicator(const std::shared_ptr<Indicator>& indicator);
	void AddIndicator(std::shared_ptr<Indicator> indicator);

	void RebuildCachedPlotPoints();
	void PlotCandlestick(const char* label_id, const double* xs, const double* opens, const double* closes, const double* lows, const double* highs, int count, bool tooltip = true, float width_percent = 0.25f, ImVec4 bullCol = ImVec4(0, 1, 0, 1), ImVec4 bearCol = ImVec4(1, 0, 0, 1));
	void PlotIndicator(const std::shared_ptr<Indicator>& indicator);

	template <typename T>
	int BinarySearch(const T* arr, int l, int r, T x);


	std::vector<double> m_dates;
	std::vector<double> m_opens;
	std::vector<double> m_highes;
	std::vector<double> m_lows;
	std::vector<double> m_closes;
	std::vector<int64_t> m_volumes;
	std::vector<int64_t> m_open_interests;

	std::vector<std::unique_ptr<Indicator>> m_available_indicators;
	std::map<std::shared_ptr<Indicator>, IndicatorData> m_applied_indicators_data;
	std::vector<std::shared_ptr<Indicator>> m_remove_applied_indicators;
	
	double x_axis_min, x_axis_max, y_axis_min, y_axis_max;

	std::string m_cached_label_id;
	std::shared_ptr<Counter> m_counter;

	bool m_is_dirty;

	static size_t _INCREMENTAL_ID_;
};

