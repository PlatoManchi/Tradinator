#pragma once

#if 1
#define _INDICATORS_ISPC_
#else
#endif

#include <vector>

#include "Indicators/Indicator.h"


class CandlesData;

class Strategy
{
public:
	virtual EStrategy GetStrategyType() const = 0;

	virtual std::vector<bool> Check(const CandlesData& candles_data) = 0;
	virtual std::vector<std::unique_ptr<Indicator>> GetIndicatorsNeeded() const = 0;
};


class Long_Strategy_1 : public Strategy
{
	virtual EStrategy GetStrategyType() const { return EStrategy::Long_Strategy_1; };

	virtual std::vector<bool> Check(const CandlesData& candles_data) override;
	virtual std::vector<std::unique_ptr<Indicator>> GetIndicatorsNeeded() const override;
};


class Short_Strategy_1 : public Strategy
{
	virtual EStrategy GetStrategyType() const { return EStrategy::Short_Strategy_1; };

	virtual std::vector<bool> Check(const CandlesData& candles_data) override;
	virtual std::vector<std::unique_ptr<Indicator>> GetIndicatorsNeeded() const override;
};