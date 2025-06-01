#include "Indicators/RSI.h"

#include <iostream>
#include <numeric>

#include "Data/Security.h"
#include "Data/AsyncData.h"
#include "Utils/StopWatch.h"

#ifdef _RSI_ISPC_
#include "indicator_helper_ispc.h"
#endif // _RSI_ISPC_


bool RSI::operator==(const RSI& other) const
{
	return Indicator::operator==(other);
}



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
		uint64_t count = data.m_dates.size();

		if (count == 0 || m_length > count) return result;

		std::vector<double> rsi(count, 0.0);


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

void RSI::CalculateRaw(const double* input, double* output, uint64_t window_size, uint64_t data_size)
{
	output[0] = 0;

	double prev_gain_avg = 0;
	double prev_loss_avg = 0;
	for (uint64_t i = 1; i < window_size + 1; ++i)
	{
		double diff = input[i] - input[i - 1];
		prev_gain_avg += (diff > 0.0 ? diff : 0.0);
		prev_loss_avg -= (diff < 0.0 ? diff : 0.0);

		output[i] = 0;
	}

	prev_gain_avg = prev_gain_avg / window_size;
	prev_loss_avg = prev_loss_avg / window_size;

	for (uint64_t i = window_size + 1; i < data_size; ++i)
	{
		double diff = input[i] - input[i - 1];

		double gain = (diff > 0.0 ? diff : 0.0);
		double loss = -(diff < 0.0 ? diff : 0.0);

		prev_gain_avg = (prev_gain_avg * (window_size - 1) + gain) / window_size;
		prev_loss_avg = (prev_loss_avg * (window_size - 1) + loss) / window_size;

		if (prev_loss_avg < 0.0000000001) // e-10 is enough to be considered as zero
		{
			output[i] = 100.0;
		}
		else
		{
			double relative_strength = prev_gain_avg / prev_loss_avg;
			output[i] = 100.0 - 100.0 / (1 + relative_strength);
		}
	}
}