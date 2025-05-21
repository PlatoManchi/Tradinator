#include "TradinatorCoreThread.h"

#include <memory>
#include <thread>

#include "SQLiteCpp/SQLiteCpp.h"

#include "Market/Market.h"
#include "Utils/AsyncTaskManager.h"
#include "Utils/AsyncTask.h"
#include "Utils/SerialAsyncTask.h"
#include "Utils/ParallelAsyncTask.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"

TradinatorCoreThread::TradinatorCoreThread(std::string data_folder_path)
	: m_data_folder_path(data_folder_path)
	, m_async_task_manager(std::make_shared<AsyncTaskManager>())
	, m_is_initialized(false)
	, m_is_shut_down(false)
{
	Log::GetInstance().SetFolderPath(m_data_folder_path);
	TradinatorCoreSpace::Utils::SetTradinatorWorkingFolderPath(m_data_folder_path);
	TradinatorCoreSpace::Utils::SetupFolderStructure();

	InitializeDB();
}

void TradinatorCoreThread::Init()
{
	std::vector<std::unique_ptr<AsyncTask>> init_markets_tasks_list;
	size_t count = m_market_list.size();

	// Gathering the security list
	if (count == 1)
	{
		init_markets_tasks_list.emplace_back(std::move(m_market_list[0]->GetGatherSecuritiesTask()));
	}
	else if(count > 1)
	{
		std::vector<std::unique_ptr<AsyncTask>> gather_securities_tasks_list;
		for (std::shared_ptr<Market> market : m_market_list)
		{
			gather_securities_tasks_list.emplace_back(std::move(market->GetGatherSecuritiesTask()));
		}

		init_markets_tasks_list.emplace_back(std::move(std::make_unique<SerialAsyncTask>(
			std::string(""),
			m_async_task_manager,
			std::move(gather_securities_tasks_list),
			[]() {}
		)));
	}


	m_is_initialized = true;
	std::unique_ptr<AsyncTask> init_task = std::move(std::make_unique<SerialAsyncTask>(
		std::string(""),
		m_async_task_manager,
		std::move(init_markets_tasks_list),
		[&]() 
		{
			OnSecurityDataLoaded();
		}
	));
	m_async_task_manager->AddTask(std::move(init_task));
}

void TradinatorCoreThread::OnSecurityDataLoaded()
{
	std::vector<std::unique_ptr<AsyncTask>> check_if_new_download_data_exist;
	size_t count = m_market_list.size();

	// Gathering the security list
	if (count == 1)
	{
		check_if_new_download_data_exist.emplace_back(std::move(m_market_list[0]->GetDoesNewDataExistToDownloadTask()));
	}
	else if (count > 1)
	{
		std::vector<std::unique_ptr<AsyncTask>> gather_securities_tasks_list;
		for (std::shared_ptr<Market> market : m_market_list)
		{
			gather_securities_tasks_list.emplace_back(std::move(market->GetDoesNewDataExistToDownloadTask()));
		}

		check_if_new_download_data_exist.emplace_back(std::move(std::make_unique<ParallelAsyncTask>(
			std::string(""),
			m_async_task_manager,
			std::move(gather_securities_tasks_list),
			[]() {}
		)));
	}

	std::unique_ptr<AsyncTask> init_task = std::move(std::make_unique<SerialAsyncTask>(
		std::string("Check to see if there is new data available online"),
		m_async_task_manager,
		std::move(check_if_new_download_data_exist),
		[&]()
		{
			OnCheckingForNewDataCompleted();
		}
	));

	m_async_task_manager->AddTask(std::move(init_task));
}


void TradinatorCoreThread::OnCheckingForNewDataCompleted()
{
	std::vector<std::unique_ptr<AsyncTask>> download_and_write_tasks;
	size_t count = m_market_list.size();

	// Download latest security data from internet and write the data into local database
	if (count > 0)
	{
		download_and_write_tasks.emplace_back(std::move(m_market_list[0]->GetParallelDownloadTask()));

		for (int i = 1; i < count; ++i)
		{
			download_and_write_tasks.emplace_back(std::move(std::make_unique<ParallelAsyncTask>(
				std::string(""),
				m_async_task_manager,
				std::move(m_market_list[i - 1]->GetSerialWriteTask()),
				std::move(m_market_list[i]->GetParallelDownloadTask()),
				[]() {}
			)));
		}

		download_and_write_tasks.emplace_back(std::move(m_market_list[count - 1]->GetSerialWriteTask()));
	}

	std::unique_ptr<AsyncTask> download_and_write_task = std::move(std::make_unique<SerialAsyncTask>(
		std::string(""),
		m_async_task_manager,
		std::move(download_and_write_tasks),
		[&]()
		{
			OnDownloadAndWriteCompleted();
		}
	));

	m_async_task_manager->AddTask(std::move(download_and_write_task));
}

void TradinatorCoreThread::OnDownloadAndWriteCompleted()
{
	std::vector<std::unique_ptr<AsyncTask>> tasks;

	for (std::shared_ptr<Market> market : m_market_list)
	{
		std::vector<std::unique_ptr<AsyncTask>> market_tasks = std::move(market->GetGenerateNewsPointsTask());

		tasks.reserve(tasks.size() + market_tasks.size());

		tasks.insert(tasks.end(), std::make_move_iterator(market_tasks.begin()), std::make_move_iterator(market_tasks.end()));
	}

	m_async_task_manager->AddTask(std::move(std::make_unique<ParallelAsyncTask>(
		std::string("Analysing all securities data for patterns and stratiges"),
		m_async_task_manager,
		std::move(tasks),
		[](){},
		TradinatorCoreSpace::Utils::GetMaxParallelAnalysis()
	)));
}

void TradinatorCoreThread::AddMarket(std::shared_ptr<Market>&& market)
{
	assert(!m_is_initialized && "Add markets before TradinatorCoreThread::Init is called.");
	std::shared_ptr<Market>& stored_market = m_market_list.emplace_back(market);
	stored_market->SetOwningTradinatorCoreThread(this->weak_from_this());
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
			"LatestCandleData INTEGER      NOT NULL," \
			"CandlesCount     INTEGER      NOT NULL); ");
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

const AsyncData<std::vector<std::weak_ptr<Security>>>& TradinatorCoreThread::GetTenNewestIPOs() const
{
	return m_market_list[0]->GetTenNewestIPOs();
}