#pragma once

#if 1
#define _BOLLINGER_BAND_ISPC_
#else
#endif

#include "Indicators/Indicator.h"


class BollingerBand : public Indicator
{
public:
	BollingerBand() : Indicator() 
	{
		m_standard_deviation_multiplier = 2.0;
	}

	BollingerBand(EIndicatorSource source) : Indicator(source)
	{
		m_standard_deviation_multiplier = 2.0;
	}

	BollingerBand(uint64_t length, double standard_deviation_multiplier) 
		: Indicator(length)
	{
		m_standard_deviation_multiplier = standard_deviation_multiplier;
	}

	BollingerBand(EIndicatorSource source, uint64_t length, double standard_deviation_multiplier)
		: Indicator(source, length)
	{
		m_standard_deviation_multiplier = standard_deviation_multiplier;
	}

	BollingerBand(uint64_t length, std::weak_ptr<Security> security, double standard_deviation_multiplier)
		: Indicator(length, security)
	{
		m_standard_deviation_multiplier = standard_deviation_multiplier;
	}

	BollingerBand(EIndicatorSource source, uint64_t length, std::weak_ptr<Security> security, double standard_deviation_multiplier)
		: Indicator(source, length, security)
	{
		m_standard_deviation_multiplier = standard_deviation_multiplier;
	}

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* input, double* top, double* sma, double* bottom, uint64_t window_size, double standard_deviation_multiplier, uint64_t data_size);

	double GetStandardDeviationMultiplier() const { return m_standard_deviation_multiplier; }
	void SetStandardDeviationMultiplier(double multiplier)
	{
		m_standard_deviation_multiplier = multiplier;
	}

	virtual std::string GetName() const override { return std::format("Bollinger Band ({}, {})", m_length, m_standard_deviation_multiplier); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_BOLLINGER_BAND; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<BollingerBand>(*this);
	}

protected:
	double m_standard_deviation_multiplier;
};

