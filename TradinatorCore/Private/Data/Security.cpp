#include "Data/Security.h"

#include <iostream>
#include <format>
#include <fstream>
#include <sstream>
#include <chrono>

#include "json/json.h"

#include "TradinatorCore.h"
#include "Data/Candle.h"
#include "Market/Market.h"
#include "Utils/ThreadManager.h"
#include "Utils/AsyncTask.h"
#include "Utils/DownloadTask.h"
#include "Utils/Utils.h"

static std::string _STATUS_ = "status";
static std::string _SUCCESS_ = "success";

static std::string _DATA_ = "data";
static std::string _CANDLES_ = "candles";

static std::string _DATE_ = "date";
static std::string _OPEN_ = "open";
static std::string _HIGH_ = "high";
static std::string _LOW_ = "low";
static std::string _CLOSE_ = "close";
static std::string _VOLUME_ = "volume";
static std::string _OPEN_INTEREST_ = "open_interest";



Security::Security()
	: Company()
	, m_series()
	, m_paid_up_value(0)
	, m_market_lot(0)
	, m_face_value(0)
{

}



void Security::DownloadSecurityData(std::function<void()> callback)
{
	m_callback = callback;

	std::shared_ptr<Market> parent_market;

	if (!(parent_market = m_parent_market.lock()))
	{
		return;
	}
	
	//TODO: Use std::tmpfile instead of creating temp file like this
	// store downloaded data in temp file and process to our standards
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::system_clock::rep count = now.time_since_epoch().count();
	std::string tmp_file_name = parent_market->GetRawDataFolderPath() + "/" + std::format("{}.tmp", count);

	std::function<void()> load_historical_data_if_exists = std::bind(&Security::LoadHistoricalDataIfExists, this);
	std::function<void()> download_daily_data = std::bind(&Security::DownloadDailyData, this, tmp_file_name);

	std::shared_ptr<TradinatorCore> tradinator_core;
	if (!(tradinator_core = parent_market->GetTradinatorCore().lock()))
	{
		return;
	}

	
	tradinator_core->GetThreadManager()->AddTask(std::make_unique<AsyncTask>(
		"Gathering candles data for " + m_symbol,
		[&]() 
		{
			// callback
		},
		load_historical_data_if_exists,
		download_daily_data
		));
}

void Security::LoadHistoricalDataIfExists()
{
	if (DoesRawHistoricalDataExist() && !m_candle_data.WasEverReadyBefore())
	{
		m_candle_data.SetDataReady(false);
		Json::Value historical_candle_json_data;

		{
			// scope to make sure we aren't holding the file for more time than we should
			std::string raw_historical_file_path = GetRawHistoricalDataFilePath();
			std::ifstream raw_historical_file(raw_historical_file_path);
			raw_historical_file >> historical_candle_json_data;
		}
		
		Json::Value json_candles = historical_candle_json_data[_DATA_][_CANDLES_];
		for (Json::Value& candle : json_candles)
		{
			// Gather data from json
			std::string date_str = candle[_DATE_].asCString();
			std::istringstream is{ date_str };
			std::chrono::system_clock::time_point date;
			is >> std::chrono::parse("%F", date);

			double open = candle[_OPEN_].asDouble();
			double high = candle[_HIGH_].asDouble();
			double low = candle[_LOW_].asDouble();
			double close = candle[_CLOSE_].asDouble();
			uint64_t volume = candle[_VOLUME_].asUInt64();
			uint64_t open_interest = candle[_OPEN_INTEREST_].asUInt64();


			Candle candle_data;
			candle_data.date = date;
			candle_data.open = open;
			candle_data.high = high;
			candle_data.low = low;
			candle_data.close = close;
			candle_data.volume = volume;
			candle_data.open_interest = open_interest;

			m_candle_data.GetAsyncDataCopy()[date] = candle_data;
		}

		m_candle_data.SetDataReady(true);
	}
}

void Security::DownloadDailyData(std::string tmp_file_path)
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point to_tp = now;

	std::chrono::days delta_time(50 * 365); // get 50 years of historical data
	std::chrono::system_clock::time_point from_tp = to_tp - delta_time;
	
	// ensuring const version of getters get called 
	const AsyncCandleData& candle_data = m_candle_data;

	if (candle_data.GetData().begin() != candle_data.GetData().end())
	{
		// historical data exist

		// Get the latest date until which historical data is available
		// m_candle_data is sorted by date
		from_tp = (*candle_data.GetData().begin()).second.date + std::chrono::days(1);
	}

	if (to_tp <= (from_tp + std::chrono::days(1)))
	{
		// if to is not more than a day of from, historical data is up to date
		return;
	}
	
	
	std::shared_ptr<TradinatorCore> tradinator_core;
	std::shared_ptr<Market> parent_market;

	if (!(parent_market = m_parent_market.lock()))
	{
		return;
	}
	if (!(tradinator_core = parent_market->GetTradinatorCore().lock()))
	{
		return;
	}
	
	std::string to = std::format("{:%F}", to_tp);
	std::string from = std::format("{:%F}", from_tp);

	/* eg url format
	https://api.upstox.com/v2/historical-candle/NSE_EQ|INE696F01016/day/2025-04-06/2025-04-01*/
	std::string url = std::format("https://api.upstox.com/v2/historical-candle/{}_{}|{}/day/{}/{}"
		, parent_market->GetMarketCode()
		, m_series
		, m_isin_number
		, to
		, from);

	std::cout << "URL: " << url << std::endl;
	tradinator_core->GetThreadManager()->AddTask(std::make_unique<DownloadTask>(
		[tmp_file_path, this]() {
			// callback
			ProcessDownloadedData(tmp_file_path);
		},
		url, tmp_file_path
	));
}

void Security::ProcessDownloadedData(std::string tmp_file_path)
{
	Json::Value downloaded_json_data;
	{
		std::ifstream downloaded_tmp_file(tmp_file_path);
		downloaded_tmp_file >> downloaded_json_data;
	}

	Json::Value historical_candle_json_data;
	{
		if (DoesRawHistoricalDataExist())
		{
			std::string raw_historical_file_path = GetRawHistoricalDataFilePath();
			std::ifstream raw_historical_file(raw_historical_file_path);
			raw_historical_file >> historical_candle_json_data;
		}
	}

	bool should_seralize_data = false;

	if (downloaded_json_data[_STATUS_] == _SUCCESS_ && downloaded_json_data[_DATA_] && downloaded_json_data[_DATA_][_CANDLES_])
	{
		Json::Value json_candles = downloaded_json_data[_DATA_][_CANDLES_];
		Json::ArrayIndex candles_count = json_candles.size();

		for (Json::ArrayIndex i = 0; i < candles_count; ++i)
		{
			should_seralize_data = true;

			std::string date_str = json_candles[i][0].asCString();
			std::istringstream is{ date_str };
			std::chrono::system_clock::time_point date;
			is >> std::chrono::parse("%F", date);

			Json::Value candle;
			candle[_DATE_] = std::format("{:%F}", date);
			candle[_OPEN_] = json_candles[i][1];
			candle[_HIGH_] = json_candles[i][2];
			candle[_LOW_] = json_candles[i][3];
			candle[_CLOSE_] = json_candles[i][4];
			candle[_VOLUME_] = json_candles[i][5];
			candle[_OPEN_INTEREST_] = json_candles[i][6];

			historical_candle_json_data[_DATA_][_CANDLES_].insert(i, candle);

			Candle candle_data;
			candle_data.date = date;
			candle_data.open = json_candles[i][1].asDouble();
			candle_data.high = json_candles[i][2].asDouble();
			candle_data.low = json_candles[i][3].asDouble();
			candle_data.close = json_candles[i][4].asDouble();
			candle_data.volume = json_candles[i][5].asLargestUInt();
			candle_data.open_interest = json_candles[i][6].asLargestUInt();

			// This will fail once I have serial async tasks
			//m_candle_data.GetData()[date] = candle_data;
		}
	}

	if (should_seralize_data)
	{
		// if there is new data seralize it.
		std::string raw_historical_file_path = GetRawHistoricalDataFilePath();
		std::ofstream raw_historical_file(raw_historical_file_path);
		raw_historical_file << historical_candle_json_data;
	}

	// finally delete temp file
	std::remove(tmp_file_path.c_str());
}

void Security::SetParentMarket(std::weak_ptr<Market> parent)
{
	m_parent_market = parent;
}


std::string Security::GetRawHistoricalDataFilePath() const
{
	if (std::shared_ptr<Market> parent_market = m_parent_market.lock())
	{
		std::string folder_path = parent_market->GetRawDataFolderPath();
		return folder_path + "/" + m_symbol+".json";
	}

	return "";
}

bool Security::DoesRawHistoricalDataExist() const
{
	return Utils::DoesFileExist(GetRawHistoricalDataFilePath());
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

#define READ_STRING_FROM_STREAM(stream, string_name, string_size, buffer)			\
stream.read(reinterpret_cast<char*>(&string_size), sizeof(string_size));			\
buffer = new char[string_size + 1];													\
stream.read(reinterpret_cast<char*>(buffer), string_size);							\
buffer[string_size] = '\0';															\
string_name = buffer;																\
delete[] buffer;																	\
buffer = nullptr;																	\

void Security::ReadFromStream(std::istream& stream)									
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

void Security::WriteToFile(std::ofstream& stream)
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

std::string Security::ToString() const
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


std::ofstream& operator << (std::ofstream& stream, Security& security)
{
	security.WriteToFile(stream);

	return stream;
}

std::ostream& operator << (std::ostream& stream, Security& security)
{
	stream << security.ToString();

	return stream;
}

std::istream& operator >> (std::istream& stream, Security& security)
{
	security.ReadFromStream(stream);
	return stream;
}