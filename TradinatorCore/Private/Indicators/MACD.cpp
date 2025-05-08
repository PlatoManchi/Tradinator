#include "Indicators/MACD.h"

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"
#include "Indicators/EMA.h"
#include "Utils/StopWatch.h"

#ifdef _MACD_ISPC_
#include "indicator_helper_ispc.h"
#endif // _MACD_ISPC_


std::vector<std::vector<double>> MACD::Calculate()
{
	std::vector<std::vector<double>> result;
	if (m_period_1 == 0 || m_period_2 == 0 || m_signal_period == 0) return result;

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

		if (count == 0 || m_period_1 > count || m_period_2 > count || m_signal_period > count) return result;

		std::vector<double> period_1_ema(count, 0.0);
		std::vector<double> period_2_ema(count, 0.0);

		std::vector<double> macd(count, 0.0);
		std::vector<double> signal(count, 0.0);
		std::vector<double> histogram(count, 0.0);
		
#ifdef _MACD_ISPC_
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			ispc::calculate_macd(data.m_closes.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), m_period_1, m_period_2, m_signal_period, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			ispc::calculate_macd(data.m_highs.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), m_period_1, m_period_2, m_signal_period, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			ispc::calculate_macd(data.m_opens.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), m_period_1, m_period_2, m_signal_period, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			ispc::calculate_macd(data.m_lows.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), m_period_1, m_period_2, m_signal_period, count);
		}
#else
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			CalculateRaw(data.m_closes.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), m_period_1, m_period_2, m_signal_period, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			CalculateRaw(data.m_highs.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), m_period_1, m_period_2, m_signal_period, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			CalculateRaw(data.m_opens.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), m_period_1, m_period_2, m_signal_period, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			CalculateRaw(data.m_lows.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), m_period_1, m_period_2, m_signal_period, count);
		}
#endif // _MACD_ISPC_

		result.emplace_back(std::move(macd));
		result.emplace_back(std::move(signal));
		result.emplace_back(std::move(histogram));
	}

	return result;
}



void MACD::CalculateRaw(const double* input, double* ema_period_1_buffer, double* ema_period_2_buffer, double* macd_output, double* signal_output, double* histogram_output, uint64_t period_1, uint64_t period_2, uint64_t signal_period, uint64_t data_size)
{
	EMA ema;

	ema.CalculateRaw(input, ema_period_1_buffer, period_1, data_size);
	ema.CalculateRaw(input, ema_period_2_buffer, period_2, data_size);

	for (uint64_t i = 0; i < data_size; ++i)
	{
		macd_output[i] = ema_period_1_buffer[i] - ema_period_2_buffer[i];
	}

	ema.CalculateRaw(macd_output, signal_output, signal_period, data_size);

	for (uint64_t i = 0; i < data_size; ++i)
	{
		histogram_output[i] = macd_output[i] - signal_output[i];
	}
}