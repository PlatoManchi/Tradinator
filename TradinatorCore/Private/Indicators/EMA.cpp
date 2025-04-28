#include "Indicators/EMA.h"

#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"

#ifdef _EMA_ISPC_
#include "indicator_helper_ispc.h"
#endif // _EMA_ISPC_



std::vector<std::vector<IndicatorPoint>> EMA::Calculate()
{
	std::vector<std::vector<IndicatorPoint>> result;
	if (m_length == 0) return result;

	std::shared_ptr<Counter> counter = m_counter.lock();

	if (counter)
	{
		const std::shared_ptr<const AsyncData<AsyncCandleData>>& candle_data = counter->GetCandleData();
		bool is_ready = candle_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candle_data->IsDataReady();
		}

		//std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		size_t count = candle_data->GetData().size();
		if (count == 0) return result;
		
		std::vector<IndicatorPoint> ema;

#ifdef _EMA_ISPC_
		ema.reserve(count);

		const AsyncCandleData& data = candle_data->GetData();
		std::vector<double> ispc_input;
		std::vector<double> ispc_output(count);
		ispc_input.reserve(count);

		// ISPC doesn't like reading from back to front. So reverse the array here and unreverse the output
		auto itr = candle_data->GetData().end();
		for (size_t i = 0; i < count; ++i)
		{
			itr = std::prev(itr, 1);
			ispc_input.emplace_back((*itr).second.m_close);
		}

		ispc::calculate_ema(ispc_input.data(), ispc_output.data(), count, m_length);

		itr = candle_data->GetData().end();
		for (double sma : ispc_output)
		{
			itr = std::prev(itr, 1);

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = sma;

			ema.emplace_back(std::move(point));
		}
#else
		ema = std::move(std::vector<IndicatorPoint>(count));

		auto itr = candle_data->GetData().end();

		itr = std::prev(itr, 1);

		// 0th element is same value as closing
		IndicatorPoint first_point;
		first_point.date = (*itr).first;
		first_point.value = (*itr).second.m_close;

		ema[count - 1] = first_point;

		const double factor = 2.0 / (m_length + 1.0);

		itr = std::prev(itr, 1);

		for (int64_t i = count - 2; i >= 0; --i)
		{
			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = (*itr).second.m_close * factor + ema[i + 1].value * (1.0 - factor);

			ema[i] = point;

			itr = std::prev(itr, 1);
		}
#endif // _EMA_ISPC_
		
		result.emplace_back(std::move(ema));

		//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		//std::cout << "EMA Took " << std::to_string(std::chrono::duration<double>(end - start).count()) << " sec" << std::endl;
	}

	return result;
}

void EMA::CalculateRaw(double* input, double* output, int64_t data_size, int64_t window_size)
{
	output[data_size - 1] = input[data_size - 1];

	const double factor = 2.0 / (window_size + 1.0);
	for (int64_t i = data_size - 2; i >= 0; --i)
	{
		output[i] = input[i] * factor + output[i + 1] * (1.0 - factor);
	}
}