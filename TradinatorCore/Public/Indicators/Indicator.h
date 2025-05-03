#pragma once

#include <memory>
#include <chrono>
#include <vector>


class Counter;

struct IndicatorPoint
{
	IndicatorPoint() : date(std::chrono::system_clock::duration(0)), value(0) {};

	std::chrono::system_clock::time_point date;
	double value;

	// copy and move sementics
	IndicatorPoint(const IndicatorPoint& other) = default;
	IndicatorPoint(IndicatorPoint&& other) noexcept = default;
	IndicatorPoint& operator = (const IndicatorPoint& other) = default;
	IndicatorPoint& operator = (IndicatorPoint&& other) noexcept = default;
};

enum EIndicatorType {
	MIN,

	E_SMA,
	E_WMA,
	E_EMA,
	E_BOLLINGER_BAND,
	E_ROC,
	E_RSI,
	E_OBV,
	E_MACD,
	E_ATR,


	MAX
};

class Indicator
{
public:
	Indicator();
	Indicator(size_t length);
	Indicator(std::weak_ptr<Counter> counter, size_t length);

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() = 0;
	virtual std::string GetName() const = 0;
	virtual EIndicatorType IndicatorType() const = 0;
	virtual std::unique_ptr<Indicator> Clone() = 0;

	// If true: only one instance of this indicator can be applied to a counter
	virtual bool IsSingleInstanceType() const { return false; }
	
	void SetCounter(std::weak_ptr<Counter> counter) { m_counter = counter; }
	void SetLength(size_t length) { m_length = length; }
	size_t& GetLength() { return m_length; }

protected:
	std::weak_ptr<Counter> m_counter;
	size_t m_length;
};

