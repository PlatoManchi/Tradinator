#pragma once

#include "Market.h"

#include <string>

// Class that handles nse market
class NSE_Market : public Market
{
public:
	NSE_Market();

	
	virtual std::string GetMarketName() const override { return "National Stock Exchange, India"; }
	virtual std::string GetMarketCode() const override { return "NSE"; }

	
protected:
	virtual void ParseCounterListData() override;

private:
	std::string GetCounterListRawDataFileName() const;
	std::string GetCounterListProcessedDataFileName() const;

	bool IsRawFileExist() const;
	bool IsProcessedFileExist() const;
};

