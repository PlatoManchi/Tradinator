#include "Indicators/WMA.h"

#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"

#include "indicator_ispc.h"

std::vector<IndicatorPoint> WMA::Calculate()
{
	std::vector<IndicatorPoint> result;
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
		result.reserve(count);

		/*if (count > m_length)
		{
			auto itr = candle_data->GetData().begin();
			for (size_t i = 0; i < count; ++i)
			{
				size_t window_count = i + m_length < count ? m_length : count - i;
				auto tmp_itr = itr;
				double cummulative_closing_price = 0;
				double weighted_count = 0;

				for (size_t j = 0; j < window_count; ++j)
				{
					cummulative_closing_price += ((window_count - j) * (*tmp_itr).second.m_close);
					weighted_count += (window_count - j);

					std::advance(tmp_itr, 1);
				}

				IndicatorPoint point;
				point.date = (*itr).first;
				point.value = cummulative_closing_price / weighted_count;

				result.push_back(point);

				std::advance(itr, 1);
			}
		}*/


		const AsyncCandleData& data = candle_data->GetData();
		std::vector<double> ispc_input;
		std::vector<double> ispc_output(count);
		ispc_input.reserve(count);

		for (auto& pair : data)
		{
			ispc_input.push_back(pair.second.m_close);
		}

		ispc::calculate_wma(ispc_input.data(), ispc_output.data(), count, m_length);

		auto itr = candle_data->GetData().begin();
		for (double sma : ispc_output)
		{
			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = sma;

			result.push_back(point);

			std::advance(itr, 1);
		}

		//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		//std::cout << "Took " << std::to_string(std::chrono::duration<double>(end - start).count()) << " sec" << std::endl;
	}

	return result;
}