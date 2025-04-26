#include "Indicators/SMA.h"

#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"

#include "indicator_helper_ispc.h"

#if 1
#define _SMA_ISPC_
#else
#endif


SMA::SMA(size_t length)
	: Indicator(length)
{

}

SMA::SMA(std::weak_ptr<Counter> counter, size_t length)
	: Indicator(counter, length)
{

}

std::vector<IndicatorPoint> SMA::Calculate()
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

#ifdef _SMA_ISPC_
		const AsyncCandleData& data = candle_data->GetData();
		std::vector<double> ispc_input;
		std::vector<double> ispc_output(count);
		ispc_input.reserve(count);

		for (auto& pair : data)
		{
			ispc_input.emplace_back(pair.second.m_close);
		}

		ispc::calculate_sma(ispc_input.data(), ispc_output.data(), count, m_length);

		auto itr = candle_data->GetData().begin();
		for (double sma : ispc_output)
		{
			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = sma;

			result.emplace_back(std::move(point));

			std::advance(itr, 1);
		}
#else
		auto itr = candle_data->GetData().begin();
		for (size_t i = 0; i < count; ++i)
		{
			size_t window_count = i + m_length < count ? m_length : count - i;
			auto tmp_itr = itr;
			double cummulative_closing_price = 0;
			for (size_t j = 0; j < window_count; ++j)
			{
				cummulative_closing_price += (*tmp_itr).second.m_close;
				std::advance(tmp_itr, 1);
			}

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = cummulative_closing_price / window_count;

			result.emplace_back(std::move(point));

			std::advance(itr, 1);
		}
#endif // _SMA_ISPC_

		

		//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		//std::cout << "SMA Took " << std::to_string(std::chrono::duration<double>(end - start).count()) << " sec" << std::endl;
	}

	return result;
}