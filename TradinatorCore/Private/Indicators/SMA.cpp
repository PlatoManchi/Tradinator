#include "Indicators/SMA.h"

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"

#include "Utils/StopWatch.h"

#ifdef _SMA_ISPC_
#include "indicator_helper_ispc.h"
#endif // _SMA_ISPC_



bool SMA::operator==(const SMA& other) const
{
	return Indicator::operator==(other);
}



std::vector<std::vector<double>> SMA::Calculate()
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

		std::vector<double> sma(count, 0.0);
		
		
#ifdef _SMA_ISPC_
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			ispc::calculate_sma(data.m_closes.data(), sma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			ispc::calculate_sma(data.m_highs.data(), sma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			ispc::calculate_sma(data.m_opens.data(), sma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			ispc::calculate_sma(data.m_lows.data(), sma.data(), m_length, count);
		}
#else
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			CalculateRaw(data.m_closes.data(), sma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			CalculateRaw(data.m_highs.data(), sma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			CalculateRaw(data.m_opens.data(), sma.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			CalculateRaw(data.m_lows.data(), sma.data(), m_length, count);
		}
#endif // _SMA_ISPC_

		result.emplace_back(std::move(sma));
	}

	return result;
}


void SMA::CalculateRaw(const double* input, double* output, uint64_t window_size, uint64_t data_size)
{
	output[0] = input[0];
	for (uint64_t i = 1; i < data_size; ++i)
	{
		uint64_t start = window_size > data_size ? 0 : (i + 1 < window_size ? 0 : i + 1 - window_size);

		double sum = 0.0f;
		for (uint64_t j = start; j <= i; ++j)
		{
			sum += input[j];
		}

		output[i] = sum / (i + 1 - start);
	}
}