#include "Indicators/ATR.h"
#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"

#include "Utils/StopWatch.h"

#include "Indicators/SMA.h"

#ifdef _ATR_ISPC_
#include "indicator_helper_ispc.h"
#endif // _ATR_ISPC_

std::vector<std::vector<double>> ATR::Calculate()
{
	std::vector<std::vector<double>> result;
	if (m_length == 0) return result;

	std::shared_ptr<Security> security = m_security.lock();

	if (security)
	{
		const std::shared_ptr<const AsyncData<CandlesData>>& candles_data = security->GetCandlesData();
		bool is_ready = candles_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candles_data->IsDataReady();
		}

		StopWatch stop_watch(GetName());

		const CandlesData& data = candles_data->GetData();
		uint64_t count = data.m_dates.size();

		if (count == 0 || m_length > count) return result;

		std::vector<double> average_true_ranges(count, 0.0);

#ifdef _ATR_ISPC_
		ispc::calculate_atr(data.m_highs.data(), data.m_lows.data(), data.m_closes.data(), average_true_ranges.data(), m_length, count);
#else
		CalculateRaw(data.m_highs.data(), data.m_lows.data(), data.m_closes.data(), average_true_ranges.data(), m_length, count);
#endif // _ATR_ISPC_

		result.emplace_back(std::move(average_true_ranges));
	}

	return result;
}



void ATR::CalculateRaw(const double* highs, const double* lows, const double* closes, double* output, uint64_t window_size, uint64_t data_size)
{
	output[0] = highs[0] - lows[0];

	// for first window_size items there is no previous ATR,
	// so assign 0 to them and calculate average true range for that period
	double true_range_sum = output[0];
	for (uint64_t i = 1; i < window_size; ++i)
	{
		double diff_1 = highs[i] - lows[i];
		double diff_2 = highs[i] - closes[i - 1];
		double diff_3 = lows[i] - closes[i - 1];

		true_range_sum += std::max({ diff_1, diff_2, diff_3 });

		output[i] = true_range_sum / (i + 1);
	}

	for (uint64_t i = window_size; i < data_size; ++i)
	{
		double diff_1 = highs[i] - lows[i];
		double diff_2 = highs[i] - closes[i - 1];
		double diff_3 = lows[i] - closes[i - 1];

		double true_range = std::max({ diff_1, diff_2, diff_3 });

		output[i] = (output[i - 1] * (window_size - 1) + true_range) / window_size;
	}
}