#include "Indicators/RSI.h"

#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"
#include "Utils/StopWatch.h"

#ifdef _RSI_ISPC_
#include "indicator_helper_ispc.h"
#endif // _RSI_ISPC_



std::vector<std::vector<IndicatorPoint>> RSI::Calculate()
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

		std::vector<IndicatorPoint> rsi;
		rsi.reserve(count);


#ifdef _RSI_ISPC_
		const CandleDataMapType& data = candle_data->GetData();
		std::vector<double> ispc_input;
		std::vector<double> ispc_output(count);
		ispc_input.reserve(count);

		for (auto& pair : data)
		{
			ispc_input.emplace_back(pair.second.m_close);
		}

		ispc::calculate_rsi(ispc_input.data(), ispc_output.data(), count, m_length);

		auto itr = candle_data->GetData().begin();
		for (double sma : ispc_output)
		{
			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = sma;

			rsi.emplace_back(std::move(point));

			std::advance(itr, 1);
		}
#else
		auto itr = candle_data->GetData().begin();
		auto end_itr = candle_data->GetData().end();

		for (size_t i = 0; i < count-1; ++i)
		{
			size_t window_size = i + m_length < count - 1 ? m_length : count - i - 1;
			auto tmp_itr = itr;
			
			double cumulative_gain = 0;
			double cumulative_loss = 0;

			for (size_t j = 0; j < window_size; ++j)
			{
				double current = (*tmp_itr).second.m_close;

				std::advance(tmp_itr, 1);
				
				double prev = (*tmp_itr).second.m_close;

				double diff = current - prev;
				if (diff > 0)
					cumulative_gain += diff;
				else if (diff < 0)
					cumulative_loss += diff;
			}
			double relative_strength = cumulative_gain / fabs(cumulative_loss);
			double relative_strength_index = 100.0 - 100.0 / (1 + relative_strength);

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = relative_strength_index;

			rsi.emplace_back(std::move(point));

			std::advance(itr, 1);
		}

		IndicatorPoint point;
		point.date = (*itr).first;
		point.value = 0;

		rsi.emplace_back(std::move(point));
#endif // _RSI_ISPC_

		result.emplace_back(std::move(rsi));
	}

	return result;
}