#include "TradinatorCoreThread.h"

#include <thread>

#include "SQLiteCpp/SQLiteCpp.h"

#include "Market/Market.h"
#include "Utils/AsyncTaskManager.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"

TradinatorCoreThread::TradinatorCoreThread(std::string data_folder_path)
	: m_data_folder_path(data_folder_path)
	, m_async_task_manager(std::make_shared<AsyncTaskManager>())
	, m_is_shut_down(false)
{
	Log::GetInstance().SetFolderPath(m_data_folder_path);
	TradinatorCoreSpace::Utils::SetTradinatorWorkingFolderPath(m_data_folder_path);
	TradinatorCoreSpace::Utils::SetupFolderStructure();

	InitializeDB();
}

void TradinatorCoreThread::AddMarket(std::shared_ptr<Market>&& market)
{
	std::shared_ptr<Market>& stored_market = m_market_list.emplace_back(market);
	stored_market->SetOwningTradinatorCoreThread(this->weak_from_this());
	stored_market->Init();
}



void TradinatorCoreThread::InitializeDB()
{
	try
	{
		SQLite::Database db(TradinatorCoreSpace::Utils::GetTradinatorDatabasePath(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		// Begin transaction
		SQLite::Transaction transaction(db);
		db.exec("CREATE TABLE IF NOT EXISTS Securities("  \
			"ISIN CHAR(12) PRIMARY KEY     NOT NULL," \
			"Symbol           TEXT         NOT NULL," \
			"Name             TEXT         NOT NULL," \
			"Series           TEXT         NOT NULL," \
			"DateOfListing    INTEGER      NOT NULL," \
			"PaidUpValue      INTEGER      NOT NULL," \
			"MarketLot        INTEGER      NOT NULL," \
			"FaceValue        INTEGER      NOT NULL," \
			"LatestCandleData INTEGER      NOT NULL );");
		transaction.commit();
	}
	

	catch (std::exception& e)
	{
		Log::GetInstance().Write(std::format("ERROR: SQLite exception: {}", e.what()));

		// Database might be locked by another thread. Wait for a bit and try again.
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void TradinatorCoreThread::Update()
{
	// meat and potatoes
	while (true)
	{
		if (m_is_shut_down)
		{
			bool can_safely_shut_down = m_async_task_manager.get() && m_async_task_manager->CanSafelyShutDown();

			if (can_safely_shut_down)
			{
				// If thread can safely shutdown, then break out of loop.
				break;
			}
		}

		m_async_task_manager->Update();

		// Update 1000 times in a second. Is it necessary? Can it be 100 times?
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

bool TradinatorCoreThread::CanSafelyShutdown() const
{
	return m_async_task_manager.get() && m_async_task_manager->CanSafelyShutDown();
}

void TradinatorCoreThread::Shutdown()
{
	m_is_shut_down = true;

	m_async_task_manager->Shutdown();
}

const AsyncData<std::vector<std::weak_ptr<Counter>>>& TradinatorCoreThread::GetTenNewestIPOs() const
{
	return m_market_list[0]->GetTenNewestIPOs();
}