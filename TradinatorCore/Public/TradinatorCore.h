#pragma once

#include <vector>
#include <memory>
#include <string>

#include "TradinatorCoreThread.h"

static std::string _RAW_DATA_FOLDER_ = "Raw Data";
static std::string _PROCESSED_DATA_FOLDER_ = "Processed Data";

class TradinatorCore final
{
public:
	TradinatorCore(std::string data_folder_path);
	~TradinatorCore();
	void Shutdown();


	// Taking market as rvalue ref because we want to take the ownership 
	// and want to make sure that caller understands that caller no longer has ownership
	void AddMarket(std::shared_ptr<Market>&& market);

	inline std::vector<std::shared_ptr<Market>> GetAllMarkets() const { return m_tradinator_core_thread->GetAllMarkets(); }
	inline std::shared_ptr<AsyncTaskManager> GetThreadManager() const { return m_tradinator_core_thread->GetAsyncTaskManager(); }
	inline std::string GetDataFolderPath() const { return m_tradinator_core_thread->GetDataFolderPath(); }

private:
	// Core is going to run as a detached thread from main thread on its own.
	// TradinatorCore class is used to interface outside world into core thread.
	std::shared_ptr<TradinatorCoreThread> m_tradinator_core_thread;


	bool m_is_shutting_down;
};