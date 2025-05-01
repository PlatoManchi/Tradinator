#include "Indicators/OBV.h"

#include <iostream>

#include "Data/Counter.h"
#include "Data/AsyncData.h"



std::vector<std::vector<IndicatorPoint>> OBV::Calculate()
{
	std::vector<std::vector<IndicatorPoint>> result;
	
	std::shared_ptr<Counter> counter = m_counter.lock();

	if (counter)
	{
		const std::shared_ptr<const AsyncData<CandleDataMapType>>& candle_data = counter->GetCandleData();
		bool is_ready = candle_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candle_data->IsDataReady();
		}

		//std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		size_t count = candle_data->GetData().size();
		if (count == 0) return result;

		
		std::vector<IndicatorPoint> obv = std::vector<IndicatorPoint>(count);

		auto itr = candle_data->GetData().end();
		itr = std::prev(itr, 1);

		// 0th element is same value as volume
		IndicatorPoint first_point;
		first_point.date = (*itr).first;
		first_point.value = (*itr).second.m_volume;

		obv[count - 1] = first_point;

		double prev_obv = (*itr).second.m_volume;

		for (int64_t i = count - 2; i >= 0; --i)
		{
			double prev_closing = (*itr).second.m_close;
			
			itr = std::prev(itr, 1);
			double curr_closing = (*itr).second.m_close;
			size_t curr_volume = (*itr).second.m_volume;

			double factor = curr_closing > prev_closing ? 1.0 : -1.0;

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = prev_obv + factor * ((double)curr_volume);

			prev_obv = point.value;

			obv[i] = point;
		}

		result.emplace_back(std::move(obv));

		//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		//std::cout << "OBV Took " << std::to_string(std::chrono::duration<double>(end - start).count()) << " sec" << std::endl;
	}

	return result;
}