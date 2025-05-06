#include "Indicators/WMA.h"

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"
#include "Utils/StopWatch.h"

#ifdef _WMA_ISPC_
#include "indicator_helper_ispc.h"
#endif // _WMA_ISPC_



std::vector<std::vector<double>> WMA::Calculate()
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
		size_t count = data.m_dates.size();

		if (count == 0) return result;

		std::vector<double> wma(count);

#ifdef _WMA_ISPC_
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			ispc::calculate_wma(data.m_closes.data(), wma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			ispc::calculate_wma(data.m_highs.data(), wma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			ispc::calculate_wma(data.m_opens.data(), wma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			ispc::calculate_wma(data.m_lows.data(), wma.data(), m_length, count);
		}
#else
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			CalculateRaw(data.m_closes.data(), wma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			CalculateRaw(data.m_highs.data(), wma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			CalculateRaw(data.m_opens.data(), wma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			CalculateRaw(data.m_lows.data(), wma.data(), m_length, count);
		}
#endif // _WMA_ISPC_

		result.emplace_back(std::move(wma));
	}

	return result;
}


void WMA::CalculateRaw(const double* input, double* output, int64_t window_size, int64_t data_size)
{
	output[0] = input[0];
	for (size_t i = 1; i < data_size; ++i)
	{
		size_t start = window_size > data_size ? 0 : (i < window_size ? 0 : i - window_size);

		double sum = 0.0f;
		size_t count = 0;
		for (size_t j = start; j <= i; ++j)
		{
			count = count + 1;
			sum += (count * input[j]);
		}

		size_t sum_count = (count * (count + 1)) / 2; // Sum of n numbers is N(N+1)/2
		output[i] = sum / sum_count;
	}
}