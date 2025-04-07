#include "Market/NSE_Market.h"

#include <iostream>
#include <fstream>
#include <format>

#include <curl/curl.h>

#include "TradinatorCore.h"
#include "Utils/AsyncTask.h"
#include "Utils/DownloadTask.h"
#include "Utils/Utils.h"


static std::string _SRC_EQUITY_DATA_FILE_ = "EQUITY_L.csv";
static std::string _SRC_SME_EQUITY_DATA_FILE_ = "SME_EQUITY_L.csv";

static std::string _DST_EQUITY_DATA_FILE_ = "EQUITY_L.bin";
static std::string _DST_SME_EQUITY_DATA_FILE_ = "SME_EQUITY_L.bin";

NSE_Market::NSE_Market(std::shared_ptr<TradinatorCore> tradinator_core)
	: Market(tradinator_core)
{

}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, FILE* userp)
{
    size_t written = fwrite(contents, size, nmemb, userp);
    std::cout << "Written : " << written << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // return -1 to cancel the download
    return written;
}

void NSE_Market::GatherSymbols()
{
    std::function<void()> process_raw_equity_data = [&]()
        {
            if (IsRawFileExist() && !IsProcessedFileExist())
            {
                CreateFolderStructure();

                // Raw file exist but not processed.
                std::ifstream  raw_file(GetRawDataFolder() + "/" + _SRC_EQUITY_DATA_FILE_);
                std::ofstream  processed_file(GetProcessedDataFolder() + "/" + _DST_EQUITY_DATA_FILE_, std::ios::binary);

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
                    m_equities_list_loader[tmp_equity.symbol()] = tmp_equity;
                    //processed_file << tmp_equity; // do i really need to save processed data as binary data since its loaded once during startup
                }                
            }
            else if (IsProcessedFileExist())
            {
                std::ifstream  processed_file(GetProcessedDataFolder() + "/" + _DST_EQUITY_DATA_FILE_);
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
            function();
        },
        url, "Data/Raw/example2.txt"
    ));*/

}

void NSE_Market::OnGatherSymbolsCompleted()
{
    // Since loading is finished, move loaded data into actual variable in main thread.
    m_equities_list = std::move(m_equities_list_loader);
}

bool NSE_Market::IsValid() const
{
    return IsRawFileExist() && IsProcessedFileExist();
}

bool NSE_Market::IsRawFileExist() const
{
    std::string raw_file_path = GetRawDataFolder() + "/" + _SRC_EQUITY_DATA_FILE_;
    return Utils::DoesFileExist(raw_file_path);
}

bool NSE_Market::IsProcessedFileExist() const
{
    std::string processed_file_path = GetProcessedDataFolder() + "/" + _DST_EQUITY_DATA_FILE_;
    return Utils::DoesFileExist(processed_file_path);
}

void NSE_Market::function()
{
    std::cout << "callback Main Thread ? " << std::endl;
}