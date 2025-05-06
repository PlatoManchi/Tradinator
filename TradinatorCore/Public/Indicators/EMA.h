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
	EMA(EIndicatorSource source) : Indicator(source) {}
	EMA(size_t length) : Indicator(length) {}
	EMA(EIndicatorSource source, size_t length) : Indicator(source, length) {}
	EMA(size_t length, std::weak_ptr<Security> security) : Indicator(length, security) {}
	EMA(EIndicatorSource source, size_t length, std::weak_ptr<Security> security) : Indicator(source, length, security) {}

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* input, double* output, size_t window_size, size_t data_size);


	virtual std::string GetName() const override { return std::format("Exponential Moving Average ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_EMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<EMA>(*this);
	}
};

