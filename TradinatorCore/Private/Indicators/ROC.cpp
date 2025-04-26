#include "Indicators/ROC.h"

#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"

#include "indicator_helper_ispc.h"

#if 0
#define _ROC_ISPC_
#else
#endif

std::vector<IndicatorPoint> ROC::Calculate()
{
	std::vector<IndicatorPoint> result;
	if (m_length == 0) return result;

	std::shared_ptr<Counter> counter = m_counter.lock();

	if (counter)
	{
		const std::shared_ptr<const AsyncData<AsyncCandleData>>& candle_data = counter->GetCandleData();
		bool is_ready = candle_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candle_data->IsDataReady();
		}

		//std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		size_t count = candle_data->GetData().size();
		result.reserve(count);

#ifdef _ROC_ISPC_
#else

#endif // _ROC_ISPC_


		//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		//std::cout << "ROC Took " << std::to_string(std::chrono::duration<double>(end - start).count()) << " sec" << std::endl;
	}

	return result;
}