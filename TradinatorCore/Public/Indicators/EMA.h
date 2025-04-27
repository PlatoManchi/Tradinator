#pragma once

// Because each value is dependent on previous value, calculations cannot be parallelized.
// Which makes ISPC version is slow because of overhead of making it gather previous value
#if 0
#define _EMA_ISPC_
#else
#endif


#include "Indicators/Indicator.h"

class EMA : public Indicator
{
public:
	EMA() : Indicator() {}

	EMA(size_t length) : Indicator(length) {}
	EMA(std::weak_ptr<Counter> counter, size_t length) : Indicator(counter, length) {}

	virtual std::vector<IndicatorPoint> Calculate() override;
	static void CalculateRaw(double* input, double* output, int64_t data_size, int64_t window_size);


	virtual std::string GetName() const override { return std::format("Exponential Moving Average ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_EMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<EMA>(*this);
	}
};

