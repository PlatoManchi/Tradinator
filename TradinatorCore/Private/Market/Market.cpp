#include "Market/Market.h"

#include <filesystem>
#include <functional>
#include <iostream>

#include "TradinatorCore.h"
#include "Utils/AsyncTask.h"
#include "Utils/ParallelAsyncTask.h"
#include "Utils/SerialAsyncTask.h"
#include "Utils/AsyncTaskManager.h"
#include "Utils/Utils.h"

Market::Market()
	: m_securities_async_data()
{

}

void Market::Init()
{
	CreateFolderStructure();

	std::function<void()> parse_counter_data = std::bind(&Market::ParseCounterListData, this);
	std::function<void()> on_parse_counter_data_completed = std::bind(&Market::OnParseCounterListCompleted, this);

	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	m_securities_async_data.SetDataReady(false);

	owning_tradinator_core_thread->GetAsyncTaskManager()->AddTask(std::make_unique<AsyncTask>(
		std::format("Gathering counter list for {}({}) market", GetMarketCode(), GetMarketName()),
		parse_counter_data,
		on_parse_counter_data_completed
	));
}

void Market::OnParseCounterListCompleted()
{
    m_securities_async_data.SetDataReady(true);

    std::vector<std::unique_ptr<AsyncTask>> download_tasks; // downloading latest candle data
    std::vector<std::unique_ptr<AsyncTask>> writting_tasks; // writting downloaded data into db
    const  std::map<std::string, std::shared_ptr<Counter>>& securities_list = m_securities_async_data.GetData();
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

    for (std::pair<std::string, std::shared_ptr<Counter>> pair : securities_list)
    {
        if (pair.second->IsHistoricalCandleDataOutDated())
        {
            download_tasks.push_back(std::move(pair.second->GetDownloadLatestCandleDataTask([]() {})));
            writting_tasks.push_back(std::move(std::make_unique<AsyncTask>(
                std::format("Inserting latest candle data for {} into database", pair.second->Symbol()),
                std::bind(&Counter::InsertRawDataToDatabase, pair.second),
                []() {}
            )));
        }
    }
    
    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    std::unique_ptr<AsyncTask> parallel_download = std::make_unique<ParallelAsyncTask>(
        std::format("Downloading latest candle data for {}({}) Market", GetMarketCode(), GetMarketName()),
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(download_tasks),
        []()
        { }
        , 100 // Magic number. Don't want to be considered as a DDoS attack by server. 
    );

    // Writting data into SQLiteCpp database needs to be serial. Otherwise task has to wait for other task to release 
    // the database before it can insert it's data. 
    std::unique_ptr<AsyncTask> serial_write = std::make_unique<SerialAsyncTask>(
        std::format("Inserting latest candle data for {}({}) Market to local database", GetMarketCode(), GetMarketName()),
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(writting_tasks),
        []()
        {
        }
    );

    std::vector<std::unique_ptr<AsyncTask>> download_and_write_serial_tasks;
    download_and_write_serial_tasks.push_back(std::move(parallel_download));
    download_and_write_serial_tasks.push_back(std::move(serial_write));

    owning_tradinator_core_thread->GetAsyncTaskManager()->AddTask(
        std::make_unique<SerialAsyncTask>(
            std::format("---------------- Updating candle data for {}({}) Market ------------", GetMarketCode(), GetMarketName()),
            owning_tradinator_core_thread->GetAsyncTaskManager(),
            std::move(download_and_write_serial_tasks),
            []()
            { }
        )
    );
}

void Market::CreateFolderStructure() const
{
	std::filesystem::create_directory(Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode());
	std::filesystem::create_directory(GetRawDataFolderPath());
	std::filesystem::create_directory(GetProcessedDataFolderPath());
}

std::string Market::GetRawDataFolderPath() const
{
	return Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + _RAW_DATA_FOLDER_;
}

std::string Market::GetProcessedDataFolderPath() const
{
	return Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + _PROCESSED_DATA_FOLDER_;
}

std::string Market::GetCounterListRawDataFilePath() const
{
	return Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + GetCounterListRawDataFileName();
}

std::string Market::GetCounterListProcessedDataFilePath() const
{
	return Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + GetCounterListProcessedDataFileName();
}