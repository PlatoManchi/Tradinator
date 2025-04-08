#pragma once

#include <vector>
#include <memory>
#include <string>
#include <thread>


//class ThreadManager;
class Market;
class ThreadManager;

static std::string _RAW_DATA_FOLDER_ = "Raw Data";
static std::string _PROCESSED_DATA_FOLDER_ = "Processed Data";

class TradinatorCore
{
public:
	TradinatorCore(std::string data_folder_path);

	void InitializeAllMarkets();
	void Update();

	// Taking market as rvalue ref because we want to take the ownership 
	// and want to make sure that caller understands that caller no longer has ownership
	void AddMarket(std::shared_ptr<Market>&& market);

	std::vector<std::shared_ptr<Market>> GetAllMarkets() const { return m_market_list; }
	std::shared_ptr<ThreadManager> GetThreadManager() const { return m_thread_manager; }

	inline std::string GetDataFolderPath() const { return m_data_folder_path; }
	inline std::thread::id GetTradinatorCoreThreadID() const { return m_tradinator_core_thread_id; }

private:
	// The ID of thread TradinatorCore is constructed on.
	// Caching and using this to make debugging easy
	const std::thread::id m_tradinator_core_thread_id;


	std::shared_ptr<ThreadManager> m_thread_manager;

	// List of markets that this core will process
	std::vector<std::shared_ptr<Market>> m_market_list;

	std::string m_data_folder_path;
};