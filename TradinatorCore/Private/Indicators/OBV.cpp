#include "Indicators/OBV.h"

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"
#include "Utils/StopWatch.h"

#ifdef _OBV_ISPC_
#include "indicator_helper_ispc.h"
#endif // _SMA_ISPC_


bool OBV::operator==(const OBV& other) const
{
	return m_source == other.m_source &&
		IndicatorType() == other.IndicatorType();
}


std::vector<std::vector<double>> OBV::Calculate()
{
	std::vector<std::vector<double>> result;
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

		std::vector<double> obv(count, 0.0);

#ifdef _OBV_ISPC_
		ispc::calculate_obv(data.m_closes.data(), data.m_volumes.data(), obv.data(), count);
#else
		CalculateRaw(data.m_closes.data(), data.m_volumes.data(), obv.data(), count);
#endif // _OBV_ISPC_

		result.emplace_back(std::move(obv));
	}

	return result;
}


void OBV::CalculateRaw(const double* closes, const uint64_t* volumes, double* output, uint64_t data_size)
{
	double prev_close = closes[0];
	double prev_obv = (double)volumes[0];

	output[0] = prev_obv;
	for (uint64_t i = 1; i < data_size; ++i)
	{
		double factor = closes[i] > prev_close ? 1.0 : -1.0;
		prev_obv = prev_obv + factor * ((double)volumes[i]);
		output[i] = prev_obv;
	}
}