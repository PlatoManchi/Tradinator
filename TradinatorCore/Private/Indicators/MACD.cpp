#include "Indicators/MACD.h"

#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"
#include "Indicators/EMA.h"

#ifdef _MACD_ISPC_
#include "indicator_helper_ispc.h"
#endif // _MACD_ISPC_


std::vector<std::vector<IndicatorPoint>> MACD::Calculate()
{
	std::vector<std::vector<IndicatorPoint>> result;
	if (m_period_1 == 0 || m_period_2 == 0 || m_signal_period == 0) return result;

	std::shared_ptr<Counter> counter = m_counter.lock();

	if (counter)
	{
		const std::shared_ptr<const AsyncData<AsyncCandleData>>& candle_data = counter->GetCandleData();
		bool is_ready = candle_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candle_data->IsDataReady();
		}

		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		size_t count = candle_data->GetData().size();
		if (count == 0) return result;

		std::vector<IndicatorPoint> macd;
		macd.reserve(count);
		std::vector<double> macd_raw;
		

		std::vector<IndicatorPoint> signal;
		signal.reserve(count);
		std::vector<double> signal_raw(count);
		

		std::vector<IndicatorPoint> histogram;
		histogram.reserve(count);

#ifdef _MACD_ISPC_
		macd_raw = std::vector<double>(count);

		const AsyncCandleData& data = candle_data->GetData();
		std::vector<double> ispc_input;
		ispc_input.reserve(count);

		// ISPC doesn't like reading from back to front because it will become non-contiguous memory. 
		// So reverse the array here and unreverse the output
		auto reverse_itr = candle_data->GetData().end();
		for (size_t i = 0; i < count; ++i)
		{
			reverse_itr = std::prev(reverse_itr, 1);
			ispc_input.emplace_back((*reverse_itr).second.m_close);
		}

		std::vector<double> period_1_ema(count);
		std::vector<double> period_2_ema(count);
		std::vector<double> histogram_raw(count);

		ispc::calculate_mcda(ispc_input.data(), period_1_ema.data(), period_2_ema.data(), 
			macd_raw.data(), signal_raw.data(), histogram_raw.data(), 
			m_period_1, m_period_2, m_signal_period, count);

		auto itr = candle_data->GetData().begin();
		for (int64_t i = count - 1; i >= 0; --i)
		{
			IndicatorPoint macd_point;
			macd_point.date = (*itr).first;
			macd_point.value = macd_raw[i];

			IndicatorPoint signal_point;
			signal_point.date = (*itr).first;
			signal_point.value = signal_raw[i];

			IndicatorPoint histogram_point;
			histogram_point.date = (*itr).first;
			histogram_point.value = histogram_raw[i];

			macd.emplace_back(std::move(macd_point));
			signal.emplace_back(std::move(signal_point));
			histogram.emplace_back(std::move(histogram_point));

			std::advance(itr, 1);
		}

#else
		macd_raw.reserve(count);

		const AsyncCandleData& data = candle_data->GetData();
		std::vector<double> price_data;
		price_data.reserve(count);

		for (auto& pair : data)
		{
			price_data.emplace_back(pair.second.m_close);
		}

		std::vector<double> period_1_ema(count);
		std::vector<double> period_2_ema(count);

		EMA::CalculateRaw(price_data.data(), period_1_ema.data(), count, m_period_1);
		EMA::CalculateRaw(price_data.data(), period_2_ema.data(), count, m_period_2);

		auto itr = candle_data->GetData().begin();
		for (int i = 0; i < count; ++i)
		{
			double macd_value = period_1_ema[i] - period_2_ema[i];

			macd_raw.push_back(macd_value);

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = macd_value;

			macd.emplace_back(std::move(point));
			std::advance(itr, 1);
		}

		EMA::CalculateRaw(macd_raw.data(), signal_raw.data(), count, m_signal_period);

		itr = candle_data->GetData().begin();
		for (int i = 0; i < count; ++i)
		{
			// signal
			IndicatorPoint signal_point;
			signal_point.date = (*itr).first;
			signal_point.value = signal_raw[i];

			signal.emplace_back(std::move(signal_point));

			// histogram
			IndicatorPoint histogram_point;
			histogram_point.date = (*itr).first;
			histogram_point.value = macd_raw[i] - signal_raw[i];

			histogram.emplace_back(std::move(histogram_point));


			std::advance(itr, 1);
		}

#endif // _MACD_ISPC_

		result.emplace_back(std::move(macd));
		result.emplace_back(std::move(signal));
		result.emplace_back(std::move(histogram));

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "MACD Took " << std::to_string(std::chrono::duration<double>(end - start).count()) << " sec" << std::endl;
	}

	return result;
}
