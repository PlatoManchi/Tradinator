#pragma once

#include "Indicator.h"

// it will be extreme inefficient to do this in ispc since current value is dependent on previous value
// which means calculations cannot be parallalized
class OBV : public Indicator
{
public:
	OBV() : Indicator() {}

	OBV(std::weak_ptr<Counter> counter) : Indicator(counter, 0) {};

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;

	virtual std::string GetName() const override { return std::format("On Balance Volume"); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_OBV; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<OBV>(*this);
	}

	virtual bool IsSingleInstanceType() const { return true; }
};

