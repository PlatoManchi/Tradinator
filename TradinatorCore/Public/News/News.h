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

	// If new is pattern type, then the pattern
	EPatternType m_pattern;
};

typedef std::vector<NewsPoint> NewsPointVectorType;