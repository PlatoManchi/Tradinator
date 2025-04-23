#include "Indicators/SMA.h"

#include "Data/Counter.h"
#include "Data/AsyncData.h"

SMA::SMA(size_t length)
	: Indicator(length)
{

}

SMA::SMA(std::weak_ptr<Counter> counter, size_t length)
	: Indicator(counter, length)
{

}

std::vector<IndicatorPoint> SMA::Calculate()
{
	std::vector<IndicatorPoint> result;

	std::shared_ptr<Counter> counter = m_counter.lock();

	if (counter)
	{
		const std::shared_ptr<const AsyncData<AsyncCandleData>>& candle_data = counter->GetCandleData();
		bool is_ready = candle_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candle_data->IsDataReady();
		}

		size_t count = candle_data->GetData().size();

		if (count > m_length)
		{
			auto itr = candle_data->GetData().begin();
			for (int i = 0; i < count - m_length; ++i)
			{
				auto tmp_itr = itr;
				double cummulative_closing_price = 0;
				for (int i = 0; i < m_length; ++i)
				{
					cummulative_closing_price += (*tmp_itr).second.m_close;
					std::advance(tmp_itr, 1);
				}

				IndicatorPoint point;
				point.date = (*itr).first;
				point.value = cummulative_closing_price / m_length;

				result.push_back(point);

				std::advance(itr, 1);
			}
		}
	}

	return result;
}