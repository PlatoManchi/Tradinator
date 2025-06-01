#pragma once

// Because each value is dependent on previous value, calculations cannot be parallelized.
// Which makes ISPC version is slow because of overhead of making it gather previous value
#if 1
#define _EMA_ISPC_
#else
#endif


#include "Indicators/Indicator.h"

class EMA : public Indicator
{
public:
	EMA() : Indicator() {}
	EMA(EIndicatorSource source) : Indicator(source) {}
	EMA(uint64_t length) : Indicator(length) {}
	EMA(EIndicatorSource source, uint64_t length) : Indicator(source, length) {}
	EMA(uint64_t length, std::weak_ptr<Security> security) : Indicator(length, security) {}
	EMA(EIndicatorSource source, uint64_t length, std::weak_ptr<Security> security) : Indicator(source, length, security) {}

	EMA(const EMA& other) = default;
	EMA(EMA&& other) noexcept = default;
	EMA& operator=(const EMA& other) = default;
	EMA& operator=(EMA&& other) noexcept = default;

	bool operator==(const EMA& other) const;

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* input, double* output, uint64_t window_size, uint64_t data_size);


	virtual std::string GetName() const override { return std::format("Exponential Moving Average ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_EMA; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<EMA>(*this);
	}
};

