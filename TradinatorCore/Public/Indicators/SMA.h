#pragma once

#if 1
#define _SMA_ISPC_
#else
#endif


#include "Indicators/Indicator.h"

#include <memory>

class Security;

class SMA final : public Indicator
{
public:
	SMA() : Indicator() {}

	SMA(size_t length) : Indicator(length) {};
	SMA(std::weak_ptr<Security> security, size_t length) : Indicator(security, length) {};

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;
	static void CalculateRaw(double* input, double* output, int64_t data_size, int64_t window_size);

	virtual std::string GetName() const override { return std::format("Simple Moving Average ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_SMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<SMA>(*this);
	}
};

