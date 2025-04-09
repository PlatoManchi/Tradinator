#pragma once

#include <string>
#include <memory>
#include <map>

#include "Data/Security.h"
#include "Data/AsyncData.h"

class TradinatorCore;

// Base class for all markets.
class Market
{
public:
	Market(std::shared_ptr<TradinatorCore> tradinator_core);

	virtual std::string GetMarketName() const = 0;
	virtual std::string GetMarketCode() const = 0;

	virtual void GatherSymbols() = 0;

	// returns true if the market data is processed and ready.
	virtual bool IsValid() const { return false; }

	void CreateFolderStructure() const;
	std::string GetRawDataFolderPath() const;
	std::string GetProcessedDataFolderPath() const;

	std::string GetRawDataFilePath() const;
	std::string GetProcessedDataFilePath() const;

	virtual std::string GetRawDataFilePathName() const = 0;
	virtual std::string GetProcessedDataFileName() const = 0;

	inline std::weak_ptr<TradinatorCore> GetTradinatorCore() const { return m_tradinator_core; }

protected:
	// get the weak_ptr from the shared_ptr stored in TradinatorCore
	std::weak_ptr<Market> GetMarket() const;

	std::shared_ptr<TradinatorCore> m_tradinator_core;

	// key value pair of Symbol to security for easier and faster access
	//std::map<std::string, std::shared_ptr<Security>> m_securities_list;

	// Temporary one that is used by async loading during loading phase.
	// Once all the data is loaded, then it will be copied into m_securities_list
	// Instead of using locks, use copies for lock free execution
	//std::map<std::string, std::shared_ptr<Security>> m_securities_list_loader;

	AsyncData<std::map<std::string, std::shared_ptr<Security>>> m_securities_async_data;
};

