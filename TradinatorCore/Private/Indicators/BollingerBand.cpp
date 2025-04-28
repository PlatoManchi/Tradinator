#include "Indicators/BollingerBand.h"

#include <iostream>
#include <cmath>

#include "Data/Counter.h"
#include "Data/AsyncData.h"
#include "Indicators/SMA.h"

#ifdef _BOLLINGER_BAND_ISPC_
#include "indicator_helper_ispc.h"
#endif // _BOLLINGER_BAND_ISPC_



std::vector<std::vector<IndicatorPoint>> BollingerBand::Calculate()
{
	std::vector<std::vector<IndicatorPoint>> result;
	result.reserve(3);

	std::vector<IndicatorPoint> top;
	std::vector<IndicatorPoint> sma;
	std::vector<IndicatorPoint> bottom;

	if (m_length == 0 || m_standard_deviation_multiplier == 0.0)
	{
		result.emplace_back(std::move(top));
		result.emplace_back(std::move(sma));
		result.emplace_back(std::move(bottom));

		return result;
	}


	std::shared_ptr<Counter> counter = m_counter.lock();

	if (counter)
	{
		const std::shared_ptr<const AsyncData<AsyncCandleData>>& candle_data = counter->GetCandleData();
		bool is_ready = candle_data->IsDataReady();

		// Wait till candle data is ready
		while (!is_ready)
		{
			is_ready = candle_data->IsDataReady();
		}

		//std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		size_t count = candle_data->GetData().size();
		if (count == 0)
		{
			result.emplace_back(std::move(top));
			result.emplace_back(std::move(sma));
			result.emplace_back(std::move(bottom));

			return result;
		}

		top.reserve(count);
		bottom.reserve(count);

#ifdef _BOLLINGER_BAND_ISPC_
		const AsyncCandleData& data = candle_data->GetData();
		std::vector<double> ispc_input;
		std::vector<double> ispc_output_top(count);
		std::vector<double> ispc_output_sma(count);
		std::vector<double> ispc_output_bottom(count);

		ispc_input.reserve(count);

		for (auto& pair : data)
		{
			ispc_input.emplace_back(pair.second.m_close);
		}

		ispc::calculate_bollinger_band(ispc_input.data(), ispc_output_top.data(), ispc_output_sma.data(), ispc_output_bottom.data(), count, m_length, m_standard_deviation_multiplier);

		auto itr = candle_data->GetData().begin();
		size_t index = 0;
		for (auto& pair : data)
		{
			IndicatorPoint top_point;
			top_point.date = pair.first;
			top_point.value = ispc_output_top[index];
			top.emplace_back(std::move(top_point));

			IndicatorPoint sma_point;
			sma_point.date = pair.first;
			sma_point.value = ispc_output_sma[index];
			sma.emplace_back(std::move(sma_point));

			IndicatorPoint bottom_point;
			bottom_point.date = pair.first;
			bottom_point.value = ispc_output_bottom[index];
			bottom.emplace_back(std::move(bottom_point));

			++index;
		}
#else
		// standard deviation calculation
		// https://en.wikipedia.org/wiki/Standard_deviation

		// Get the sma
		SMA sma_indicator(m_counter, m_length);
		sma = std::move(sma_indicator.Calculate());

		auto itr = candle_data->GetData().begin();
		for (size_t i = 0; i < count; ++i)
		{
			size_t window_size = i + m_length < count ? m_length : count - i;
			auto tmp_itr = itr;

			double cumulative_deviation_squared = 0;
			double mean = sma[i].value;

			for (size_t j = 0; j < window_size; ++j)
			{
				double deviation = (*tmp_itr).second.m_close - mean;
				cumulative_deviation_squared += (deviation * deviation);

				std::advance(tmp_itr, 1);
			}

			double variance = cumulative_deviation_squared / window_size;
			double standard_deviation = sqrt(variance);

			IndicatorPoint top_point;
			top_point.date = (*itr).first;
			top_point.value = mean + m_standard_deviation_multiplier * standard_deviation;

			top.emplace_back(std::move(top_point));

			IndicatorPoint bottom_point;
			bottom_point.date = (*itr).first;
			bottom_point.value = mean - m_standard_deviation_multiplier * standard_deviation;

			bottom.emplace_back(std::move(bottom_point));

			std::advance(itr, 1);
		}
#endif

		//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		//std::cout << "Bollinger Band Took " << std::to_string(std::chrono::duration<double>(end - start).count()) << " sec" << std::endl;
	}


	result.emplace_back(std::move(top));
	result.emplace_back(std::move(sma));
	result.emplace_back(std::move(bottom));

	return result;
}
