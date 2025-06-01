#pragma once

#if 1
#define _RSI_ISPC_
#else
#endif


#include "Indicators/Indicator.h"


class RSI : public Indicator
{
public:
	RSI() : Indicator() { m_length = 14; }
	RSI(EIndicatorSource source) : Indicator(source) { m_length = 14; }
	RSI(uint64_t length) : Indicator(length) {}
	RSI(EIndicatorSource source, uint64_t length) : Indicator(source, length) {}
	RSI(uint64_t length, std::weak_ptr<Security> security) : Indicator(length, security) {}
	RSI(EIndicatorSource source, uint64_t length, std::weak_ptr<Security> security) : Indicator(source, length, security) {}

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* input, double* output, uint64_t window_size, uint64_t data_size);

	virtual std::string GetName() const override { return std::format("Relative Strength Index ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_RSI; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<RSI>(*this);
	}
};

