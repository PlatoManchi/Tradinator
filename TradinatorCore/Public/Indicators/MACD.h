#pragma once

#if 1
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

	MACD(EIndicatorSource source) : Indicator(source)
	{
		m_period_1 = 12;
		m_period_2 = 26;
		m_signal_period = 9;
	}

	MACD(uint64_t period_1, uint64_t period_2, uint64_t signal_period) : Indicator()
	{
		m_period_1 = period_1;
		m_period_2 = period_2;
		m_signal_period = signal_period;
	}

	MACD(EIndicatorSource source, uint64_t period_1, uint64_t period_2, uint64_t signal_period) : Indicator(source)
	{
		m_period_1 = period_1;
		m_period_2 = period_2;
		m_signal_period = signal_period;
	}

	MACD(uint64_t period_1, uint64_t period_2, uint64_t signal_period, std::weak_ptr<Security> security) : Indicator(0, security)
	{
		m_period_1 = period_1;
		m_period_2 = period_2;
		m_signal_period = signal_period;
	}

	MACD(EIndicatorSource source, uint64_t period_1, uint64_t period_2, uint64_t signal_period, std::weak_ptr<Security> security) : Indicator(source, 0, security)
	{
		m_period_1 = period_1;
		m_period_2 = period_2;
		m_signal_period = signal_period;
	}

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(const double* input, double* ema_period_1_buffer, double* ema_period_2_buffer, double* macd_output, double* signal_output, double* histogram_output, uint64_t period_1, uint64_t period_2, uint64_t signal_period, uint64_t data_size);

	virtual std::string GetName() const override { return std::format("MACD ({}, {}, {})", m_period_1, m_period_2, m_signal_period); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_MACD; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<MACD>(*this);
	}

	inline uint64_t GetPeriod_1() const { return m_period_1; }
	inline uint64_t GetPeriod_2() const { return m_period_2; }
	inline uint64_t GetSignalPeriod() const { return m_signal_period; }

	inline void SetPeriod_1(uint64_t period_1) { m_period_1 = period_1; }
	inline void SetPeriod_2(uint64_t period_2) { m_period_2 = period_2; }
	inline void SetSignalPeriod(uint64_t signal_period) { m_signal_period = signal_period; }

protected:
	uint64_t m_period_1;
	uint64_t m_period_2;
	uint64_t m_signal_period;
};

