#include "Indicators/BollingerBand.h"

#include <iostream>
#include <cmath>

#include "Data/Security.h"
#include "Data/AsyncData.h"
#include "Indicators/SMA.h"
#include "Utils/StopWatch.h"

#ifdef _BOLLINGER_BAND_ISPC_
#include "indicator_helper_ispc.h"
#endif // _BOLLINGER_BAND_ISPC_



std::vector<std::vector<double>> BollingerBand::Calculate()
{
	std::vector<std::vector<double>> result;
	result.reserve(3);

	std::vector<double> top;
	std::vector<double> sma;
	std::vector<double> bottom;

	if (m_length == 0 || m_standard_deviation_multiplier == 0.0)
	{
		result.emplace_back(std::move(top));
		result.emplace_back(std::move(sma));
		result.emplace_back(std::move(bottom));

		return result;
	}


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

		if (count == 0)
		{
			result.emplace_back(std::move(top));
			result.emplace_back(std::move(sma));
			result.emplace_back(std::move(bottom));

			return result;
		}

		top = std::vector<double>(count, 0.0);
		sma = std::vector<double>(count, 0.0);
		bottom = std::vector<double>(count, 0.0);

#ifdef _BOLLINGER_BAND_ISPC_
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			ispc::calculate_bollinger_band(data.m_closes.data(), top.data(), sma.data(), bottom.data(), m_length, m_standard_deviation_multiplier, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			ispc::calculate_bollinger_band(data.m_highs.data(), top.data(), sma.data(), bottom.data(), m_length, m_standard_deviation_multiplier, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			ispc::calculate_bollinger_band(data.m_opens.data(), top.data(), sma.data(), bottom.data(), m_length, m_standard_deviation_multiplier, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			ispc::calculate_bollinger_band(data.m_lows.data(), top.data(), sma.data(), bottom.data(), m_length, m_standard_deviation_multiplier, count);
		}
#else
		// standard deviation calculation
		// https://en.wikipedia.org/wiki/Standard_deviation

		if (m_source == EIndicatorSource::E_CLOSE)
		{
			CalculateRaw(data.m_closes.data(), top.data(), sma.data(), bottom.data(), m_length, m_standard_deviation_multiplier, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			CalculateRaw(data.m_highs.data(), top.data(), sma.data(), bottom.data(), m_length, m_standard_deviation_multiplier, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			CalculateRaw(data.m_opens.data(), top.data(), sma.data(), bottom.data(), m_length, m_standard_deviation_multiplier, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			CalculateRaw(data.m_lows.data(), top.data(), sma.data(), bottom.data(), m_length, m_standard_deviation_multiplier, count);
		}
#endif

		result.emplace_back(std::move(top));
		result.emplace_back(std::move(sma));
		result.emplace_back(std::move(bottom));
	}

	return result;
}


void BollingerBand::CalculateRaw(const double* input, double* top, double* sma, double* bottom, uint64_t window_size, double standard_deviation_multiplier, uint64_t data_size)
{
	SMA sma_indicator;
	sma_indicator.CalculateRaw(input, sma, window_size, data_size);

	top[0] = input[0];
	sma[0] = input[0];
	bottom[0] = input[0];

	for (uint64_t i = 1; i < data_size; ++i)
	{
		uint64_t start = window_size > data_size ? 0 : (i + 1 < window_size ? 0 : i + 1 - window_size);

		double mean = sma[i];
		double cumulative_deviation_squared = 0;

		for (uint64_t j = start; j <= i; ++j)
		{
			double deviation = input[j] - mean;
			cumulative_deviation_squared += (deviation * deviation);
		}

		double variance = cumulative_deviation_squared / (i + 1 - start);
		double standard_deviation = sqrt(variance);

		top[i] = mean + standard_deviation_multiplier * standard_deviation;
		bottom[i] = mean - standard_deviation_multiplier * standard_deviation;
	}
}