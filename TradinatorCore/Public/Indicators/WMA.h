#pragma once

#if 1
#define _WMA_ISPC_
#else
#endif


#include "Indicators/Indicator.h"

#include <memory>

class WMA : public Indicator
{
public:
	WMA() : Indicator() {}
	WMA(EIndicatorSource source) : Indicator(source) {}
	WMA(size_t length) : Indicator(length) {}
	WMA(EIndicatorSource source, size_t length) : Indicator(source, length) {}
	WMA(size_t length, std::weak_ptr<Security> security) : Indicator(length, security) {}
	WMA(EIndicatorSource source, size_t length, std::weak_ptr<Security> security) : Indicator(source, length, security) {}


	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* input, double* output, int64_t window_size, int64_t data_size);

	virtual std::string GetName() const override { return std::format("Weighted Moving Average ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_WMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<WMA>(*this);
	}
};

