#pragma once

#if 1
#define _ROC_ISPC_
#else
#endif

#include "Indicators/Indicator.h"


class ROC : public Indicator
{
public:
	ROC() : Indicator() {}

	ROC(size_t length) : Indicator(length) {}
	ROC(std::weak_ptr<Counter> counter, size_t length) : Indicator(counter, length) {}

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;

	virtual std::string GetName() const override { return std::format("Rate Of Change ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_ROC; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<ROC>(*this);
	}
};

