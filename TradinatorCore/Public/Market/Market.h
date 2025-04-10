#pragma once

#include <string>
#include <memory>
#include <map>

#include "Data/Security.h"
#include "Data/AsyncData.h"

class TradinatorCoreThread;

// Base class for all markets.
class Market : public std::enable_shared_from_this<Market>
{
public:
	Market();

	virtual void Init() = 0;
	virtual std::string GetMarketName() const = 0;
	virtual std::string GetMarketCode() const = 0;

	virtual void GatherSymbols() = 0;

	// returns true if the market data is processed and ready.
	virtual bool IsValid() const { return false; }

	inline void SetOwningTradinatorCoreThread(std::weak_ptr<TradinatorCoreThread> owning_tradinator_core_thread) { m_owning_tradinator_core_thread = owning_tradinator_core_thread; }

	void CreateFolderStructure() const;
	std::string GetRawDataFolderPath() const;
	std::string GetProcessedDataFolderPath() const;

	std::string GetRawDataFilePath() const;
	std::string GetProcessedDataFilePath() const;

	virtual std::string GetRawDataFilePathName() const = 0;
	virtual std::string GetProcessedDataFileName() const = 0;

	inline std::weak_ptr<TradinatorCoreThread> GetTradinatorCoreThread() const { return m_owning_tradinator_core_thread; }

protected:
	std::weak_ptr<TradinatorCoreThread> m_owning_tradinator_core_thread;

	AsyncData<std::map<std::string, std::shared_ptr<Security>>> m_securities_async_data;
};

