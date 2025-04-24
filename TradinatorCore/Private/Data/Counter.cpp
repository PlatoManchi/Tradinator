#include "Data/Counter.h"

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

#include "Indicators/Indicator.h"
#include "Indicators/SMA.h"
#include "Indicators/WMA.h"
#include "Indicators/EMA.h"

static std::string _STATUS_ = "status";
static std::string _SUCCESS_ = "success";

static std::string _DATA_ = "data";
static std::string _CANDLES_ = "candles";

std::vector<std::unique_ptr<Indicator>> Counter::GetAvailableIndicators()
{
	std::vector<std::unique_ptr<Indicator>> result;

	result.push_back(std::make_unique<SMA>(20));
	result.push_back(std::make_unique<WMA>(20));
	result.push_back(std::make_unique<EMA>(20));

	return result;
}


Counter::Counter()
	: Company()
	, m_series()
	, m_paid_up_value(0)
	, m_market_lot(0)
	, m_face_value(0)
	, m_database_connection(TradinatorCoreSpace::Utils::GetTradinatorDatabasePath())
	, m_candle_data(std::make_shared<AsyncData<AsyncCandleData>>())
	, m_is_downloading(false)
	, m_is_inserting(false)
	, m_is_dirty(true)
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::days delta_time(50 * 365); // 50 years

	m_cached_latest_candle_date = now - delta_time;
}



std::string Counter::GetProcessedHistoricalDataFilePath() const
{
	std::shared_ptr<Market> owning_market = m_owning_market.lock();
	assert(owning_market);

	std::string folder_path = owning_market->GetProcessedDataFolderPath();
	return folder_path + "/" + m_symbol + ".bin";
}

bool Counter::DoesProcessedHistoricalDataExist() const
{
	return m_database_connection.tableExists(GetTableName());
}

std::chrono::system_clock::time_point Counter::GetLastCandleDataDate() const
{
	if (!m_is_dirty)
		return m_cached_latest_candle_date;

	
	if (DoesProcessedHistoricalDataExist())
	{
		SQLite::Statement query(m_database_connection, std::format("SELECT \"LatestCandleData\" FROM Securities WHERE ISIN=\"{}\"", ISIN_Number()));
		if (query.executeStep())
		{
			std::chrono::system_clock::rep time_count = query.getColumn(0);
			std::chrono::system_clock::duration duration_since_epoch(time_count);

			// cache
			m_is_dirty = false;
			m_cached_latest_candle_date = std::chrono::system_clock::time_point(duration_since_epoch);

			return m_cached_latest_candle_date;
		}
	}

	

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::days delta_time(50 * 365); // 50 years

	return now - delta_time;
}


bool Counter::IsHistoricalCandleDataOutDated() const
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

std::unique_ptr<AsyncTask> Counter::GetDownloadLatestCandleDataTask(std::function<void()> callback)
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

	
	// Set the counter to updating state
	std::unique_ptr<AsyncTask> set_updating_true = std::make_unique<AsyncTask>(
		std::string(""),
		[&]()
		{
			std::lock_guard<std::mutex> lock(m_counter_mutex);

			m_is_downloading = true;
		},
		[]() {}
	);

	std::string tmp_file_path = owning_market->GetRawDataFolderPath() + "/" + std::format("{}.json", Symbol());
	std::unique_ptr<AsyncTask> download_task = std::make_unique<DownloadTask>(callback, url, tmp_file_path);

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
			std::lock_guard<std::mutex> lock(m_counter_mutex); 
			m_is_downloading = false; 
		});
	//return download_task;
	//return std::make_unique<DownloadTask>(callback, url, tmp_file_path);
}

void Counter::InsertRawDataToDatabase()
{
	if (m_is_downloading || m_is_inserting) return;
	
	{
		std::lock_guard<std::mutex> lock(m_counter_mutex);
		m_is_inserting = true;
	}
	

	std::shared_ptr<Market> owning_market = m_owning_market.lock();
	assert(owning_market);
	std::string tmp_file_path = owning_market->GetRawDataFolderPath() + "/" + std::format("{}.json", Symbol());

	Json::Value downloaded_candle_json_data;
	{
		// scope to make sure we aren't holding the file for more time than we should
		std::ifstream downloaded_tmp_file(tmp_file_path);
		downloaded_tmp_file >> downloaded_candle_json_data;
	}

	Json::Value json_candles = downloaded_candle_json_data[_DATA_][_CANDLES_];
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
					// table that contains daily candle data for this counter

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

				SQLite::Transaction transaction(db);
				for (Json::Value& candle : json_candles)
				{
					std::string date_str = candle[0].asCString();
					std::istringstream is{ date_str };
					std::chrono::system_clock::time_point date;
					is >> std::chrono::parse("%F", date);

					if (date > m_cached_latest_candle_date)
					{
						m_cached_latest_candle_date = date;
					}

					WriteCandleToDatabase(db, candle);
				}
				transaction.commit();

				UpdateLatestCandleDataDate();

				is_success = true;
			}
			catch (std::exception&)
			{
				is_success = false;
				//Log::GetInstance().Write(std::format("ERROR: SQLite exception: {}", e.what()));
				
				// Database might be locked by another thread. Wait for a bit and try again.
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}
	
	{
		std::lock_guard<std::mutex> lock(m_counter_mutex);
		m_is_inserting = false;
	}
}

void Counter::WriteCandleToDatabase(SQLite::Database& db, const Json::Value& candle)
{
	// Gather data from json
	std::string date_str = candle[0].asCString();
	std::istringstream is{ date_str };
	std::chrono::system_clock::time_point date;
	is >> std::chrono::parse("%F", date);

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

	std::string query_str = std::format("INSERT OR IGNORE INTO \"{}\" (Date, Open, High, Low, Close, Volume, OpenInterest) VALUES "  \
		"(\"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\")"
		, GetTableName()
		, date.time_since_epoch().count()
		, candle[1].asDouble()
		, candle[2].asDouble()
		, candle[3].asDouble()
		, candle[4].asDouble()
		, volume
		, open_interest);

	db.exec(query_str);
}

void Counter::WriteCandleToDatabaseAndLoadToMemory(SQLite::Database& db, const Json::Value& candle)
{
	WriteCandleToDatabase(db, candle);

	std::string date_str = candle[0].asCString();
	std::istringstream is{ date_str };
	std::chrono::system_clock::time_point date;
	is >> std::chrono::parse("%F", date);

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

	Candle candle_data;
	candle_data.m_date = date;
	candle_data.m_open = candle[1].asDouble();
	candle_data.m_high = candle[2].asDouble();
	candle_data.m_low = candle[3].asDouble();
	candle_data.m_close = candle[4].asDouble();
	candle_data.m_volume = volume;
	candle_data.m_open_interest = open_interest;

	m_candle_data->GetAsyncDataCopy()[date] = candle_data;
}

void Counter::LoadCandleDataToMemory()
{
	std::function<void()> load_candle_data_to_memory = [&]()
		{
			if (!DoesProcessedHistoricalDataExist())
			{
				m_candle_data->SetDataReady(true);
				return;
			}

			bool is_success = false;
			while (!is_success)
			{
				try
				{
					if (m_candle_data->IsDataReady())
					{
						m_candle_data->SetDataReady(false);
					}


					std::string query_str = std::format("SELECT * FROM \"{}\"", GetTableName());
					SQLite::Statement query(m_database_connection, query_str);
					while (query.executeStep())
					{
						// Date, Open, High, Low, Close, Volume, OpenInterest
						std::chrono::system_clock::rep time_count = query.getColumn(0);
						std::chrono::system_clock::duration duration_since_epoch(time_count);
						std::chrono::system_clock::time_point date(duration_since_epoch);

						Candle candle_data;
						candle_data.m_date = date;
						candle_data.m_open = query.getColumn(1);
						candle_data.m_high = query.getColumn(2);
						candle_data.m_low = query.getColumn(3);
						candle_data.m_close = query.getColumn(4);
						candle_data.m_volume = query.getColumn(5).getInt64();
						candle_data.m_open_interest = query.getColumn(6).getInt64();

						m_candle_data->GetAsyncDataCopy()[date] = candle_data;
					}

					m_candle_data->SetDataReady(true);
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
		};

	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	owning_tradinator_core_thread->GetAsyncTaskManager()->AddTask(std::make_unique<AsyncTask>(
		std::format("Loading candle data for {}", m_symbol),
		load_candle_data_to_memory,
		[]() {}
	));
}

void Counter::UnloadCandleDataFromMemory()
{
	m_candle_data->Reset();
}


void Counter::UpdateLatestCandleDataDate()
{
	bool is_success = false;
	while (!is_success)
	{
		try
		{
			SQLite::Database db(TradinatorCoreSpace::Utils::GetTradinatorDatabasePath(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

			// Begin transaction
			SQLite::Transaction transaction(db);
			std::string query2 = std::format("UPDATE Securities SET LatestCandleData = \"{}\" WHERE  ISIN = \"{}\";"
				, m_cached_latest_candle_date.time_since_epoch().count(), ISIN_Number());

			db.exec(query2);
			transaction.commit();

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

void Counter::FromString(std::string str)
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

#define READ_STRING_FROM_STREAM(stream, string_name, string_size, buffer)			\
stream.read(reinterpret_cast<char*>(&string_size), sizeof(string_size));			\
buffer = new char[string_size + 1];													\
stream.read(reinterpret_cast<char*>(buffer), string_size);							\
buffer[string_size] = '\0';															\
string_name = buffer;																\
delete[] buffer;																	\
buffer = nullptr;																	\

void Counter::ReadFromStream(std::istream& stream)									
{
	std::size_t string_size;
	char* buffer = nullptr;
	
	READ_STRING_FROM_STREAM(stream, m_symbol, string_size, buffer);
	READ_STRING_FROM_STREAM(stream, m_name, string_size, buffer);
	READ_STRING_FROM_STREAM(stream, m_series, string_size, buffer);

	std::chrono::system_clock::rep count;
	stream.read(reinterpret_cast<char*>(&count), sizeof(count));
	std::chrono::system_clock::duration duration_since_epoch(count);
	std::chrono::system_clock::time_point date_of_listing(duration_since_epoch);
	m_date_of_listing = date_of_listing;

	stream.read(reinterpret_cast<char*>(&m_paid_up_value), sizeof(m_paid_up_value));
	stream.read(reinterpret_cast<char*>(&m_market_lot), sizeof(m_market_lot));
	READ_STRING_FROM_STREAM(stream, m_isin_number, string_size, buffer);
	stream.read(reinterpret_cast<char*>(&m_face_value), sizeof(m_face_value));
}

#define WRITE_STRING_TO_STREAM(stream, string_name, string_size)						\
string_size = string_name.size();														\
/* write the size of string first */													\
stream.write(reinterpret_cast<const char*>(&string_size), sizeof(string_size));			\
/* write the string contents */															\
stream.write(reinterpret_cast<const char*>(string_name.c_str()), string_size);			\

void Counter::WriteToFile(std::ofstream& stream)
{
	std::size_t string_size;
	// Writting in binary format
	WRITE_STRING_TO_STREAM(stream, m_symbol, string_size);
	WRITE_STRING_TO_STREAM(stream, m_name, string_size);
	WRITE_STRING_TO_STREAM(stream, m_series, string_size);

	std::chrono::system_clock::rep count = m_date_of_listing.time_since_epoch().count();
	stream.write(reinterpret_cast<const char*>(&count), sizeof(count));

	stream.write(reinterpret_cast<const char*>(&m_paid_up_value), sizeof(m_paid_up_value));
	stream.write(reinterpret_cast<const char*>(&m_market_lot), sizeof(m_market_lot));
	WRITE_STRING_TO_STREAM(stream, m_isin_number, string_size);
	stream.write(reinterpret_cast<const char*>(&m_face_value), sizeof(m_face_value));
}

std::string Counter::ToString() const
{
	return std::format("{},{},{},{},{},{},{},{}"
		, m_symbol
		, m_name
		, m_series
		, std::format("{:%d-%b-%y}", m_date_of_listing)
		, m_paid_up_value
		, m_market_lot
		, m_isin_number
		, m_face_value);
}


std::ofstream& operator << (std::ofstream& stream, Counter& counter)
{
	counter.WriteToFile(stream);

	return stream;
}

std::ostream& operator << (std::ostream& stream, Counter& counter)
{
	stream << counter.ToString();

	return stream;
}

std::istream& operator >> (std::istream& stream, Counter& counter)
{
	counter.ReadFromStream(stream);
	return stream;
}