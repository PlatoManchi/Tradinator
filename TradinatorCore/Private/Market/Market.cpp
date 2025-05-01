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

std::unique_ptr<AsyncTask> Market::GetGatherSecuritiesTask()
{
    CreateFolderStructure();

    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    std::function<void()> parse_counter_data = std::bind(&Market::ParseCounterListData, this);
    std::unique_ptr<AsyncTask> parse_counter_data_task = std::make_unique<AsyncTask>(
        std::string("Parse securities data from local file"),
        [&]() 
        {
            m_securities_async_data.SetDataReady(false);
        },
        parse_counter_data,
        [&]()
        {
            m_securities_async_data.SetDataReady(true);
        }
    );

    std::unique_ptr<AsyncTask> find_ten_newest_iops_task = std::make_unique<AsyncTask>(
        std::format("Finding Ten Newest IPOs for {}({}) market", GetMarketCode(), GetMarketName()),
        std::bind(&Market::FindTenNewestIPOs, this),
        []() {}
    );


    
    return std::make_unique<SerialAsyncTask>(
        std::format("Gathering counter list for {}({}) market", GetMarketCode(), GetMarketName()),
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(parse_counter_data_task),
        std::move(find_ten_newest_iops_task),
        []() { }
    );
}

void Market::FindTenNewestIPOs()
{
    m_ten_newest_counters.SetDataReady(false);

    const std::map<std::string, std::shared_ptr<Counter>>& security_data = m_securities_async_data.GetData();

    int count = 0;
    for (const std::pair<std::string, std::shared_ptr<Counter>>& pair : security_data)
    {
        if (count < 10)
        {
            count++;
            m_ten_newest_counters.GetAsyncDataCopy().push_back(pair.second);
        }
        else
        {
            std::function <bool(std::weak_ptr<Counter>)> comparing_fun = [&](std::weak_ptr<Counter> counter)
                {
                    std::shared_ptr<Counter> counter_ptr = counter.lock();
                    return pair.second->DateOfListing() > counter_ptr->DateOfListing();
                };

            auto should_replace_itr = std::find_if(m_ten_newest_counters.GetAsyncDataCopy().begin(), m_ten_newest_counters.GetAsyncDataCopy().end(), comparing_fun);
            if (should_replace_itr != m_ten_newest_counters.GetAsyncDataCopy().end())
            {
                *should_replace_itr = pair.second;
            }
        }
    }

    m_ten_newest_counters.SetDataReady(true);
}

std::unique_ptr<AsyncTask> Market::GetParallelDownloadTask()
{
    std::vector<std::unique_ptr<AsyncTask>> download_tasks; // downloading latest candle data

    const  std::map<std::string, std::shared_ptr<Counter>>& securities_list = m_securities_async_data.GetData();
    
    for (std::pair<std::string, std::shared_ptr<Counter>> pair : securities_list)
    {
        if (pair.second->IsHistoricalCandleDataOutDated())
        {
            //download_tasks.push_back(std::move(pair.second->GetDownloadLatestCandleDataTask([]() {})));
        }
    }

    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    return std::make_unique<ParallelAsyncTask>(
        std::format("Downloading latest candle data for {}({}) Market", GetMarketCode(), GetMarketName()),
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(download_tasks),
        []() {}
        , 100 // Magic number. Don't want to be considered as a DDoS attack by server. 
    );
}

std::unique_ptr<AsyncTask> Market::GetSerialWriteTask()
{
    std::vector<std::unique_ptr<AsyncTask>> writting_tasks; // writting downloaded data into db
    const  std::map<std::string, std::shared_ptr<Counter>>& securities_list = m_securities_async_data.GetData();

    for (std::pair<std::string, std::shared_ptr<Counter>> pair : securities_list)
    {
        if (pair.second->IsHistoricalCandleDataOutDated())
        {
            /*writting_tasks.push_back(std::move(std::make_unique<AsyncTask>(
                std::format("Inserting latest candle data for {} into database", pair.second->Symbol()),
                std::bind(&Counter::InsertRawDataToDatabase, pair.second),
                []() {}
            )));*/
        }
    }

    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    // Writting data into SQLiteCpp database needs to be serial. Otherwise task has to wait for other task to release 
    // the database before it can insert it's data. 
    return std::make_unique<SerialAsyncTask>(
        std::format("Inserting latest candle data for {}({}) Market to local database", GetMarketCode(), GetMarketName()),
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(writting_tasks),
        []() { }
    );
}

void Market::CreateFolderStructure() const
{
	std::filesystem::create_directory(TradinatorCoreSpace::Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode());
	std::filesystem::create_directory(GetRawDataFolderPath());
	std::filesystem::create_directory(GetProcessedDataFolderPath());
}

std::string Market::GetRawDataFolderPath() const
{
	return TradinatorCoreSpace::Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + _RAW_DATA_FOLDER_;
}

std::string Market::GetProcessedDataFolderPath() const
{
	return TradinatorCoreSpace::Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + _PROCESSED_DATA_FOLDER_;
}

std::string Market::GetCounterListRawDataFilePath() const
{
	return TradinatorCoreSpace::Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + GetCounterListRawDataFileName();
}

std::string Market::GetCounterListProcessedDataFilePath() const
{
	return TradinatorCoreSpace::Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + GetCounterListProcessedDataFileName();
}