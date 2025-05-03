#include "Indicators/ATR.h"
#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"

#include "Indicators/SMA.h"

#ifdef _ATR_ISPC_
#include "indicator_helper_ispc.h"
#endif // _ATR_ISPC_

std::vector<std::vector<IndicatorPoint>> ATR::Calculate()
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

		//std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

		size_t count = candle_data->GetData().size();
		if (count == 0) return result;

		std::vector<double> true_ranges;
		std::vector<double> average_true_ranges(count);
		std::vector<IndicatorPoint> average_true_ranges_result;
		average_true_ranges_result.reserve(count);

#ifdef _ATR_ISPC_
		true_ranges = std::vector<double>(count);

		std::vector<double> highs;
		std::vector<double> lows;
		std::vector<double> closes;
		highs.reserve(count);
		lows.reserve(count);
		closes.reserve(count);

		const CandleDataMapType& candle_data_map = candle_data->GetData();

		for (const auto& itr : candle_data_map)
		{
			highs.push_back(itr.second.m_high);
			lows.push_back(itr.second.m_low);
			closes.push_back(itr.second.m_close);
		}
		
		ispc::calculate_atr(highs.data(), lows.data(), closes.data(), true_ranges.data(), average_true_ranges.data(), count, m_length);
		
		auto itr = candle_data->GetData().begin();
		for (double sma_value : average_true_ranges)
		{
			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = sma_value;

			average_true_ranges_result.emplace_back(std::move(point));

			std::advance(itr, 1);
		}
		
#else
		true_ranges.reserve(count);

		auto itr = candle_data->GetData().begin();
		auto end = candle_data->GetData().end();
		while (itr != end)
		{
			const Candle& curr_candle = (*itr).second;
			
			double diff_1 = curr_candle.m_high - curr_candle.m_low;
			double diff_2 = 0;
			double diff_3 = 0;

			std::advance(itr, 1);

			if (itr != end)
			{
				const Candle& prev_candle = (*itr).second;
				diff_2 = fabs(curr_candle.m_high - prev_candle.m_close);
				diff_3 = fabs(curr_candle.m_low - prev_candle.m_close);
			}

			double true_range = std::max({ diff_1, diff_2, diff_3 });
			true_ranges.push_back(true_range);
		}

		SMA::CalculateRaw(true_ranges.data(), average_true_ranges.data(), count, m_length);

		itr = candle_data->GetData().begin();
		for (double sma_value : average_true_ranges)
		{
			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = sma_value;

			average_true_ranges_result.emplace_back(std::move(point));

			std::advance(itr, 1);
		}
#endif // _ATR_ISPC_




		result.emplace_back(std::move(average_true_ranges_result));

		//std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
		//std::cout << "ATR Took " << std::to_string(std::chrono::duration<double>(end_time - start_time).count()) << " sec" << std::endl;
	}
	return result;
}