#pragma once

#include "Market.h"

#include <string>
#include <thread>




class TradinatorCore;

// Class that handles nse market
class NSE_Market : public Market
{
public:
	NSE_Market(std::shared_ptr<TradinatorCore> tradinator_core);

	virtual bool IsValid() const override;

	virtual std::string GetMarketName() const override
	{
		return "NSE Market";
	}

	virtual void GatherSymbols() override;

	void function();
private:
	void OnGatherSymbolsCompleted();

	bool IsRawFileExist() const;
	bool IsProcessedFileExist() const;
};

