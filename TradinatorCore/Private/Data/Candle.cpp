#include "Data/Candle.h"

#include <iostream>
#include <cmath>


bool Candle::IsDoji() const
{
	// 5.0 is Magic number. Adjust for better results;
	return fabs(m_open - m_close) < 5.0;
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
		return (fabs(m_open - m_high) <= LDBL_EPSILON && fabs(m_close - m_low) <= LDBL_EPSILON) || // red candle case
			(fabs(m_open - m_low) <= LDBL_EPSILON && fabs(m_close - m_high) <= LDBL_EPSILON); // green candle case
	}

	return false;
}


