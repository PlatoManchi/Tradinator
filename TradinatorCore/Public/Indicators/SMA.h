#pragma once
#include "Indicators/Indicator.h"

#include <memory>

class Counter;

class SMA final : public Indicator
{
public:
	SMA() : Indicator() {}

	SMA(size_t length);
	SMA(std::weak_ptr<Counter> counter, size_t length);

	virtual std::vector<IndicatorPoint> Calculate() override;


	virtual std::string GetName() const override { return "Simple Moving Average"; }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_SMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<SMA>(*this);
	}
};

