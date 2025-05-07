#include "Indicators/ROC.h"

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"
#include "Utils/StopWatch.h"

#ifdef _ROC_ISPC_
#include "indicator_helper_ispc.h"
#endif // _ROC_ISPC_



std::vector<std::vector<double>> ROC::Calculate()
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

		std::vector<double> roc(count, 0.0);

#ifdef _ROC_ISPC_
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			ispc::calculate_roc(data.m_closes.data(), roc.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			ispc::calculate_roc(data.m_highs.data(), roc.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			ispc::calculate_roc(data.m_opens.data(), roc.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			ispc::calculate_roc(data.m_lows.data(), roc.data(), m_length, count);
		}
#else
		if (m_source == EIndicatorSource::E_CLOSE)
		{
			CalculateRaw(data.m_closes.data(), roc.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_HIGH)
		{
			CalculateRaw(data.m_highs.data(), roc.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_OPEN)
		{
			CalculateRaw(data.m_opens.data(), roc.data(), m_length, count);
		}
		else if (m_source == EIndicatorSource::E_LOW)
		{
			CalculateRaw(data.m_lows.data(), roc.data(), m_length, count);
		}
#endif // _ROC_ISPC_

		result.emplace_back(std::move(roc));
	}

	return result;
}


void ROC::CalculateRaw(const double* input, double* output, uint64_t window_size, uint64_t data_size)
{
	for (uint64_t i = 0; i < data_size; ++i)
	{
		uint64_t compare_with_index = window_size > data_size ? 0 : (i < window_size ? 0 : i - window_size);

		double current = input[i];
		double compare_with = input[compare_with_index];
		if (compare_with != 0)
		{
			output[i] = ((current - compare_with) / compare_with) * 100.0;
		}
		else
		{
			output[i] = 0.0;
		}
	}
}