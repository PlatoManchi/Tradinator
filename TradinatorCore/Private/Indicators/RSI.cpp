#include "Indicators/RSI.h"

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"
#include "Utils/StopWatch.h"

#ifdef _RSI_ISPC_
#include "indicator_helper_ispc.h"
#endif // _RSI_ISPC_



std::vector<std::vector<double>> RSI::Calculate()
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

		std::vector<double> rsi(count);


#ifdef _RSI_ISPC_
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			ispc::calculate_rsi(data.m_closes.data(), rsi.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			ispc::calculate_rsi(data.m_highs.data(), rsi.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			ispc::calculate_rsi(data.m_opens.data(), rsi.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			ispc::calculate_rsi(data.m_lows.data(), rsi.data(), m_length, count);
		}
#else
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			CalculateRaw(data.m_closes.data(), rsi.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			CalculateRaw(data.m_highs.data(), rsi.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			CalculateRaw(data.m_opens.data(), rsi.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			CalculateRaw(data.m_lows.data(), rsi.data(), m_length, count);
		}
#endif // _RSI_ISPC_

		result.emplace_back(std::move(rsi));
	}

	return result;
}

void RSI::CalculateRaw(const double* input, double* output, size_t window_size, size_t data_size)
{
	output[0] = 0;

	for (size_t i = 1; i < data_size; ++i)
	{
		size_t start = window_size > data_size ? 0 : (i < window_size ? 0 : i - window_size);

		double cumulative_gain = 0;
		double cumulative_loss = 0;

		for (size_t j = start; j <= i; ++j)
		{
			double diff = input[j] - input[j - 1];

			cumulative_gain += (diff > 0.0 ? diff : 0.0);
			cumulative_loss += (diff < 0.0 ? diff : 0.0);
		}

		double relative_strength = cumulative_gain / abs(cumulative_loss);
		output[i] = 100.0 - 100.0 / (1 + relative_strength);
	}
}