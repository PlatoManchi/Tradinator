#include "Strategy/Strategy.h"

#include "Data/Candle.h"
#include "Indicators/EMA.h"
#include "Indicators/SMA.h"
#include "Indicators/RSI.h"
#include "Indicators/MACD.h"
#include "Utils/Utils.h"

#ifdef _INDICATORS_ISPC_
#include "indicator_helper_ispc.h"
#endif // _RSI_ISPC_




std::string Strategy::Name()
{
	return TradinatorCoreSpace::Utils::GetStrategyTypeStr(GetStrategyType());
}



std::vector<bool> Long_Strategy_1::Check(const CandlesData& candles_data)
{
	size_t count = candles_data.m_dates.size();
	std::vector<bool> result(count, false);

	if (count == 0)
	{
		return result;
	}

	std::vector<double> ema_10(count, 0.0);
	std::vector<double> sma_20(count, 0.0);
	std::vector<double> rsi(count, 0.0);

	// tmp space to calculate macd
	std::vector<double> period_1_ema(count, 0.0);
	std::vector<double> period_2_ema(count, 0.0);
	std::vector<double> macd(count, 0.0);
	std::vector<double> signal(count, 0.0);
	std::vector<double> histogram(count, 0.0);

#ifdef _INDICATORS_ISPC_
	ispc::calculate_ema(candles_data.m_closes.data(), ema_10.data(), 10, count);
	ispc::calculate_sma(candles_data.m_closes.data(), sma_20.data(), 20, count);
	ispc::calculate_rsi(candles_data.m_closes.data(), rsi.data(), 14, count);
	ispc::calculate_macd(candles_data.m_closes.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), 12, 26, 9, count);
#else
	EMA().CalculateRaw(candles_data.m_closes.data(), ema_10.data(), 10, count);
	SMA().CalculateRaw(candles_data.m_closes.data(), sma_20.data(), 20, count);
	RSI().CalculateRaw(candles_data.m_closes.data(), rsi.data(), 14, count);
	MACD().CalculateRaw(candles_data.m_closes.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), 12, 26, 9, count);
#endif // _INDICATORS_ISPC_

	// Amount of days that rules can happen.
	const size_t delta_length = 2;

	for (size_t i = delta_length; i < count; ++i)
	{
		size_t loop_end = i + delta_length >= count ? count : i + delta_length;

		size_t min_index = loop_end;
		size_t max_index = i - delta_length;

		// Rule 1: Short Term Moving Average crossing Long Term Moving Average from below to above.
		bool is_rule_1 = false;
		for (size_t j = i - delta_length; j < loop_end - 1; ++j)
		{
			if (ema_10[j] < sma_20[j]
				&& ema_10[j + 1] > sma_20[j + 1])
			{
				is_rule_1 = true;

				if (min_index > j)
				{
					min_index = j;
				}
				if (max_index < j)
				{
					max_index = j;
				}
			}
		}

		// Rule 2: RSI crossing 50 level from below to above.
		bool is_rule_2 = false;
		for (size_t j = i - delta_length; j < loop_end - 1; ++j)
		{
			if (rsi[j] < 50.0
				&& rsi[j + 1] > 50.0)
			{
				is_rule_2 = true;

				if (min_index > j)
				{
					min_index = j;
				}
				if (max_index < j)
				{
					max_index = j;
				}
			}
		}


		// Rule 3: MACD crossing signal line or zero line from below to above. 
		// MACD moving above zero is better than moving above signal line.
		bool is_rule_3 = false;
		for (size_t j = i - delta_length; j < loop_end - 1; ++j)
		{
			if ((macd[j] < signal[j] && macd[j + 1] > signal[j + 1])
				|| (macd[j] < 0.0 && macd[j + 1] > 0.0))
			{
				is_rule_3 = true;

				if (min_index > j)
				{
					min_index = j;
				}
				if (max_index < j)
				{
					max_index = j;
				}
			}
		}

		if (is_rule_1 && is_rule_2 && is_rule_3)
		{
			for (size_t j = min_index; j <= max_index; ++j)
			{
				result[j] = true;
			}

			if (i < max_index)
			{
				i = max_index > delta_length ? max_index : delta_length;
			}
		}
	}
	

	return result;
}

std::vector<std::unique_ptr<Indicator>> Long_Strategy_1::GetIndicatorsNeeded() const
{
	std::vector<std::unique_ptr<Indicator>> result;
	result.push_back(std::make_unique<EMA>(10));
	result.push_back(std::make_unique<SMA>(20));
	result.push_back(std::make_unique<RSI>(14));
	result.push_back(std::make_unique<MACD>(12, 26, 9));

	return result;
}













std::vector<bool> Short_Strategy_1::Check(const CandlesData& candles_data)
{
	size_t count = candles_data.m_dates.size();
	std::vector<bool> result(count, false);

	if (count == 0)
	{
		return result;
	}

	std::vector<double> ema_10(count, 0.0);
	std::vector<double> sma_20(count, 0.0);
	std::vector<double> rsi(count, 0.0);

	// tmp space to calculate macd
	std::vector<double> period_1_ema(count, 0.0);
	std::vector<double> period_2_ema(count, 0.0);
	std::vector<double> macd(count, 0.0);
	std::vector<double> signal(count, 0.0);
	std::vector<double> histogram(count, 0.0);

#ifdef _INDICATORS_ISPC_
	ispc::calculate_ema(candles_data.m_closes.data(), ema_10.data(), 10, count);
	ispc::calculate_sma(candles_data.m_closes.data(), sma_20.data(), 20, count);
	ispc::calculate_rsi(candles_data.m_closes.data(), rsi.data(), 14, count);
	ispc::calculate_macd(candles_data.m_closes.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), 12, 26, 9, count);
#else
	EMA().CalculateRaw(candles_data.m_closes.data(), ema_10.data(), 10, count);
	SMA().CalculateRaw(candles_data.m_closes.data(), sma_20.data(), 20, count);
	RSI().CalculateRaw(candles_data.m_closes.data(), rsi.data(), 14, count);
	MACD().CalculateRaw(candles_data.m_closes.data(), period_1_ema.data(), period_2_ema.data(), macd.data(), signal.data(), histogram.data(), 12, 26, 9, count);
#endif // _INDICATORS_ISPC_

	// Amount of days that rules can happen.
	const size_t delta_length = 2;
	for (size_t i = delta_length; i < count; ++i)
	{
		size_t loop_end = i + delta_length >= count ? count : i + delta_length;

		size_t min_index = loop_end;
		size_t max_index = i - delta_length;

		// Rule 1: Short Term Moving Average crossing Long Term Moving Average from above to below.
		bool is_rule_1 = false;
		for (size_t j = i - delta_length; j < loop_end - 1; ++j)
		{
			if (ema_10[j] > sma_20[j]
				&& ema_10[j + 1] < sma_20[j + 1])
			{
				is_rule_1 = true;

				if (min_index > j)
				{
					min_index = j;
				}
				if (max_index < j)
				{
					max_index = j;
				}
			}
		}

		// Rule 2: RSI crossing 50 level from above to below.
		bool is_rule_2 = false;
		for (size_t j = i - delta_length; j < loop_end - 1; ++j)
		{
			if (rsi[j] > 50.0
				&& rsi[j + 1] < 50.0)
			{
				is_rule_2 = true;

				if (min_index > j)
				{
					min_index = j;
				}
				if (max_index < j)
				{
					max_index = j;
				}
			}
		}


		// Rule 3: MACD crossing signal line or zero line from above to below. 
		// MACD moving below zero is better than moving below signal line.
		bool is_rule_3 = false;
		for (size_t j = i - delta_length; j < loop_end - 1; ++j)
		{
			if ((macd[j] > signal[j] && macd[j + 1] < signal[j + 1])
				|| (macd[j] > 0.0 && macd[j + 1] < 0.0))
			{
				is_rule_3 = true;

				if (min_index > j)
				{
					min_index = j;
				}
				if (max_index < j)
				{
					max_index = j;
				}
			}
		}

		if (is_rule_1 && is_rule_2 && is_rule_3)
		{
			for (size_t j = min_index; j <= max_index; ++j)
			{
				result[j] = true;
			}

			if (i < max_index)
			{
				i = max_index > delta_length ? max_index : delta_length;
			}
		}
	}


	return result;
}


std::vector<std::unique_ptr<Indicator>> Short_Strategy_1::GetIndicatorsNeeded() const
{
	std::vector<std::unique_ptr<Indicator>> result;
	result.push_back(std::make_unique<EMA>(10));
	result.push_back(std::make_unique<SMA>(20));
	result.push_back(std::make_unique<RSI>(14));
	result.push_back(std::make_unique<MACD>(12, 26, 9));

	return result;
}