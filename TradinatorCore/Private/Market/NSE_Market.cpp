#include "Market/NSE_Market.h"

#include <iostream>
#include <fstream>
#include <format>
#include <memory>
#include <map>

#include <curl/curl.h>

#include "SQLiteCpp/SQLiteCpp.h"

#include "TradinatorCore.h"
#include "Utils/AsyncTask.h"
#include "Utils/DownloadTask.h"
#include "Utils/ParallelAsyncTask.h"
#include "Utils/SerialAsyncTask.h"
#include "Utils/AsyncTaskManager.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"


static std::string _SRC_EQUITY_DATA_FILE_ = "EQUITY_L.csv";
static std::string _SRC_SME_EQUITY_DATA_FILE_ = "SME_EQUITY_L.csv";

static std::string _DST_EQUITY_DATA_FILE_ = "EQUITY_L.bin";
static std::string _DST_SME_EQUITY_DATA_FILE_ = "SME_EQUITY_L.bin";

NSE_Market::NSE_Market()
	: Market()
{
}

void NSE_Market::ParseSecurityListData()
{
    if (!IsRawFileExist())
    {
        std::string err_str = std::format("ERROR: File containing symbols not found. Download from 'https://nsearchives.nseindia.com/content/securities/EQUITY_L.csv' and place the file at '{}'.", GetSecurityListRawDataFilePath());
        Log::GetInstance().Write(err_str);
        std::cout << "ERROR: File containing symbols not found. Download from 'https://nsearchives.nseindia.com/content/securities/EQUITY_L.csv' and place the file at '" << GetSecurityListRawDataFilePath() << "'." << std::endl;
    }

    if (IsRawFileExist())
    {
        // Raw file exist but not processed.
        std::ifstream  raw_file(GetSecurityListRawDataFilePath());

        std::string line;
        std::getline(raw_file, line); // first line is just headings so discard it

        int count = 0;

        // fill up the map
        while (std::getline(raw_file, line))
        {
            std::shared_ptr<Security> security = std::make_shared<Security>();
            security->FromString(line);

            // We process only securities that are equity
            if (security->Series() == "EQ")
            {
                security->SetOwningMarket(this->weak_from_this());
                security->SetOwningTradinatorCoreThread(m_owning_tradinator_core_thread);

                m_securities_async_data.GetAsyncDataCopy()[security->Symbol()] = security;
            }
        }
    }

    bool is_success = false;
    while (!is_success)
    {
        try
        {
            SQLite::Database db(TradinatorCoreSpace::Utils::GetTradinatorDatabasePath());

            // Begin transaction
            std::string query_str = std::format("SELECT Symbol, LatestCandleData, CandlesCount FROM Securities;");
            SQLite::Statement query(db, query_str);

            while (query.executeStep())
            {
                std::string symbol = query.getColumn(0);

                std::chrono::system_clock::rep time_count = query.getColumn(1);
                std::chrono::system_clock::duration duration_since_epoch(time_count);
                std::chrono::system_clock::time_point time(duration_since_epoch);


                m_securities_async_data.GetAsyncDataCopy()[symbol]->SetCachedLatestCandleDate(time);
                m_securities_async_data.GetAsyncDataCopy()[symbol]->SetCandleCount(query.getColumn(2).getInt64());
            }

            is_success = true;
        }


        catch (std::exception&)
        {
            is_success = false;
            
            // Database might be locked by another thread. Wait for a bit and try again.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

bool NSE_Market::IsRawFileExist() const
{
    return TradinatorCoreSpace::Utils::DoesFileExist(GetSecurityListRawDataFilePath());
}

bool NSE_Market::IsProcessedFileExist() const
{
    return TradinatorCoreSpace::Utils::DoesFileExist(GetSecurityListProcessedDataFilePath());
}

std::string NSE_Market::GetSecurityListRawDataFileName() const
{
    return _SRC_EQUITY_DATA_FILE_;
}

std::string NSE_Market::GetSecurityListProcessedDataFileName() const
{
    return _DST_EQUITY_DATA_FILE_;
}
