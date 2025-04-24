#pragma once
#include "Indicators/Indicator.h"

#include <memory>

class WMA : public Indicator
{
public:
	WMA() : Indicator() {}

	WMA(size_t length) : Indicator(length) {}
	WMA(std::weak_ptr<Counter> counter, size_t length) : Indicator(counter, length) {}

	virtual std::vector<IndicatorPoint> Calculate() override;


	virtual std::string GetName() const override { return "Weighted Moving Average"; }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_WMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<WMA>(*this);
	}
};

