#include "Data/Security.h"

#include <iostream>
#include <format>
#include <fstream>
#include <sstream>
#include <chrono>



#include "SQLiteCpp/SQLiteCpp.h"

#include "TradinatorCore.h"
#include "Data/Candle.h"
#include "Market/Market.h"
#include "Utils/AsyncTaskManager.h"
#include "Utils/AsyncTask.h"
#include "Utils/DownloadTask.h"
#include "Utils/SerialAsyncTask.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"

static std::string _STATUS_ = "status";
static std::string _SUCCESS_ = "success";

static std::string _DATA_ = "data";
static std::string _CANDLES_ = "candles";


Security::Security()
	: Company()
	, m_series()
	, m_paid_up_value(0)
	, m_market_lot(0)
	, m_face_value(0)
	, m_candle_count(0)
	, m_database_connection(TradinatorCoreSpace::Utils::GetTradinatorDatabasePath())
	, m_candles_data(std::make_shared<AsyncData<CandlesData>>())
	, m_news_points_data(std::make_shared<AsyncData<NewsPointVectorType>>())
	, m_is_downloading(false)
	, m_is_inserting(false)
	, m_is_latest_date_dirty(true)
	, m_is_memory_in_sync(false)
	, m_lock_in_memory(false)
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::days delta_time(50 * 365); // 50 years

	m_cached_latest_candle_date = now - delta_time;
}



std::string Security::GetProcessedHistoricalDataFilePath() const
{
	std::shared_ptr<Market> owning_market = m_owning_market.lock();
	assert(owning_market);

	std::string folder_path = owning_market->GetProcessedDataFolderPath();
	return folder_path + "/" + m_symbol + ".bin";
}






bool Security::DoesProcessedHistoricalDataExist() const
{
	return m_database_connection.tableExists(GetTableName());
}







std::chrono::system_clock::time_point Security::GetLastCandleDataDate() const
{
	if (!m_is_latest_date_dirty)
		return m_cached_latest_candle_date;

	
	if (DoesProcessedHistoricalDataExist())
	{
		SQLite::Statement query(m_database_connection, std::format("SELECT \"LatestCandleData\" FROM Securities WHERE ISIN=\"{}\"", ISIN_Number()));
		if (query.executeStep())
		{
			std::chrono::system_clock::rep time_count = query.getColumn(0);
			std::chrono::system_clock::duration duration_since_epoch(time_count);

			// cache
			m_is_latest_date_dirty = false;
			m_cached_latest_candle_date = std::chrono::system_clock::time_point(duration_since_epoch);

			return m_cached_latest_candle_date;
		}
	}

	

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::days delta_time(50 * 365); // 50 years

	return now - delta_time;
}







bool Security::IsHistoricalCandleDataOutDated() const
{
	std::chrono::system_clock::time_point to_tp = std::chrono::system_clock::now();

	// add an extra day because if candle data is available will 13th then we need data from 14th
	// don;t need 13th data again
	std::chrono::system_clock::time_point from_tp = GetLastCandleDataDate() + std::chrono::days(1);

	if (to_tp <= (from_tp + std::chrono::days(1)))
	{
		// if to is not more than a day of from, historical data is up to date
		return false;
	}

	return true;
}








std::unique_ptr<AsyncTask> Security::GetDownloadLatestCandleDataTask()
{
	std::chrono::system_clock::time_point to_tp = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point from_tp = GetLastCandleDataDate() + std::chrono::days(1);

	
	std::shared_ptr<Market> owning_market = m_owning_market.lock();
	assert(owning_market);

	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	std::string to = std::format("{:%F}", to_tp);
	std::string from = std::format("{:%F}", from_tp);

	/* eg url format
	https://api.upstox.com/v2/historical-candle/NSE_EQ|INE696F01016/day/2025-04-06/2025-04-01*/
	std::string url = std::format("https://api.upstox.com/v2/historical-candle/{}_{}|{}/day/{}/{}"
		, owning_market->GetMarketCode()
		, m_series
		, m_isin_number
		, to
		, from);

	
	// Set the security to updating state
	std::unique_ptr<AsyncTask> set_updating_true = std::make_unique<AsyncTask>(
		std::string(""),
		[&]()
		{
			std::lock_guard<std::mutex> lock(m_security_mutex);

			m_is_downloading = true;
		},
		[]() {}
	);

	std::string tmp_file_path = owning_market->GetRawDataFolderPath() + "/" + std::format("{}.json", Symbol());
	std::unique_ptr<AsyncTask> download_task = std::make_unique<DownloadTask>([](){}, url, tmp_file_path);

	std::vector<std::unique_ptr<AsyncTask>> tasks;
	if (!m_is_downloading)
	{
		tasks.push_back(std::move(set_updating_true));
		tasks.push_back(std::move(download_task));
	}

	return std::make_unique<SerialAsyncTask>(
		std::string(""), 
		owning_tradinator_core_thread->GetAsyncTaskManager(),
		std::move(tasks),
		[&]() { 
			std::lock_guard<std::mutex> lock(m_security_mutex); 
			m_is_downloading = false; 
		});
}







void Security::ReadFromRawFileToMemory()
{
	std::shared_ptr<Market> owning_market = m_owning_market.lock();
	assert(owning_market);

	std::string tmp_file_path = owning_market->GetRawDataFolderPath() + "/" + std::format("{}.json", Symbol());

	std::ifstream downloaded_tmp_file(tmp_file_path);
	downloaded_tmp_file >> m_raw_downloaded_data;
}








void Security::InsertRawDataToDatabase()
{
	Log::GetInstance().Write(std::format("Inserting: {}", Name()));

	if (m_is_downloading || m_is_inserting) return;
	{
		std::lock_guard<std::mutex> lock(m_security_mutex);
		m_is_inserting = true;
	}
	

	Json::Value json_candles = m_raw_downloaded_data[_DATA_][_CANDLES_];
	Json::ArrayIndex count = json_candles.size();
	if (count > 0)
	{
		bool is_success = false;
		while (!is_success)
		{
			try
			{
				SQLite::Database db(TradinatorCoreSpace::Utils::GetTradinatorDatabasePath(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

				if (!DoesProcessedHistoricalDataExist())
				{
					// table that contains daily candle data for this security

					/*
					*	std::chrono::system_clock::time_point m_date;
						double m_open;
						double m_high;
						double m_low;
						double m_close;
						int64_t m_volume;
						int64_t m_open_interest;
					*/
					std::string query_str = std::format("CREATE TABLE IF NOT EXISTS \"{}\" ("  \
						"Date     INTEGER PRIMARY KEY     NOT NULL," \
						"Open             REAL         NOT NULL," \
						"High             REAL         NOT NULL," \
						"Low              REAL         NOT NULL," \
						"Close            REAL         NOT NULL," \
						"Volume           INTEGER      NOT NULL," \
						"OpenInterest     INTEGER      NOT NULL);", GetTableName());


					SQLite::Transaction transaction(db);
					db.exec(query_str);
					transaction.commit();
				}

				// Get the skeleton data for Security
				{
					std::string query_str = std::format("SELECT LatestCandleData, CandlesCount FROM Securities WHERE ISIN=\"{}\"", ISIN_Number());
					SQLite::Statement query(m_database_connection, query_str);
					if (query.executeStep())
					{
						std::chrono::system_clock::rep time_count = query.getColumn(0);
						std::chrono::system_clock::duration duration_since_epoch(time_count);

						// cache
						m_is_latest_date_dirty = false;
						m_cached_latest_candle_date = std::chrono::system_clock::time_point(duration_since_epoch);

						m_candle_count = query.getColumn(1).getInt64();
					}
				}

				m_candle_count += count;

				SQLite::Transaction transaction(db);
				// Prepare the insert statement once
				SQLite::Statement insert(db, std::format("INSERT OR IGNORE INTO \"{}\" (Date, Open, High, Low, Close, Volume, OpenInterest) VALUES "  \
					"(?, ?, ?, ?, ?, ?, ?)", GetTableName()));

				std::chrono::system_clock::time_point tmp_latest_candle_date = m_cached_latest_candle_date;

				for (Json::Value& candle : json_candles)
				{
					std::string date_str = candle[0].asCString();
					std::istringstream is{ date_str };
					std::chrono::system_clock::time_point date;
					is >> std::chrono::parse("%F", date);

					// Only process new data
					if (date > tmp_latest_candle_date)
					{
						if (date > m_cached_latest_candle_date)
						{
							m_cached_latest_candle_date = date;
						}

						// one candle from the data has negative value for volumes and open interest for some reason and this 
						// is for that one random wrong value
						int64_t volume = candle[5].asInt64();
						if (volume < 0)
						{
							volume = 0;
						}
						int64_t open_interest = candle[6].asInt64();
						if (open_interest < 0)
						{
							open_interest = 0;
						}

						insert.bind(1, date.time_since_epoch().count());
						insert.bind(2, candle[1].asDouble());
						insert.bind(3, candle[2].asDouble());
						insert.bind(4, candle[3].asDouble());
						insert.bind(5, candle[4].asDouble());
						insert.bind(6, volume);
						insert.bind(7, open_interest);

						insert.exec();                // execute insert
						insert.reset();               // reset statement for next use
						insert.clearBindings();       // clear bound values
					}
				}
				transaction.commit();

				UpdateSecuritySkeletonData();

				m_is_memory_in_sync = false;
				is_success = true;
			}
			catch (std::exception& e)
			{
				is_success = false;
				Log::GetInstance().Write(std::format("ERROR: SQLite exception: {}", e.what()));
				
				// Database might be locked by another thread. Wait for a bit and try again.
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}
	
	// Unload the raw data. We never need it again
	m_raw_downloaded_data = std::move(Json::Value());

	{
		std::lock_guard<std::mutex> lock(m_security_mutex);
		m_is_inserting = false;
	}
}






void Security::LoadCandleDataToMemory()
{
	if (m_is_memory_in_sync)
	{
		return;
	}

	if (!DoesProcessedHistoricalDataExist())
	{
		m_candles_data->SetDataReady(true);
		return;
	}

	bool is_success = false;
	while (!is_success)
	{
		try
		{
			if (m_candles_data->IsDataReady())
			{
				m_candles_data->SetDataReady(false);
			}

			m_candles_data->GetAsyncDataCopy().m_dates.reserve(m_candle_count);
			m_candles_data->GetAsyncDataCopy().m_opens.reserve(m_candle_count);
			m_candles_data->GetAsyncDataCopy().m_highs.reserve(m_candle_count);
			m_candles_data->GetAsyncDataCopy().m_lows.reserve(m_candle_count);
			m_candles_data->GetAsyncDataCopy().m_closes.reserve(m_candle_count);
			m_candles_data->GetAsyncDataCopy().m_volumes.reserve(m_candle_count);
			m_candles_data->GetAsyncDataCopy().m_open_interests.reserve(m_candle_count);

			std::string query_str = std::format("SELECT * FROM \"{}\" ORDER BY Date ASC", GetTableName());
			SQLite::Statement query(m_database_connection, query_str);
			
			while (query.executeStep())
			{
				// Date, Open, High, Low, Close, Volume, OpenInterest
				std::chrono::system_clock::rep time_count = query.getColumn(0);
				std::chrono::system_clock::duration duration_since_epoch(time_count);
				std::chrono::system_clock::time_point date(duration_since_epoch);

				
				m_candles_data->GetAsyncDataCopy().m_dates.push_back(date);
				m_candles_data->GetAsyncDataCopy().m_opens.push_back(query.getColumn(1));
				m_candles_data->GetAsyncDataCopy().m_highs.push_back(query.getColumn(2));
				m_candles_data->GetAsyncDataCopy().m_lows.push_back(query.getColumn(3));
				m_candles_data->GetAsyncDataCopy().m_closes.push_back(query.getColumn(4));
				m_candles_data->GetAsyncDataCopy().m_volumes.push_back(query.getColumn(5).getInt64());
				m_candles_data->GetAsyncDataCopy().m_open_interests.push_back(query.getColumn(6).getInt64());
			}

			m_candles_data->SetDataReady(true);

			m_is_memory_in_sync = true;
			is_success = true;
		}
		catch (std::exception& e)
		{
			is_success = false;
			Log::GetInstance().Write(std::format("ERROR: SQLite exception: {}", e.what()));

			// Database might be locked by another thread. Wait for a bit and try again.
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}






void Security::LoadCandleDataToMemoryAsync()
{
	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	std::function<void()> load_candle_data = std::bind(&Security::LoadCandleDataToMemory, this);

	owning_tradinator_core_thread->GetAsyncTaskManager()->AddTask(std::move(std::make_unique<AsyncTask>(
		std::format("Loading candle data for {}", m_symbol),
		load_candle_data,
		[]() {}
	)));
}






void Security::UnloadCandleDataFromMemory()
{
	if (!m_lock_in_memory)
	{
		m_is_memory_in_sync = false;
		m_candles_data->Reset();
	}
}






void Security::UpdateSecuritySkeletonData()
{
	bool is_success = false;
	while (!is_success)
	{
		try
		{
			SQLite::Database db(TradinatorCoreSpace::Utils::GetTradinatorDatabasePath(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

			// Begin transaction
			SQLite::Transaction transaction(db);
			std::string query2 = std::format("INSERT OR REPLACE INTO Securities (ISIN, Symbol, Name, Series, DateOfListing, PaidUpValue, MarketLot, FaceValue, LatestCandleData, CandlesCount)\
				 VALUES (\"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\");", 
				ISIN_Number(), 
				Symbol(), 
				Name(), 
				Series(), 
				DateOfListing().time_since_epoch().count(), 
				PaidUpValue(), 
				MarketLot(), 
				FaceValue(), 
				m_cached_latest_candle_date.time_since_epoch().count(),
				m_candle_count);

			db.exec(query2);
			transaction.commit();

			is_success = true;
		}


		catch (std::exception& e)
		{
			is_success = false;
			//std::cout << e.what() << std::endl;
			// Database might be locked by another thread. Wait for a bit and try again.
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}








std::unique_ptr<AsyncTask> Security::GetGenerateNewsPointsTask()
{
	std::function<void()> generate_news_points = [&]()
		{
			LoadCandleDataToMemory();

			std::vector<std::unique_ptr<Pattern>> patterns = std::move(TradinatorCoreSpace::Utils::GetAvailablePatterns());

			const CandlesData& candles_data = m_candles_data->GetData();

			for (uint64_t i = 0; i < candles_data.m_dates.size(); ++i)
			{
				for (const std::unique_ptr<Pattern>& pattern : patterns)
				{
					std::vector<uint64_t> pattern_range = pattern->Check(i, candles_data);
					if (pattern_range.size() > 0)
					{
						NewsPoint news_point(this->shared_from_this());
						news_point.m_date_range = pattern_range;
						news_point.m_pattern = pattern->PatternType();

						m_news_points_data->GetAsyncDataCopy().emplace_back(std::move(news_point));

						// First come first serve
						break;
					}
				}
			}
		};


	return std::make_unique<AsyncTask>(
		std::format("Analysing candle data of {}", Symbol()),
		[&]()
		{
			m_news_points_data->SetDataReady(false);
		},
		generate_news_points,
		[&]()
		{
			m_news_points_data->SetDataReady(true);
		}
	);
}
















void Security::FromString(std::string str)
{
	std::vector<std::string> split_strings;
	int prev_index = 0;
	for (int i = 0; i <= str.size(); ++i)
	{
		if (i == str.size())
		{
			split_strings.push_back(str.substr(prev_index, str.size() - prev_index));
		}
		else if (str[i] == ',')
		{
			split_strings.push_back(str.substr(prev_index, i - prev_index));
			prev_index = i + 1;
		}
	}

	m_symbol = split_strings[0];
	m_name = split_strings[1];
	m_series = split_strings[2];

	std::tm tm = {};
	std::stringstream ss(split_strings[3]);
	ss >> std::get_time(&tm, "%d-%b-%y");
	m_date_of_listing = std::chrono::system_clock::from_time_t(std::mktime(&tm));

	m_paid_up_value = std::stoi(split_strings[4]);
	m_market_lot = std::stoi(split_strings[5]);
	m_isin_number = split_strings[6];
	m_face_value = std::stoi(split_strings[7]);
}