#pragma once

#if 1
#define _RSI_ISPC_
#else
#endif


#include "Indicator.h"
class RSI : public Indicator
{
public:
	RSI() : Indicator() {}
	RSI(size_t length) : Indicator(length) {}
	RSI(std::weak_ptr<Security> security, size_t length) : Indicator(security, length) {}

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;

	virtual std::string GetName() const override { return std::format("Relative Strength Index ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_RSI; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<RSI>(*this);
	}
};

