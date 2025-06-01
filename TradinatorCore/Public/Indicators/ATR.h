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

	ATR(uint64_t length) : Indicator(length) {};
	ATR(std::weak_ptr<Security> security, uint64_t length) : Indicator(length, security) {};

	ATR(const ATR& other) = default;
	ATR(ATR&& other) noexcept = default;
	ATR& operator=(const ATR& other) = default;
	ATR& operator=(ATR&& other) noexcept = default;

	bool operator==(const ATR& other) const;

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* highs,	const double* lows, const double* closes, double* output, uint64_t window_size, uint64_t data_size);

	virtual std::string GetName() const override { return std::format("Average True Range ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_ATR; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<ATR>(*this);
	}
};

