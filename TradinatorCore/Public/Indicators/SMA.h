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
	SMA(EIndicatorSource source) : Indicator(source) {}
	SMA(uint64_t length) : Indicator(length) {}
	SMA(EIndicatorSource source, uint64_t length) : Indicator(source, length) {}
	SMA(uint64_t length, std::weak_ptr<Security> security) : Indicator(length, security) {}
	SMA(EIndicatorSource source, uint64_t length, std::weak_ptr<Security> security) : Indicator(source, length, security) {}

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* input, double* output, uint64_t window_size, uint64_t data_size);

	virtual std::string GetName() const override { return std::format("Simple Moving Average ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_SMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<SMA>(*this);
	}
};

