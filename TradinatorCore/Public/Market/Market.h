#pragma once

#include <string>
#include <memory>
#include <map>

#include "Data/Counter.h"
#include "Data/AsyncData.h"

class TradinatorCoreThread;



// Base class for all markets.
class Market : public std::enable_shared_from_this<Market>
{
public:
	Market();

	virtual std::string GetMarketName() const = 0;
	virtual std::string GetMarketCode() const = 0;



	virtual std::string GetCounterListRawDataFileName() const = 0;
	virtual std::string GetCounterListProcessedDataFileName() const = 0;
	
	std::string GetRawDataFolderPath() const;
	std::string GetProcessedDataFolderPath() const;

	std::string GetCounterListRawDataFilePath() const;
	std::string GetCounterListProcessedDataFilePath() const;

	std::unique_ptr<AsyncTask> GetGatherSecuritiesTask();
	std::unique_ptr<AsyncTask> GetParallelDownloadTask();
	std::unique_ptr<AsyncTask> GetSerialWriteTask();
	

	inline void SetOwningTradinatorCoreThread(std::weak_ptr<TradinatorCoreThread> owning_tradinator_core_thread) { m_owning_tradinator_core_thread = owning_tradinator_core_thread; }
	inline std::weak_ptr<TradinatorCoreThread> GetTradinatorCoreThread() const { return m_owning_tradinator_core_thread; }
	inline bool IsCounterDataAvailable() const { return m_securities_async_data.IsDataReady(); }
	inline const AsyncData<std::map<std::string, std::shared_ptr<Counter>>>& GetCounterAsyncData() const { return m_securities_async_data; }
	inline const AsyncData<std::vector<std::weak_ptr<Counter>>>& GetTenNewestIPOs() const { return m_ten_newest_counters; }

protected:
	// Each market will have different ways of organizing and acquiring data.
	virtual void ParseCounterListData() = 0;
	void FindTenNewestIPOs();
	void CreateFolderStructure() const;

	std::weak_ptr<TradinatorCoreThread> m_owning_tradinator_core_thread;


	AsyncData<std::vector<std::weak_ptr<Counter>>> m_ten_newest_counters;
	
	
	//
	AsyncData<std::map<std::string, std::shared_ptr<Counter>>> m_securities_async_data;
};

