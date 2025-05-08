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

	virtual std::string GetMarketName() const = 0;
	virtual std::string GetMarketCode() const = 0;



	virtual std::string GetSecurityListRawDataFileName() const = 0;
	virtual std::string GetSecurityListProcessedDataFileName() const = 0;
	
	std::string GetRawDataFolderPath() const;
	std::string GetProcessedDataFolderPath() const;

	std::string GetSecurityListRawDataFilePath() const;
	std::string GetSecurityListProcessedDataFilePath() const;

	std::unique_ptr<AsyncTask> GetGatherSecuritiesTask();
	std::unique_ptr<AsyncTask> GetParallelDownloadTask();
	std::unique_ptr<AsyncTask> GetSerialWriteTask();
	std::vector<std::unique_ptr<AsyncTask>> GetGenerateNewsPointsTask();

	inline void SetOwningTradinatorCoreThread(std::weak_ptr<TradinatorCoreThread> owning_tradinator_core_thread) { m_owning_tradinator_core_thread = owning_tradinator_core_thread; }
	inline std::weak_ptr<TradinatorCoreThread> GetTradinatorCoreThread() const { return m_owning_tradinator_core_thread; }
	inline bool IsSecurityDataAvailable() const { return m_securities_async_data.IsDataReady(); }
	inline const AsyncData<std::map<std::string, std::shared_ptr<Security>>>& GetSecurityAsyncData() const { return m_securities_async_data; }
	inline const AsyncData<std::vector<std::weak_ptr<Security>>>& GetTenNewestIPOs() const { return m_ten_newest_securities; }

protected:
	// Each market will have different ways of organizing and acquiring data.
	virtual void ParseSecurityListData() = 0;
	void FindTenNewestIPOs();
	void CreateFolderStructure() const;

	std::weak_ptr<TradinatorCoreThread> m_owning_tradinator_core_thread;


	AsyncData<std::vector<std::weak_ptr<Security>>> m_ten_newest_securities;
	
	
	//
	AsyncData<std::map<std::string, std::shared_ptr<Security>>> m_securities_async_data;
};

