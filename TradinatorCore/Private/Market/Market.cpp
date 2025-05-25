#include "Market/Market.h"

#include <filesystem>
#include <functional>
#include <iostream>

#include "json/json.h"

#include "TradinatorCore.h"
#include "Utils/AsyncTask.h"
#include "Utils/ParallelAsyncTask.h"
#include "Utils/SerialAsyncTask.h"
#include "Utils/DownloadTask.h"
#include "Utils/AsyncTaskManager.h"
#include "Utils/Utils.h"

Market::Market()
	: m_securities_async_data()
    , m_does_new_data_exist_to_download(false)
{

}




std::unique_ptr<AsyncTask> Market::GetGatherSecuritiesTask()
{
    CreateFolderStructure();

    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    std::function<void()> parse_security_data = std::bind(&Market::ParseSecurityListData, this);
    std::unique_ptr<AsyncTask> parse_security_data_task = std::make_unique<AsyncTask>(
        std::string("Parse securities data from local file"),
        [&]() 
        {
            m_securities_async_data.SetDataReady(false);
        },
        parse_security_data,
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
        std::format("Gathering security list for {}({}) market", GetMarketCode(), GetMarketName()),
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(parse_security_data_task),
        std::move(find_ten_newest_iops_task),
        []() { }
    );
}




void Market::FindTenNewestIPOs()
{
    m_ten_newest_securities.SetDataReady(false);

    const std::map<std::string, std::shared_ptr<Security>>& security_data = m_securities_async_data.GetData();

    int count = 0;
    for (const std::pair<std::string, std::shared_ptr<Security>>& pair : security_data)
    {
        if (count < 10)
        {
            count++;
            m_ten_newest_securities.GetAsyncDataCopy().push_back(pair.second);
        }
        else
        {
            std::function <bool(std::weak_ptr<Security>)> comparing_fun = [&](std::weak_ptr<Security> security)
                {
                    std::shared_ptr<Security> security_ptr = security.lock();
                    return pair.second->DateOfListing() > security_ptr->DateOfListing();
                };

            auto should_replace_itr = std::find_if(m_ten_newest_securities.GetAsyncDataCopy().begin(), m_ten_newest_securities.GetAsyncDataCopy().end(), comparing_fun);
            if (should_replace_itr != m_ten_newest_securities.GetAsyncDataCopy().end())
            {
                *should_replace_itr = pair.second;
            }
        }
    }

    m_ten_newest_securities.SetDataReady(true);
}




std::unique_ptr<AsyncTask> Market::GetParallelDownloadTask()
{
    std::vector<std::unique_ptr<AsyncTask>> download_tasks; // downloading latest candle data

    if (GetDoesNewDataExistToDownload())
    {
        const  std::map<std::string, std::shared_ptr<Security>>& securities_list = m_securities_async_data.GetData();

        for (std::pair<std::string, std::shared_ptr<Security>> pair : securities_list)
        {
            if (pair.second->IsHistoricalCandleDataOutDated())
            {
                //download_tasks.push_back(std::move(pair.second->GetDownloadLatestCandleDataTask()));
            }
        }
    }
    

    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    return std::make_unique<ParallelAsyncTask>(
        std::format("Downloading latest candle data for {}({}) Market", GetMarketCode(), GetMarketName()),
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(download_tasks),
        [&]() 
        {
            m_does_new_data_exist_to_download = false;
        }
        , TradinatorCoreSpace::Utils::GetMaxParallelDownloads()
    );
}




std::unique_ptr<AsyncTask> Market::GetSerialWriteTask()
{
    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    std::vector<std::unique_ptr<AsyncTask>> read_write_tasks;

    if (GetDoesNewDataExistToDownload())
    {
        const  std::map<std::string, std::shared_ptr<Security>>& securities_list = m_securities_async_data.GetData();

        const size_t count = securities_list.size();
        const size_t batch_size = TradinatorCoreSpace::Utils::GetReadWriteBatchSize();
        const size_t total_batch_count = count / batch_size + (count % batch_size == 0 ? 0 : 1);

        auto itr = securities_list.begin();

        std::vector<std::function<void()>> batch_serial_inserts;
        batch_serial_inserts.reserve(batch_size);
        // while parallel reading current batch, insert into db for previous batch
        // there by doing reading and writting in parallel
        for (size_t batch = 1; batch <= total_batch_count; ++batch)
        {
            size_t items_in_batch = batch_size * batch < count ? batch_size : count - batch_size * (batch - 1);

            std::vector<std::unique_ptr<AsyncTask>> batch_parallel_read_task;
            batch_parallel_read_task.reserve(batch_size);

            std::vector<std::function<void()>> prev_batch_serial_inserts;
            prev_batch_serial_inserts.insert(prev_batch_serial_inserts.end(),
                std::make_move_iterator(batch_serial_inserts.begin()), std::make_move_iterator(batch_serial_inserts.end()));

            batch_serial_inserts = std::vector<std::function<void()>>();
            batch_serial_inserts.reserve(batch_size);

            for (int i = 0; i < items_in_batch; ++i)
            {
                batch_parallel_read_task.push_back(std::move(std::make_unique<AsyncTask>(
                    std::string(""),
                    std::bind(&Security::ReadFromRawFileToMemory, (*itr).second),
                    std::bind(&Security::AnalyzeDownloadedData, (*itr).second),
                    []() {}
                )));

                batch_serial_inserts.push_back(std::bind(&Security::InsertRawDataToDatabase, (*itr).second));

                std::advance(itr, 1);
            }

            if (batch == 1)
            {
                read_write_tasks.push_back(std::move(std::make_unique<ParallelAsyncTask>(
                    std::format("", batch),
                    owning_tradinator_core_thread->GetAsyncTaskManager(),
                    std::move(batch_parallel_read_task),
                    []() {}
                )));
            }
            else
            {
                std::vector<std::unique_ptr<AsyncTask>> tmp;
                tmp.push_back(std::move(std::make_unique<ParallelAsyncTask>(
                    std::format("", batch),
                    owning_tradinator_core_thread->GetAsyncTaskManager(),
                    std::move(batch_parallel_read_task),
                    []() {}
                )));
                tmp.push_back(std::move(std::make_unique<AsyncTask>(
                    std::format("", batch - 1),
                    std::move(prev_batch_serial_inserts),
                    std::function<void()>([]() {})
                )));

                read_write_tasks.push_back(std::move(std::make_unique<ParallelAsyncTask>(
                    std::string(""),
                    owning_tradinator_core_thread->GetAsyncTaskManager(),
                    std::move(tmp),
                    []() {}
                )));
            }

            if (batch == total_batch_count)
            {
                read_write_tasks.push_back(std::move(std::make_unique<AsyncTask>(
                    std::format("", batch),
                    std::move(batch_serial_inserts),
                    std::function<void()>([]() {})
                )));
            }
        }
    }
    


    return std::make_unique<SerialAsyncTask>(
        std::format("Inserting latest candle data for {}({}) Market to local database", GetMarketCode(), GetMarketName()),
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(read_write_tasks),
        //[]() {}
        std::function<void()>([&]() 
            {
                m_does_new_data_exist_to_download = false;
            })
    );
}


std::shared_ptr<Security> Market::GetSecurity(std::string symbol) const
{
    if (m_securities_async_data.IsDataReady())
    {
        auto itr = m_securities_async_data.GetData().find(symbol);
        if (itr != m_securities_async_data.GetData().end())
        {
            return (*itr).second;
        }
    }

    return nullptr;
}




std::vector<std::unique_ptr<AsyncTask>> Market::GetGenerateNewsPointsTask()
{
    std::vector<std::unique_ptr<AsyncTask>> result;

    const  std::map<std::string, std::shared_ptr<Security>>& securities_list = m_securities_async_data.GetData();
    int count = 0;
    for (std::pair<std::string, std::shared_ptr<Security>> pair : securities_list)
    {
        count++;
        if (count > 10)
            break;

        result.emplace_back(std::move(pair.second->GetGenerateNewsPointsTask()));
        
    }

    return result;
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

std::string Market::GetSecurityListRawDataFilePath() const
{
	return TradinatorCoreSpace::Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + GetSecurityListRawDataFileName();
}

std::string Market::GetSecurityListProcessedDataFilePath() const
{
	return TradinatorCoreSpace::Utils::GetTradinatorWorkingFolderPath() + "/" + GetMarketCode() + "/" + GetSecurityListProcessedDataFileName();
}