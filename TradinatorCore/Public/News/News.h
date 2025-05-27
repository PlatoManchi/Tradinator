#pragma once

#include <memory>
#include <chrono>
#include <vector>

#include "Patterns/Pattern.h"



class Security;

class NewsPoint
{
public:
	NewsPoint();
	NewsPoint(std::shared_ptr<Security> security);

	NewsPoint(const NewsPoint& other) = default;
	NewsPoint(NewsPoint&& other) = default;
	NewsPoint& operator=(const NewsPoint& other) = default;
	NewsPoint& operator=(NewsPoint&& other) = default;

	// Which security this news belongs to
	std::shared_ptr<Security> m_security;

	// Most patterns/strategirs happen over a range of candles. This will hold the start and end indices of the range.
	std::vector<uint64_t> m_date_range;

	// cached date from database
	std::chrono::system_clock::time_point m_date;

	// If news is strategy type, then strategy ID
	int64_t m_strategy = -1;


	// If news is pattern type, then the pattern
	EPattern m_pattern = EPattern::None;
};

typedef std::vector<NewsPoint> NewsPointVectorType;