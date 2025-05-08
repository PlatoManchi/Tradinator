#include "Data/Candle.h"

#include <iostream>
#include <cmath>

#define _DOJI_BODY_RATIO_ 0.02 // 2%

#define _HAMMER_BODY_RATIO_ 0.2 // 20%
#define _HAMMER_SHADOW_RATIO_ 0.1 // 10%

#define _DRAGON_FLY_DOJI_SHADOW_RATIO_ 0.1 // 10% upper_shadow/lower_shadow ratio
#define _GRAVE_STONE_DOJI_SHADOW_RATIO_ 0.1 // 10% lower_shadow/upper_shadow ratio

#define _LONG_LEG_DOJI_BODY_TO_SHADO_RATIO 10 // Both shadows should be 10 times or more than body
#define _LONG_LEG_DOJI_SHADOW_RATIO 0.1 // 10% difference between upper and lower shadows

bool Candle::IsDoji() const
{
	double body_size = fabs(m_close - m_open);
	double total_range = m_high - m_low;

	if (total_range < DBL_EPSILON)
	{
		return false;
	}
	
	double body_ratio = body_size / total_range;

	return body_ratio < _DOJI_BODY_RATIO_;
}


bool Candle::IsMarubozu() const
{
	bool is_indian_market = true;
	if (is_indian_market)
	{
		// In Indian market, closing price is not the last transaction. It is avg of last half hour
		// so green marubozu will have a small upper shadow and for red marubozu will have a small lower shadow.
		bool is_green = m_open < m_close;

		// How much shadow is acceptable
		// 5% shadow is acceptable? Magic Number, adjust for better results
		const double shadow_threshold = fabs(m_open < m_close) * 0.05; 

		if (is_green)
		{
			// Green marubozu will have small upper shadow
			return (m_high - m_close < shadow_threshold) && (fabs(m_open - m_low) <= LDBL_EPSILON);
		}
		else
		{
			// Red marubozu will have small lower shadow
			return (m_close - m_low < shadow_threshold) && (fabs(m_open - m_high) <= LDBL_EPSILON);
		}
	}
	else
	{
		return (fabs(m_open - m_high) <= LDBL_EPSILON && fabs(m_close - m_low) <= LDBL_EPSILON) || // condition for red candle case
			(fabs(m_open - m_low) <= LDBL_EPSILON && fabs(m_close - m_high) <= LDBL_EPSILON); // condition for green candle case
	}

	return false;
}


bool Candle::IsHammer() const
{
	if (IsDoji())
	{
		return false;
	}
	
	double body_size = fabs(m_close - m_open);
	double total_range = m_high - m_low;
	double upper_shadow = m_high - m_open;
	double lower_shadow = m_close - m_low;

	if (total_range < DBL_EPSILON || lower_shadow < DBL_EPSILON)
	{
		return false;
	}

	double body_ratio = body_size / total_range;
	double upper_to_lower_ratio = upper_shadow / lower_shadow;

	return (upper_to_lower_ratio < _HAMMER_SHADOW_RATIO_) && (body_ratio < _HAMMER_BODY_RATIO_);
}


bool Candle::IsInvertedHammer() const
{
	if (IsDoji())
	{
		return false;
	}

	double body_size = fabs(m_close - m_open);
	double total_range = m_high - m_low;
	double upper_shadow = m_high - m_open;
	double lower_shadow = m_close - m_low;

	if (total_range < DBL_EPSILON || upper_shadow < DBL_EPSILON)
	{
		return false;
	}

	double body_ratio = body_size / total_range;
	double lower_to_upper_ratio = lower_shadow / upper_shadow;

	return (lower_to_upper_ratio < _HAMMER_SHADOW_RATIO_) && (body_ratio < _HAMMER_BODY_RATIO_);
}


bool Candle::IsDragonflyDoji() const
{
	if (!IsDoji())
	{
		return false;
	}

	double upper_shadow = m_high - m_open;
	double lower_shadow = m_close - m_low;

	if (lower_shadow < DBL_EPSILON)
	{
		return false;
	}

	double upper_to_lower_ratio = upper_shadow / lower_shadow;

	return upper_to_lower_ratio < _DRAGON_FLY_DOJI_SHADOW_RATIO_;
}


bool Candle::IsGravestoneDoji() const
{
	if (!IsDoji())
	{
		return false;
	}

	double upper_shadow = m_high - m_open;
	double lower_shadow = m_close - m_low;

	if (upper_shadow < DBL_EPSILON)
	{
		return false;
	}

	double lower_to_upper_ratio = lower_shadow / upper_shadow;

	return lower_to_upper_ratio < _GRAVE_STONE_DOJI_SHADOW_RATIO_;
}


bool Candle::IsLongLegDoji() const
{
	if (!IsDoji())
	{
		return false;
	}

	double body_size = fabs(m_close - m_open);
	double total_range = m_high - m_low;
	double upper_shadow = m_high - m_open;
	double lower_shadow = m_close - m_low;

	if (lower_shadow < DBL_EPSILON)
	{
		return false;
	}

	double shadow_ratio = upper_shadow / lower_shadow;

	return upper_shadow > body_size * _LONG_LEG_DOJI_BODY_TO_SHADO_RATIO &&		// upper shadow should be long
		lower_shadow > body_size * _LONG_LEG_DOJI_BODY_TO_SHADO_RATIO &&		// lower shadow should be long
		shadow_ratio >= (1.0 - _LONG_LEG_DOJI_SHADOW_RATIO) &&					// shadows ratio should be in range
		shadow_ratio <= (1.0 + _LONG_LEG_DOJI_SHADOW_RATIO);
}




bool CandlesData::IsDoji(size_t idx) const
{
	double body_size = fabs(m_closes[idx] - m_opens[idx]);
	double total_range = m_highs[idx] - m_lows[idx];

	if (total_range < DBL_EPSILON)
	{
		return false;
	}

	double body_ratio = body_size / total_range;

	return body_ratio < _DOJI_BODY_RATIO_;
}

bool CandlesData::IsMarubozu(size_t idx) const
{
	bool is_indian_market = true;
	if (is_indian_market)
	{
		// In Indian market, closing price is not the last transaction. It is avg of last half hour
		// so green marubozu will have a small upper shadow and for red marubozu will have a small lower shadow.
		bool is_green = m_opens[idx] < m_closes[idx];

		// How much shadow is acceptable
		// 5% shadow is acceptable? Magic Number, adjust for better results
		const double shadow_threshold = fabs(m_opens[idx] < m_closes[idx]) * 0.05;

		if (is_green)
		{
			// Green marubozu will have small upper shadow
			return (m_highs[idx] - m_closes[idx] < shadow_threshold) && (fabs(m_opens[idx] - m_lows[idx]) <= LDBL_EPSILON);
		}
		else
		{
			// Red marubozu will have small lower shadow
			return (m_closes[idx] - m_lows[idx] < shadow_threshold) && (fabs(m_opens[idx] - m_highs[idx]) <= LDBL_EPSILON);
		}
	}
	else
	{
		return (fabs(m_opens[idx] - m_highs[idx]) <= LDBL_EPSILON && fabs(m_closes[idx] - m_lows[idx]) <= LDBL_EPSILON) || // condition for red candle case
			(fabs(m_opens[idx] - m_lows[idx]) <= LDBL_EPSILON && fabs(m_closes[idx] - m_highs[idx]) <= LDBL_EPSILON); // condition for green candle case
	}

	return false;
}

bool CandlesData::IsHammer(size_t idx) const
{
	if (IsDoji(idx))
	{
		return false;
	}

	double body_size = fabs(m_closes[idx] - m_opens[idx]);
	double total_range = m_highs[idx] - m_lows[idx];
	double upper_shadow = m_highs[idx] - m_opens[idx];
	double lower_shadow = m_closes[idx] - m_lows[idx];

	if (total_range < DBL_EPSILON || lower_shadow < DBL_EPSILON)
	{
		return false;
	}

	double body_ratio = body_size / total_range;
	double upper_to_lower_ratio = upper_shadow / lower_shadow;

	return (upper_to_lower_ratio < _HAMMER_SHADOW_RATIO_) && (body_ratio < _HAMMER_BODY_RATIO_);
}

bool CandlesData::IsInvertedHammer(size_t idx) const
{
	if (IsDoji(idx))
	{
		return false;
	}

	double body_size = fabs(m_closes[idx] - m_opens[idx]);
	double total_range = m_highs[idx] - m_lows[idx];
	double upper_shadow = m_highs[idx] - m_opens[idx];
	double lower_shadow = m_closes[idx] - m_lows[idx];

	if (total_range < DBL_EPSILON || upper_shadow < DBL_EPSILON)
	{
		return false;
	}

	double body_ratio = body_size / total_range;
	double lower_to_upper_ratio = lower_shadow / upper_shadow;

	return (lower_to_upper_ratio < _HAMMER_SHADOW_RATIO_) && (body_ratio < _HAMMER_BODY_RATIO_);
}

bool CandlesData::IsDragonflyDoji(size_t idx) const
{
	if (!IsDoji(idx))
	{
		return false;
	}

	double upper_shadow = m_highs[idx] - m_opens[idx];
	double lower_shadow = m_closes[idx] - m_lows[idx];

	if (lower_shadow < DBL_EPSILON)
	{
		return false;
	}

	double upper_to_lower_ratio = upper_shadow / lower_shadow;

	return upper_to_lower_ratio < _DRAGON_FLY_DOJI_SHADOW_RATIO_;
}

bool CandlesData::IsGravestoneDoji(size_t idx) const
{
	if (!IsDoji(idx))
	{
		return false;
	}

	double upper_shadow = m_highs[idx] - m_opens[idx];
	double lower_shadow = m_closes[idx] - m_lows[idx];

	if (upper_shadow < DBL_EPSILON)
	{
		return false;
	}

	double lower_to_upper_ratio = lower_shadow / upper_shadow;

	return lower_to_upper_ratio < _GRAVE_STONE_DOJI_SHADOW_RATIO_;
}

bool CandlesData::IsLongLegDoji(size_t idx) const
{
	if (!IsDoji(idx))
	{
		return false;
	}

	double body_size = fabs(m_closes[idx] - m_opens[idx]);
	double total_range = m_highs[idx] - m_lows[idx];
	double upper_shadow = m_highs[idx] - m_opens[idx];
	double lower_shadow = m_closes[idx] - m_lows[idx];

	if (lower_shadow < DBL_EPSILON)
	{
		return false;
	}

	double shadow_ratio = upper_shadow / lower_shadow;

	return upper_shadow > body_size * _LONG_LEG_DOJI_BODY_TO_SHADO_RATIO &&		// upper shadow should be long
		lower_shadow > body_size * _LONG_LEG_DOJI_BODY_TO_SHADO_RATIO &&		// lower shadow should be long
		shadow_ratio >= (1.0 - _LONG_LEG_DOJI_SHADOW_RATIO) &&					// shadows ratio should be in range
		shadow_ratio <= (1.0 + _LONG_LEG_DOJI_SHADOW_RATIO);
}
