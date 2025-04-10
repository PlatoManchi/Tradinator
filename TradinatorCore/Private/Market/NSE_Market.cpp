#include "Market/NSE_Market.h"

#include <iostream>
#include <fstream>
#include <format>
#include <memory>

#include <curl/curl.h>

#include "TradinatorCore.h"
#include "Utils/AsyncTask.h"
#include "Utils/DownloadTask.h"
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
    std::function<void()> process_raw_security_data = [&]()
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

                Security tmp_security;

                std::string line;
                std::getline(raw_file, line); // first line is just headings so discard it
                
                // clean start
                m_securities_async_data.SetDataReady(false);
                //m_securities_list_loader.clear();
                
                int count = 0;

                // fill up the map
                while (std::getline(raw_file, line))
                {
                    tmp_security.FromString(line);
                    
                    std::shared_ptr<Security> security = std::make_shared<Security>(tmp_security);
                    security->SetOwningMarket(this->weak_from_this());
                    security->SetOwningTradinatorCoreThread(m_owning_tradinator_core_thread);

                    //m_securities_list_loader[tmp_security.Symbol()] = security;
                    m_securities_async_data.GetAsyncDataCopy()[tmp_security.Symbol()] = security;

                    //processed_file << tmp_security; // do i really need to save processed data as binary data since its loaded once during startup
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
        process_raw_security_data,
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
    //m_securities_list.begin()->second->LoadSecurityData(std::bind(&NSE_Market::OnSecurityDataLoaded, this));
    m_securities_async_data.GetData().begin()->second->DownloadSecurityData(std::bind(&NSE_Market::OnSecurityDataLoaded, this));
}

void NSE_Market::OnSecurityDataLoaded()
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
