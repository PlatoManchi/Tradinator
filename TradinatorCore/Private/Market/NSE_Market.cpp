#include "Market/NSE_Market.h"

#include <iostream>
#include <fstream>
#include <format>
#include <memory>
#include <map>

#include <curl/curl.h>

#include "TradinatorCore.h"
#include "Utils/AsyncTask.h"
#include "Utils/DownloadTask.h"
#include "Utils/ParallelAsyncTask.h"
#include "Utils/SerialAsyncTask.h"
#include "Utils/AsyncTaskManager.h"
#include "Utils/Utils.h"


static std::string _SRC_EQUITY_DATA_FILE_ = "EQUITY_L.csv";
static std::string _SRC_SME_EQUITY_DATA_FILE_ = "SME_EQUITY_L.csv";

static std::string _DST_EQUITY_DATA_FILE_ = "EQUITY_L.bin";
static std::string _DST_SME_EQUITY_DATA_FILE_ = "SME_EQUITY_L.bin";

NSE_Market::NSE_Market()
	: Market()
{
}

void NSE_Market::Init()
{
    CreateFolderStructure();
    GatherSymbols();
}

void NSE_Market::GatherSymbols()
{
    std::function<void()> process_raw_counter_data = [&]()
        {
            if (!IsRawFileExist())
            {
                std::cout << "ERROR: File containing symbols not found. Download from 'https://nsearchives.nseindia.com/content/securities/EQUITY_L.csv' and place the file at '" << GetRawDataFilePath()<<"'." << std::endl;
            }

            if (IsRawFileExist() /* && !IsProcessedFileExist()*/)
            {
                // Raw file exist but not processed.
                std::ifstream  raw_file(GetRawDataFilePath());
                std::ofstream  processed_file(GetProcessedDataFilePath(), std::ios::binary);

                

                std::string line;
                std::getline(raw_file, line); // first line is just headings so discard it
                
                // clean start
                m_securities_async_data.SetDataReady(false);
                //m_securities_list_loader.clear();
                
                int count = 0;

                // fill up the map
                while (std::getline(raw_file, line))
                {
                    Counter tmp_counter;
                    tmp_counter.FromString(line);
                    
                    std::shared_ptr<Counter> counter = std::make_shared<Counter>(tmp_counter);
                    counter->SetOwningMarket(this->weak_from_this());
                    counter->SetOwningTradinatorCoreThread(m_owning_tradinator_core_thread);

                    //m_securities_list_loader[tmp_counter.Symbol()] = counter;
                    m_securities_async_data.GetAsyncDataCopy()[tmp_counter.Symbol()] = counter;

                    //processed_file << tmp_counter; // do i really need to save processed data as binary data since its loaded once during startup
                }                
            }
            else if (IsProcessedFileExist())
            {
                std::ifstream  processed_file(GetProcessedDataFilePath());
            }
        };

    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    owning_tradinator_core_thread->GetAsyncTaskManager()->AddTask(std::make_unique<AsyncTask>(
        std::format("{} {}", "Gathering Symbols for", GetMarketName()),
        process_raw_counter_data,
        [&]()
        {
            OnGatherSymbolsCompleted();
        }
    ));
}

void NSE_Market::OnGatherSymbolsCompleted()
{
    // Since loading is finished, move loaded data into actual variable in main thread.
    //m_securities_list = std::move(m_securities_list_loader);
    m_securities_async_data.SetDataReady(true);
    //m_securities_list.begin()->second->LoadCounterData(std::bind(&NSE_Market::OnCounterDataLoaded, this));
    //m_securities_async_data.GetData().begin()->second->DownloadCounterData(std::bind(&NSE_Market::OnCounterDataLoaded, this));

    /*std::vector<std::unique_ptr<AsyncTask>> tasks;
    const std::map<std::string, std::shared_ptr<Counter>>& securities_list = m_securities_async_data.GetData();
    size_t count = 0;
    for (std::pair<std::string, std::shared_ptr<Counter>> pair : securities_list)
    {
        std::unique_ptr<AsyncTask> task = std::make_unique<AsyncTask>(
            pair.second->ISIN_Number(),
            [pair]() {
                pair.second->DownloadCounterData([]() {});
            },
            []() {}
        );

        tasks.push_back(std::move(task));

        count++;
        if (count == 100)
        {
            break;
        }
    }
    
    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    owning_tradinator_core_thread->GetAsyncTaskManager()->AddTask(std::make_unique<ParallelAsyncTask>(
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(tasks),
        std::bind(&NSE_Market::OnCounterDataLoaded, this),
        1
    ));*/

    std::vector<std::unique_ptr<AsyncTask>> tasks;
    for (int i = 0 ; i < 10 ; ++i)
    {
        std::unique_ptr<AsyncTask> task = std::make_unique<AsyncTask>(
            std::format("Serial Task : {}", i),
            [i]() {
                std::cout << "Serial Task ("<<i<<") - Body" << std::endl;
            },
            [i]() {
                std::cout << "Serial Task ("<<i<<") - Callback" << std::endl;
            }
        );

        tasks.push_back(std::move(task));
    }

    std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
    assert(owning_tradinator_core_thread);

    owning_tradinator_core_thread->GetAsyncTaskManager()->AddTask(std::make_unique<SerialAsyncTask>(
        owning_tradinator_core_thread->GetAsyncTaskManager(),
        std::move(tasks),
        [&]() {
            std::cout << "Serial Task Completed Callback" << std::endl;
        }
    ));
}

void NSE_Market::OnCounterDataLoaded()
{

}

bool NSE_Market::IsValid() const
{
    return IsRawFileExist() && IsProcessedFileExist();
}

bool NSE_Market::IsRawFileExist() const
{
    return Utils::DoesFileExist(GetRawDataFilePath());
}

bool NSE_Market::IsProcessedFileExist() const
{
    return Utils::DoesFileExist(GetProcessedDataFilePath());
}

std::string NSE_Market::GetRawDataFilePathName() const
{
    return _SRC_EQUITY_DATA_FILE_;
}

std::string NSE_Market::GetProcessedDataFileName() const
{
    return _DST_EQUITY_DATA_FILE_;
}
