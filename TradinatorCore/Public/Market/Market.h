#pragma once

#include <string>
#include <memory>
#include <map>

#include<Data/Equity.h>

class TradinatorCore;

// Base class for all markets.
class Market
{
public:
	Market(std::shared_ptr<TradinatorCore> tradinator_core);

	virtual void Update() = 0;
	virtual std::string GetMarketName() const = 0;

	virtual void GatherSymbols() = 0;

	// returns true if the market data is processed and ready.
	virtual bool IsValid() const { return false; }

	void CreateFolderStructure() const;
	std::string GetRawDataFolder() const;
	std::string GetProcessedDataFolder() const;

protected:
	std::shared_ptr<TradinatorCore> m_tradinator_core;

	// key value pair of symbol to equity for easier and faster access
	std::map<std::string, Equity> m_equities_list;
};

