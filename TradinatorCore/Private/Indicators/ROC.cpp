#include "Indicators/ROC.h"

#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"
#include "Utils/StopWatch.h"

#ifdef _ROC_ISPC_
#include "indicator_helper_ispc.h"
#endif // _ROC_ISPC_



std::vector<std::vector<IndicatorPoint>> ROC::Calculate()
{
	std::vector<std::vector<IndicatorPoint>> result;
	if (m_length == 0) return result;

	std::shared_ptr<Counter> counter = m_counter.lock();

	if (counter)
	{
		const std::shared_ptr<const AsyncData<CandleDataMapType>>& candle_data = counter->GetCandleData();
		bool is_ready = candle_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candle_data->IsDataReady();
		}

		//StopWatch stop_watch(GetName());

		size_t count = candle_data->GetData().size();
		if (count == 0) return result;

		std::vector<IndicatorPoint> roc;
		roc.reserve(count);

#ifdef _ROC_ISPC_
		const CandleDataMapType& data = candle_data->GetData();
		std::vector<double> ispc_input;
		std::vector<double> ispc_output(count);
		ispc_input.reserve(count);

		for (auto& pair : data)
		{
			ispc_input.emplace_back(pair.second.m_close);
		}

		ispc::calculate_roc(ispc_input.data(), ispc_output.data(), count, m_length);

		auto itr = candle_data->GetData().begin();
		for (double sma : ispc_output)
		{
			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = sma;

			roc.emplace_back(std::move(point));

			std::advance(itr, 1);
		}
#else
		auto itr = candle_data->GetData().begin();
		auto end_itr = candle_data->GetData().end();
		for (size_t i = 0 ; i < count; ++i)
		{
			size_t window_count = i + m_length < count ? m_length : count - i;

			double current = (*itr).second.m_close;
			auto tmp_itr = std::next(itr, window_count);
			double compare_with = tmp_itr != end_itr ? (*tmp_itr).second.m_close : current;

			double roc_value = ((current - compare_with) / compare_with) * 100.0;

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = roc_value;

			roc.emplace_back(point);

			std::advance(itr, 1);
		}
#endif // _ROC_ISPC_

		result.emplace_back(std::move(roc));
	}

	return result;
}