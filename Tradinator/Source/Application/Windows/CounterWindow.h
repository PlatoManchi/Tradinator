#pragma once

#include <memory>
#include <string>

#include "imgui.h"
#include "implot.h"
#include "implot_internal.h"
#include  "imspinner/imspinner.h"

class Counter;

class CounterWindow
{
public:
	CounterWindow(std::shared_ptr<Counter> counter);
	void Show();

	bool m_close;
	bool m_maximize;
private:
	void PlotCandlestick(const char* label_id, const double* xs, const double* opens, const double* closes, const double* lows, const double* highs, int count, bool tooltip = true, float width_percent = 0.25f, ImVec4 bullCol = ImVec4(0, 1, 0, 1), ImVec4 bearCol = ImVec4(1, 0, 0, 1));

	template <typename T>
	int BinarySearch(const T* arr, int l, int r, T x);

	std::string m_cached_label_id;
	std::shared_ptr<Counter> m_counter;
};

