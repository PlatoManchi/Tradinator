#pragma once

#include <map>
#include <chrono>
#include <cstdint>
#include <fstream>


class CandlesData
{
public:
	CandlesData() = default;

	CandlesData(const CandlesData& other) = default;
	CandlesData(CandlesData&& other) noexcept = default;
	CandlesData& operator=(const CandlesData& other) = default;
	CandlesData& operator=(CandlesData&& other) noexcept = default;

	inline bool IsBearish(size_t idx) const
	{
		return m_opens[idx] > m_closes[idx];
	}
	inline bool IsBullish(size_t idx) const
	{
		return m_closes[idx] > m_opens[idx];
	}

	bool IsDoji(size_t idx) const;
	bool IsMarubozu(size_t idx) const;
	bool IsHammer(size_t idx) const;
	bool IsInvertedHammer(size_t idx) const;
	bool IsDragonflyDoji(size_t idx) const;
	bool IsGravestoneDoji(size_t idx) const;
	bool IsLongLegDoji(size_t idx) const;


	/**
	* Volume should be a unsigned int. But for one stock one one particular day, the volume from upstox website
	* shows negative volume which breaks entire program.
	*
	* https://api.upstox.com/v2/historical-candle/NSE_EQ%7CINE669E01016/day/2024-08-30/2024-08-30
	*
	* Because of this had to change that
	*/
	std::vector<std::chrono::system_clock::time_point> m_dates;
	std::vector<double> m_opens;
	std::vector<double> m_highs;
	std::vector<double> m_lows;
	std::vector<double> m_closes;
	std::vector<uint64_t> m_volumes;
	std::vector<uint64_t> m_open_interests;
};