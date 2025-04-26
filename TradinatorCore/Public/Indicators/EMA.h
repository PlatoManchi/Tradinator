#pragma once
#include "Indicators/Indicator.h"

class EMA : public Indicator
{
public:
	EMA() : Indicator() {}

	EMA(size_t length) : Indicator(length) {}
	EMA(std::weak_ptr<Counter> counter, size_t length) : Indicator(counter, length) {}

	virtual std::vector<IndicatorPoint> Calculate() override;


	virtual std::string GetName() const override { return std::format("Exponential Moving Average ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_EMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<EMA>(*this);
	}
};

