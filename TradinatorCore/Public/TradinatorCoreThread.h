#pragma once

#include <memory>
#include <string>
#include <vector>

class AsyncTaskManager;
class Market;

class TradinatorCoreThread : public std::enable_shared_from_this<TradinatorCoreThread>
{
public:
	TradinatorCoreThread(std::string data_folder_path);
	void Update();
	void Shutdown();


	// Taking market as rvalue ref because we want to take the ownership 
	// and want to make sure that caller understands that caller no longer has ownership
	void AddMarket(std::shared_ptr<Market>&& market);
	bool CanSafelyShutdown() const;


	inline std::string GetDataFolderPath() const { return m_data_folder_path; }
	inline std::shared_ptr<AsyncTaskManager> GetAsyncTaskManager() const { return m_async_task_manager; }
	inline std::vector<std::shared_ptr<Market>> GetAllMarkets() const { return m_market_list; }
	
private:
	// Working directory for all data
	std::string m_data_folder_path;

	std::shared_ptr<AsyncTaskManager> m_async_task_manager;
	
	// List of markets that this core will process
	std::vector<std::shared_ptr<Market>> m_market_list;

	bool m_is_shut_down;
};

