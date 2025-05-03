#pragma once

#include <map>
#include <chrono>
#include <cstdint>
#include <fstream>

struct Candle
{
public:
	Candle() = default;

	Candle(const Candle& other) = default;
	Candle(Candle&& other) noexcept = default;
	Candle& operator=(const Candle& other) = default;
	Candle& operator=(Candle&& other) noexcept = default;

	inline bool IsRed() const
	{
		return m_open > m_close;
	}
	inline bool IsGreen() const
	{
		return m_close > m_open;
	}

	bool IsDoji() const;
	bool IsMarubozu() const;
	bool IsHammer() const;
	bool IsInvertedHammer() const;
	bool IsDragonflyDoji() const;
	bool IsGravestoneDoji() const;
	bool IsLongLegDoji() const;


	/**
	* Volume should be a unsigned int. But for one stock one one particular day, the volume from upstox website
	* shows negative volume which breaks entire program. 
	* 
	* https://api.upstox.com/v2/historical-candle/NSE_EQ%7CINE669E01016/day/2024-08-30/2024-08-30
	* 
	* Because of this had to change that
	*/
	std::chrono::system_clock::time_point m_date;
	double m_open;
	double m_high;
	double m_low;
	double m_close;
	size_t m_volume;
	size_t m_open_interest;
	
	
};



typedef std::map<std::chrono::system_clock::time_point, Candle, std::greater<std::chrono::system_clock::time_point>> CandleDataMapType;