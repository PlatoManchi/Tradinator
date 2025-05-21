#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Utils/AsyncTaskManager.h"
#include "Data/AsyncData.h"

#include "News/News.h"

class AsyncTaskManager;
class Market;
class Security;

class TradinatorCoreThread : public std::enable_shared_from_this<TradinatorCoreThread>
{
public:
	TradinatorCoreThread(std::string data_folder_path);

	// Call Init after adding all the markets. Not before.
	void Init();
	void Update();
	void Shutdown();


	// Taking market as rvalue ref because we want to take the ownership 
	// and want to make sure that caller understands that caller no longer has ownership
	void AddMarket(std::shared_ptr<Market>&& market);
	bool CanSafelyShutdown() const;
	
	const AsyncData<std::vector<std::weak_ptr<Security>>>& GetTenNewestIPOs() const;
	const AsyncData<NewsPointVectorType>& GetGlobalNews() const { return m_global_news; }


	inline bool IsProcessing() const { return m_async_task_manager->IsProcessing(); }

	inline std::string GetTradinatorWorkingFolderPath() const { return m_data_folder_path; }
	inline std::shared_ptr<AsyncTaskManager> GetAsyncTaskManager() const { return m_async_task_manager; }
	inline std::vector<std::shared_ptr<Market>> GetAllMarkets() const { return m_market_list; }
	

private:
	void InitializeDB();
	void OnSecurityDataLoaded();
	void OnCheckingForNewDataCompleted();
	void OnDownloadAndWriteCompleted();

	// Working directory for all data
	std::string m_data_folder_path;

	// Task manager
	std::shared_ptr<AsyncTaskManager> m_async_task_manager;
	
	// List of markets that this core will process
	std::vector<std::shared_ptr<Market>> m_market_list;

	// Will contain cummulation of news points from all securities in all markets.
	AsyncData<NewsPointVectorType> m_global_news;

	bool m_is_initialized;
	bool m_is_shut_down;
};

