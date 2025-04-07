#include "Market/NSE_Market.h"

#include <iostream>
#include <fstream>
#include <format>

#include <curl/curl.h>

#include "TradinatorCore.h"
#include "Utils/AsyncTask.h"
#include "Utils/DownloadTask.h"
#include "Utils/ThreadManager.h"
#include "Utils/Utils.h"


static std::string _SRC_EQUITY_DATA_FILE_ = "EQUITY_L.csv";
static std::string _SRC_SME_EQUITY_DATA_FILE_ = "SME_EQUITY_L.csv";

static std::string _DST_EQUITY_DATA_FILE_ = "EQUITY_L.bin";
static std::string _DST_SME_EQUITY_DATA_FILE_ = "SME_EQUITY_L.bin";

NSE_Market::NSE_Market(std::shared_ptr<TradinatorCore> tradinator_core)
	: Market(tradinator_core)
{
    CreateFolderStructure();
}


void NSE_Market::GatherSymbols()
{
    std::function<void()> process_raw_equity_data = [&]()
        {
            if (!IsRawFileExist())
            {
                std::cout << "ERROR: File containing symbols not found. Download from 'https://nsearchives.nseindia.com/content/equities/EQUITY_L.csv' and place the file at '" << GetRawDataFilePath()<<"'." << std::endl;
            }

            if (IsRawFileExist() /* && !IsProcessedFileExist()*/)
            {
                // Raw file exist but not processed.
                std::ifstream  raw_file(GetRawDataFilePath());
                std::ofstream  processed_file(GetProcessedDataFilePath(), std::ios::binary);

                Equity tmp_equity;

                std::string line;
                std::getline(raw_file, line); // first line is just headings so discard it
                
                // clean start
                m_equities_list_loader.clear();

                int count = 0;

                // fill up the map
                while (std::getline(raw_file, line))
                {
                    tmp_equity.FromString(line);
                    
                    std::shared_ptr<Equity> equity = std::make_shared<Equity>(tmp_equity);
                    equity->SetParentMarket(GetMarket());

                    m_equities_list_loader[tmp_equity.symbol()] = equity;
                    //processed_file << tmp_equity; // do i really need to save processed data as binary data since its loaded once during startup
                }                
            }
            else if (IsProcessedFileExist())
            {
                std::ifstream  processed_file(GetProcessedDataFilePath());
            }
        };

    m_tradinator_core->GetThreadManager()->AddTask(std::make_unique<AsyncTask>(
        std::format("{} {}", "Gathering Symbols for", GetMarketName()),
        [&]()
        {
            OnGatherSymbolsCompleted();
        },
        process_raw_equity_data
    ));

    


    /*std::string url =
        "https://api.upstox.com/v2/historical-candle/NSE_EQ|INE696F01016/day/2025-04-06/2025-04-01";

    m_tradinator_core->GetThreadManager()->AddTask(std::make_unique<DownloadTask>(
        [&]() {
            // callback
        },
        url, "Data/Raw/example2.txt"
    ));*/

}

void NSE_Market::OnGatherSymbolsCompleted()
{
    // Since loading is finished, move loaded data into actual variable in main thread.
    m_equities_list = std::move(m_equities_list_loader);
    m_equities_list.begin()->second->LoadEquityData(std::bind(&NSE_Market::OnEquityDataLoaded, this));
}

void NSE_Market::OnEquityDataLoaded()
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
