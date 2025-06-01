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
	ROC(EIndicatorSource source) : Indicator(source) {}
	ROC(uint64_t length) : Indicator(length) {}
	ROC(EIndicatorSource source, uint64_t length) : Indicator(source, length) {}
	ROC(uint64_t length, std::weak_ptr<Security> security) : Indicator(length, security) {}
	ROC(EIndicatorSource source, uint64_t length, std::weak_ptr<Security> security) : Indicator(source, length, security) {}

	ROC(const ROC& other) = default;
	ROC(ROC&& other) noexcept = default;
	ROC& operator=(const ROC& other) = default;
	ROC& operator=(ROC&& other) noexcept = default;

	bool operator==(const ROC& other) const;

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* input, double* output, uint64_t window_size, uint64_t data_size);

	virtual std::string GetName() const override { return std::format("Rate Of Change ({})", m_length); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_ROC; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<ROC>(*this);
	}
};

