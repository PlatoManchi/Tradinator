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

	BollingerBand(size_t length, double standard_deviation_multiplier) : Indicator(length)
	{
		m_standard_deviation_multiplier = standard_deviation_multiplier;
	}

	BollingerBand(std::weak_ptr<Counter> counter, size_t length, double standard_deviation_multiplier) : Indicator(counter, length)
	{
		m_standard_deviation_multiplier = standard_deviation_multiplier;
	}

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;
	
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

