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

	WMA(size_t length) : Indicator(length) {}
	WMA(std::weak_ptr<Security> security, size_t length) : Indicator(security, length) {}

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;


	virtual std::string GetName() const override { return std::format("Weighted Moving Average ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_WMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<WMA>(*this);
	}
};

