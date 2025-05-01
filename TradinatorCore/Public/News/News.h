#pragma once

#include <memory>
#include <chrono>

#include "Patterns/Pattern.h"

class Counter;

class NewsPoint
{
public:
	std::shared_ptr<Counter> m_counter;
	std::chrono::system_clock::time_point m_date;
	EPatternType m_patterns;

};

typedef std::map<std::chrono::system_clock::time_point, NewsPoint, std::greater<std::chrono::system_clock::time_point>> NewsPointMapType;