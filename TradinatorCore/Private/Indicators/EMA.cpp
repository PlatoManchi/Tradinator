#include "Indicators/EMA.h"

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"
#include "Utils/StopWatch.h"

#ifdef _EMA_ISPC_
#include "indicator_helper_ispc.h"
#endif // _EMA_ISPC_



std::vector<std::vector<double>> EMA::Calculate()
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

		if (count == 0) return result;

		std::vector<double> ema(count, 0.0);

#ifdef _EMA_ISPC_
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			ispc::calculate_ema(data.m_closes.data(), ema.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			ispc::calculate_ema(data.m_highs.data(), ema.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			ispc::calculate_ema(data.m_opens.data(), ema.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			ispc::calculate_ema(data.m_lows.data(), ema.data(), m_length, count);
		}
#else
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			CalculateRaw(data.m_closes.data(), ema.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			CalculateRaw(data.m_highs.data(), ema.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			CalculateRaw(data.m_opens.data(), ema.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			CalculateRaw(data.m_lows.data(), ema.data(), m_length, count);
		}
#endif // _EMA_ISPC_
		
		result.emplace_back(std::move(ema));
	}

	return result;
}

void EMA::CalculateRaw(const double* input, double* output, uint64_t window_size, uint64_t data_size)
{
	const double factor = 2.0 / ((double)window_size + 1.0);

	double prev_ema = input[0];
	output[0] = prev_ema;

	for (uint64_t i = 1; i < data_size; ++i)
	{
		prev_ema = input[i] * factor + (1.0 - factor) * prev_ema;
		output[i] = prev_ema;
	}
}