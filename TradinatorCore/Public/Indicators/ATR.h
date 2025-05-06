#pragma once

#if 1
#define _ATR_ISPC_
#else
#endif

#include "Indicators/Indicator.h"

class ATR : public Indicator
{
public:
	ATR() : Indicator() {}

	ATR(size_t length) : Indicator(length) {};
	ATR(std::weak_ptr<Security> security, size_t length) : Indicator(security, length) {};

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;

	virtual std::string GetName() const override { return std::format("Average True Range ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_ATR; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<ATR>(*this);
	}
};

