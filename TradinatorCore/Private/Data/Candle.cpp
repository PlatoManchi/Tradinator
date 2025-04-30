#include "Data/Candle.h"

#include <iostream>
#include <cmath>

void Candle::WriteToFile(std::ofstream& stream) const
{
	std::chrono::system_clock::rep count = m_date.time_since_epoch().count();
	stream.write(reinterpret_cast<const char*>(&count), sizeof(count));
	stream.write(reinterpret_cast<const char*>(&m_open), sizeof(m_open));
	stream.write(reinterpret_cast<const char*>(&m_high), sizeof(m_high));
	stream.write(reinterpret_cast<const char*>(&m_low), sizeof(m_low));
	stream.write(reinterpret_cast<const char*>(&m_close), sizeof(m_close));
	stream.write(reinterpret_cast<const char*>(&m_volume), sizeof(m_volume));
	stream.write(reinterpret_cast<const char*>(&m_open_interest), sizeof(m_open_interest));

}

std::string Candle::ToString() const
{
	return std::format("Date: {}\nOpen: {}\nHigh: {}\nLow: {}\n Close: {}\nVolume: {}\nOpen Interest: {}",
		std::format("{:%d-%b-%Y %H:%M:%S}", m_date),
		m_open, m_high, m_low, m_close, m_volume, m_open_interest);
}

void Candle::ReadFromStream(std::istream& stream)
{
	std::chrono::system_clock::rep count;
	stream.read(reinterpret_cast<char*>(&count), sizeof(count));
	std::chrono::system_clock::duration duration_since_epoch(count);
	std::chrono::system_clock::time_point date(duration_since_epoch);
	m_date= date;

	stream.read(reinterpret_cast<char*>(&m_open), sizeof(m_open));
	stream.read(reinterpret_cast<char*>(&m_high), sizeof(m_high));
	stream.read(reinterpret_cast<char*>(&m_low), sizeof(m_low));
	stream.read(reinterpret_cast<char*>(&m_close), sizeof(m_close));
	stream.read(reinterpret_cast<char*>(&m_volume), sizeof(m_volume));
	stream.read(reinterpret_cast<char*>(&m_open_interest), sizeof(m_open_interest));
}


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








std::ofstream& operator << (std::ofstream& stream, Candle& candle)
{
	candle.WriteToFile(stream);

	return stream;
}

std::ostream& operator << (std::ostream& stream, Candle& candle)
{
	stream << candle.ToString();

	return stream;
}

std::istream& operator >> (std::istream& stream, Candle& candle)
{
	candle.ReadFromStream(stream);

	return stream;
}