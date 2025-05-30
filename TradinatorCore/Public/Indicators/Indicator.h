#pragma once

#include <memory>
#include <chrono>
#include <vector>


#include "TradinatorCoreTypes.h"

class Security;

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


class Indicator
{
public:
	Indicator();
	Indicator(EIndicatorSource source);
	Indicator(uint64_t length);
	Indicator(EIndicatorSource source, uint64_t length);
	Indicator(uint64_t length, std::weak_ptr<Security> security);
	Indicator(EIndicatorSource source, uint64_t length, std::weak_ptr<Security> security);

	virtual std::vector<std::vector<double>> Calculate() = 0;
	virtual std::string GetName() const = 0;
	virtual EIndicatorType IndicatorType() const = 0;
	virtual std::unique_ptr<Indicator> Clone() = 0;

	// If true: only one instance of this indicator can be applied to a security
	virtual bool IsSingleInstanceType() const { return false; }
	
	void SetSecurity(std::weak_ptr<Security> security) { m_security = security; }
	void SetLength(uint64_t length) 
	{ 
		m_length = length; 
	}
	uint64_t& GetLength() { return m_length; }

protected:

	EIndicatorSource m_source;
	uint64_t m_length;
	std::weak_ptr<Security> m_security;
};

