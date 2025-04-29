#pragma once

#if 0
#define _MACD_ISPC_
#else
#endif

#include "Indicator.h"


class MACD : public Indicator
{
public:
	MACD() : Indicator()
	{
		m_period_1 = 12;
		m_period_2 = 26;
		m_signal_period = 9;
	}

	MACD(size_t period_1, size_t period_2, size_t signal_period) : Indicator()
	{
		m_period_1 = period_1;
		m_period_2 = period_2;
		m_signal_period = signal_period;
	}

	MACD(std::weak_ptr<Counter> counter, size_t period_1, size_t period_2, size_t signal_period) : Indicator(counter, 0)
	{
		m_period_1 = period_1;
		m_period_2 = period_2;
		m_signal_period = signal_period;
	}

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;
	

	virtual std::string GetName() const override { return std::format("MACD ({}, {}, {})", m_period_1, m_period_2, m_signal_period); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_MACD; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<MACD>(*this);
	}

	inline size_t GetPeriod_1() const { return m_period_1; }
	inline size_t GetPeriod_2() const { return m_period_2; }
	inline size_t GetSignalPeriod() const { return m_signal_period; }

	inline void SetPeriod_1(size_t period_1) { m_period_1 = period_1; }
	inline void SetPeriod_2(size_t period_2) { m_period_2 = period_2; }
	inline void SetSignalPeriod(size_t signal_period) { m_signal_period = signal_period; }

protected:
	size_t m_period_1;
	size_t m_period_2;
	size_t m_signal_period;
};

