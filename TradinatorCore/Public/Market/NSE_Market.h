#pragma once

#include "Market.h"

#include <string>


// Class that handles nse market
class NSE_Market : public Market
{
public:
	NSE_Market();

	virtual void Init() override;

	virtual bool IsValid() const override;

	virtual std::string GetMarketName() const override { return "NSE (National Stock Exchange, India)"; }
	virtual std::string GetMarketCode() const override { return "NSE"; }

	virtual void GatherSymbols() override;

private:
	void OnGatherSymbolsCompleted();
	void OnSecurityDataLoaded();

	std::string GetRawDataFilePathName() const;
	std::string GetProcessedDataFileName() const;

	bool IsRawFileExist() const;
	bool IsProcessedFileExist() const;
};

