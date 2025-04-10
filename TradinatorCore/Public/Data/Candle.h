#pragma once

#include <chrono>
#include <cstdint>

struct Candle
{
public:
	Candle() = default;

	Candle(const Candle& other) = default;
	Candle(Candle&& other) noexcept = default;
	Candle& operator=(const Candle& other) = default;
	Candle& operator=(Candle&& other) noexcept = default;

	std::chrono::system_clock::time_point date;
	double open;
	double high;
	double low;
	double close;
	uint64_t volume;
	uint64_t open_interest;
};