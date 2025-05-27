

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
#include "Indicators/TrendAnalysisDebug.h"

#ifdef _TECHNICAL_ANALYSIS_ISPC_
#include "utils_ispc.h"
#endif // _TECHNICAL_ANALYSIS_ISPC_



std::string Security::_STATUS_ = "status";
std::string Security::_SUCCESS_ = "success";

std::string Security::_DATA_ = "data";
std::string Security::_CANDLES_ = "candles";


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

	bool is_success = false;
	while (!is_success)
	{
		try
		{
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

			is_success = true;
		}
		catch(std::exception e)
		{
			is_success = false;

			//Log::GetInstance().Write(std::format("ERROR: GetLastCandleDataDate: SQLite exception: {}", e.what()));

			// Database might be locked by another thread. Wait for a bit and try again.
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
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






std::string Security::GetDownloadURL() const
{
	std::chrono::system_clock::time_point to_tp = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point from_tp = GetLastCandleDataDate() + std::chrono::days(1);

	std::string to = std::format("{:%F}", to_tp);
	std::string from = std::format("{:%F}", from_tp);


	std::shared_ptr<Market> owning_market = m_owning_market.lock();
	assert(owning_market);

	/* eg url format
	https://api.upstox.com/v2/historical-candle/NSE_EQ|INE696F01016/day/2025-04-06/2025-04-01*/
	return std::format("https://api.upstox.com/v2/historical-candle/{}_{}|{}/day/{}/{}"
		, owning_market->GetMarketCode()
		, m_series
		, m_isin_number
		, to
		, from);

}

std::string Security::GetTmpDownloadFilePath() const
{
	std::shared_ptr<Market> owning_market = m_owning_market.lock();
	assert(owning_market);

	return std::format("{}/{}.json", owning_market->GetRawDataFolderPath(), Symbol());
	//return owning_market->GetRawDataFolderPath() + "/" + std::format("{}.json", Symbol());
}

std::unique_ptr<AsyncTask> Security::GetDownloadLatestCandleDataTask()
{
	std::shared_ptr<Market> owning_market = m_owning_market.lock();
	assert(owning_market);

	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	
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

	std::string tmp_file_path = GetTmpDownloadFilePath();
	std::unique_ptr<AsyncTask> download_task = std::make_unique<DownloadTask>([](){}, GetDownloadURL(), tmp_file_path);

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


	// load the json into memory
	const Json::Value* data = m_raw_downloaded_data.find(_DATA_);
	if (!data)
	{
		return;
	}

	const Json::Value* candles_data = data->find(_CANDLES_);
	if (!candles_data)
	{
		return;
	}

	int64_t json_candle_count = candles_data->size();
	if (json_candle_count == 0)
	{
		return;
	}

	// see if data is newer than what ever is stored
	std::chrono::system_clock::time_point last_candle_date = GetLastCandleDataDate();

	Json::Value::const_iterator candle_itr = candles_data->end();
	--candle_itr;

	std::string date_str = (*candle_itr)[0].asCString();
	std::istringstream is{ date_str };
	std::chrono::system_clock::time_point date;
	is >> std::chrono::parse("%F", date);

	if (date < last_candle_date)
	{
		return;
	}

	const uint64_t polynomial_order = 25;
	const uint64_t atr_window_size = 30;
	const uint64_t min_distance_btw_peaks = 15;

	// The json response from website sometimes has duplicate dates.
	// so use a map to easily remove duplicates
	std::unordered_map<std::chrono::system_clock::rep, bool> duplicate_date_checker;

	bool is_success = false;
	while (!is_success)
	{
		try
		{
			if (DoesProcessedHistoricalDataExist())
			{
				// if historical data exists load some data necessary for analysis
				// since most of analysis techniques relay on previous data of window size

				uint64_t back_data_needed = std::max({ polynomial_order, min_distance_btw_peaks, atr_window_size }) + 10; // 10 extra for smoothing

				m_new_downloaded_data.Reserve(back_data_needed + json_candle_count);

				std::string query_str = std::format("SELECT * FROM \"{}\" ORDER BY Date DESC LIMIT {}", GetTableName(), back_data_needed);
				SQLite::Statement query(m_database_connection, query_str);

				//LoadCandleDataToMemoryFromQuery(query, m_new_downloaded_data);
				while (query.executeStep())
				{
					// Date, Open, High, Low, Close, Volume, OpenInterest
					std::chrono::system_clock::rep time_count = query.getColumn(0).getInt64();
					std::chrono::system_clock::duration duration_since_epoch(time_count);
					std::chrono::system_clock::time_point date(duration_since_epoch);

					if (!duplicate_date_checker.contains(time_count))
					{
						duplicate_date_checker[time_count] = true;

						m_new_downloaded_data.m_dates.push_back(date);
						m_new_downloaded_data.m_opens.push_back(query.getColumn(1).getDouble());
						m_new_downloaded_data.m_highs.push_back(query.getColumn(2).getDouble());
						m_new_downloaded_data.m_lows.push_back(query.getColumn(3).getDouble());
						m_new_downloaded_data.m_closes.push_back(query.getColumn(4).getDouble());
						m_new_downloaded_data.m_volumes.push_back(query.getColumn(5).getInt64());
						m_new_downloaded_data.m_open_interests.push_back(query.getColumn(6).getInt64());
					}
				}

				m_back_data_count = m_new_downloaded_data.m_dates.size();
			}
			else
			{
				m_new_downloaded_data.Reserve(json_candle_count);
				m_back_data_count = 0;
			}

			is_success = true;
		}
		catch (std::exception e)
		{
			is_success = false;

			//Log::GetInstance().Write(std::format("ERROR: AnalyzeDownloadedData: SQLite exception: {}", e.what()));

			// Database might be locked by another thread. Wait for a bit and try again.
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	
	Json::Value::const_iterator itr = candles_data->end();
	bool is_done = false;
	while (!is_done)
	{
		--itr;
		is_done = (itr == candles_data->begin());

		std::string date_str = (*itr)[0].asCString();
		std::istringstream is{ date_str };
		std::chrono::system_clock::time_point date;
		is >> std::chrono::parse("%F", date);
		
		if (date > last_candle_date && !duplicate_date_checker.contains(date.time_since_epoch().count()))
		{
			duplicate_date_checker[date.time_since_epoch().count()] = true;

			m_new_downloaded_data.m_dates.push_back(date);
			m_new_downloaded_data.m_opens.push_back((*itr)[1].asDouble());
			m_new_downloaded_data.m_highs.push_back((*itr)[2].asDouble());
			m_new_downloaded_data.m_lows.push_back((*itr)[3].asDouble());
			m_new_downloaded_data.m_closes.push_back((*itr)[4].asDouble());

			// one candle from the data has negative value for volumes and open interest for some reason and this 
			// is for that one random wrong value
			int64_t volume = (*itr)[5].asInt64();
			if (volume < 0)
			{
				volume = 0;
			}
			int64_t open_interest = (*itr)[6].asInt64();
			if (open_interest < 0)
			{
				open_interest = 0;
			}

			m_new_downloaded_data.m_volumes.push_back(volume);
			m_new_downloaded_data.m_open_interests.push_back(open_interest);
		}
	}

	if (m_symbol == "63MOONS")
	{
		std::cout << m_symbol << std::endl;
	}

	// Unload the raw data. We never need it again
	m_raw_downloaded_data = std::move(Json::Value());
}


void Security::AnalyzeDownloadedData()
{
	const uint64_t window_size = 4;
	const uint64_t polynomial_order = 25;
	const uint64_t atr_window_size = 30;
	const uint64_t min_distance_btw_peaks = 15;
	const uint64_t width_for_finding_peaks = 3;
	const double relative_width = 0.5;

	
	size_t total_candles = m_new_downloaded_data.m_dates.size();

	if (total_candles > 0)
	{
		m_new_downloaded_data.m_trends = std::move(std::vector<ETrend>(total_candles, ETrend::None));
		m_new_downloaded_data.m_patterns = std::move(std::vector<EPattern>(total_candles, EPattern::None));
		m_new_downloaded_data.m_strategies = std::move(std::vector<uint64_t>(total_candles, 0));

		// Analyze
		// -------------------------- Trends ---------------------
		uint64_t max_data_required_for_analysis = std::max({ polynomial_order, min_distance_btw_peaks, atr_window_size });
		if (total_candles > max_data_required_for_analysis)
		{
			std::vector<double> a(polynomial_order * window_size, 0.0);
			std::vector<double> at(polynomial_order * window_size, 0.0);
			std::vector<double> ata(window_size * window_size, 0.0);
			std::vector<double> ata_inv(window_size * window_size, 0.0);
			std::vector<double> ata_inv_tmp(window_size * window_size, 0.0);// buffer to calculate inverse
			std::vector<double> ata_inv_at(window_size * polynomial_order, 0.0);
			std::vector<double> convolution_coefficient(polynomial_order, 0.0);
			std::vector<double> convolution_coefficient_tmp(polynomial_order, 0.0);

			std::vector<double> savitzky_golay_output(total_candles, 0.0);

			std::vector<double> average_true_ranges(total_candles, 0.0);

			// max number of peaks possible will be count / m_distance_btw_peaks
			std::vector<double> peaks(total_candles / min_distance_btw_peaks, 0.0);
			std::vector<double> troughs(total_candles / min_distance_btw_peaks, 0.0);

			uint64_t peaks_count = 0;
			uint64_t troughs_count = 0;

			std::vector<double> trend_points(total_candles, 0.0);

#ifdef _TECHNICAL_ANALYSIS_ISPC_
			std::vector<int8_t> peaks_and_trough_tmp_buff(total_candles, 0);

			ispc::calculate_trend_analysis_debug(
				m_new_downloaded_data.m_highs.data(),
				m_new_downloaded_data.m_lows.data(),
				m_new_downloaded_data.m_closes.data(),
				a.data(),
				at.data(),
				ata.data(),
				ata_inv.data(),
				ata_inv_tmp.data(),
				ata_inv_at.data(),
				convolution_coefficient.data(),
				convolution_coefficient_tmp.data(),
				savitzky_golay_output.data(),
				average_true_ranges.data(),
				peaks.data(),
				troughs.data(),
				peaks_and_trough_tmp_buff.data(),
				trend_points.data(),
				&peaks_count,
				&troughs_count,
				window_size,
				atr_window_size,
				polynomial_order,
				min_distance_btw_peaks,
				width_for_finding_peaks,
				relative_width,
				2,
				total_candles
			);
#else
			TrendAnalysisDebug trend_analysis;
			trend_analysis.CalculateRaw(
				m_new_downloaded_data.m_highs.data(),
				m_new_downloaded_data.m_lows.data(),
				m_new_downloaded_data.m_closes.data(),
				a.data(),
				at.data(),
				ata.data(),
				ata_inv.data(),
				ata_inv_tmp.data(),
				ata_inv_at.data(),
				convolution_coefficient.data(),
				convolution_coefficient_tmp.data(),
				savitzky_golay_output.data(),
				average_true_ranges.data(),
				peaks.data(),
				troughs.data(),
				trend_points.data(),
				&peaks_count,
				&troughs_count,
				window_size,
				atr_window_size,
				polynomial_order,
				min_distance_btw_peaks,
				width_for_finding_peaks,
				relative_width,
				2,
				total_candles
			);
#endif // _TECHNICAL_ANALYSIS_ISPC_



			for (uint64_t i = 0; i < total_candles; ++i)
			{
				if (trend_points[i] > 0.9)
				{
					m_new_downloaded_data.m_trends[i] = ETrend::Up;
				}
				else if (trend_points[i] < -0.9)
				{
					m_new_downloaded_data.m_trends[i] = ETrend::Down;
				}
				else
				{
					m_new_downloaded_data.m_trends[i] = ETrend::None;
				}
			}
		}


		// --------------------------- Patterns ------------------
		std::vector<std::unique_ptr<Pattern>> patterns = std::move(TradinatorCoreSpace::Utils::GetAvailablePatterns());

		for (uint64_t i = 0; i < m_new_downloaded_data.m_dates.size(); ++i)
		{
			EPattern satisfied_patterns = EPattern::None;

			for (const std::unique_ptr<Pattern>& pattern : patterns)
			{
				if (pattern->Check(i, m_new_downloaded_data))
				{
					satisfied_patterns = satisfied_patterns | pattern->PatternType();
				}
			}

			m_new_downloaded_data.m_patterns[i] = satisfied_patterns;
		}
	}
}





void Security::InsertRawDataToDatabase()
{
	if (m_is_inserting)
	{
		return;
	}
	
	Log::GetInstance().Write(std::format("Inserting: {} - {}", Symbol(), Name()));

	size_t count = m_new_downloaded_data.m_dates.size();
	if (count > 0)
	{
		{
			std::lock_guard<std::mutex> lock(m_security_mutex);
			m_is_inserting = true;
		}

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
						"OpenInterest     INTEGER      NOT NULL); ", GetTableName());


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
					else
					{
						m_candle_count = 0;
					}
				}


				// Get previous trend
				ETrend previous_trend = ETrend::None;
				{
					std::string query_str = std::format("SELECT Trend FROM Trends WHERE ISIN=\"{}\" ORDER BY Date DESC", ISIN_Number());
					SQLite::Statement query(m_database_connection, query_str);
					if (query.executeStep())
					{
						previous_trend = (ETrend)query.getColumn(0).getInt64();
					}
				}


				SQLite::Transaction transaction(db);
				// Prepare the insert_candle statement once
				SQLite::Statement insert_candle(db, std::format("INSERT OR IGNORE INTO \"{}\" (Date, Open, High, Low, Close, Volume, OpenInterest) VALUES "  \
					"(?, ?, ?, ?, ?, ?, ?)", GetTableName()));

				SQLite::Statement insert_trends(db, std::format("INSERT OR REPLACE INTO \"Trends\" (ISIN, Symbol, Date, DateIndex, Trend) VALUES "  \
					"(?, ?, ?, ?, ?)"));

				SQLite::Statement insert_patterns(db, std::format("INSERT OR REPLACE INTO \"Patterns\" (ISIN, Symbol, Date, DateIndex, Patterns) VALUES "  \
					"(?, ?, ?, ?, ?)"));

				SQLite::Statement insert_strategies(db, std::format("INSERT OR REPLACE INTO \"Strategies\" (ISIN, Symbol, Date, DateIndex, Strategies) VALUES "  \
					"(?, ?, ?, ?, ?)"));

				std::chrono::system_clock::time_point tmp_latest_candle_date = m_cached_latest_candle_date;

				size_t candles_to_insert = m_new_downloaded_data.m_dates.size();
				uint64_t start_index = m_candle_count - m_back_data_count;
				
				for (size_t i = 0; i < candles_to_insert; ++i)
				{
					std::chrono::system_clock::time_point date = m_new_downloaded_data.m_dates[i];

					if (date > m_cached_latest_candle_date)
					{
						m_cached_latest_candle_date = date;
					}

					insert_candle.bind(1, date.time_since_epoch().count());
					insert_candle.bind(2, m_new_downloaded_data.m_opens[i]);
					insert_candle.bind(3, m_new_downloaded_data.m_highs[i]);
					insert_candle.bind(4, m_new_downloaded_data.m_lows[i]);
					insert_candle.bind(5, m_new_downloaded_data.m_closes[i]);
					insert_candle.bind(6, (int64_t)m_new_downloaded_data.m_volumes[i]);
					insert_candle.bind(7, (int64_t)m_new_downloaded_data.m_open_interests[i]);
					
					// The json response sometimes has duplicate dates. Duplicates don't get inserted.
					insert_candle.exec();                // execute insert_candle
					insert_candle.reset();               // reset statement for next use
					insert_candle.clearBindings();       // clear bound values

					if (previous_trend != m_new_downloaded_data.m_trends[i])
					{
						previous_trend = m_new_downloaded_data.m_trends[i];

						insert_trends.bind(1, ISIN_Number());
						insert_trends.bind(2, Symbol());
						insert_trends.bind(3, date.time_since_epoch().count());
						insert_trends.bind(4, (int64_t)(start_index + i));
						insert_trends.bind(5, (int64_t)m_new_downloaded_data.m_trends[i]);

						insert_trends.exec();
						insert_trends.reset();
						insert_trends.clearBindings();
					}

					if (m_new_downloaded_data.m_patterns[i] != EPattern::None)
					{
						insert_patterns.bind(1, ISIN_Number());
						insert_patterns.bind(2, Symbol());
						insert_patterns.bind(3, date.time_since_epoch().count());
						insert_patterns.bind(4, (int64_t)(start_index + i));
						insert_patterns.bind(5, (int64_t)m_new_downloaded_data.m_patterns[i]);

						insert_patterns.exec();
						insert_patterns.reset();
						insert_patterns.clearBindings();
					}

					if (m_new_downloaded_data.m_strategies[i] != 0)
					{
						insert_strategies.bind(1, ISIN_Number());
						insert_strategies.bind(2, Symbol());
						insert_strategies.bind(3, date.time_since_epoch().count());
						insert_strategies.bind(4, (int64_t)(start_index + i));
						insert_strategies.bind(5, (int64_t)m_new_downloaded_data.m_strategies[i]);

						insert_strategies.exec();
						insert_strategies.reset();
						insert_strategies.clearBindings();
					}
				}
				
				transaction.commit();

				{
					std::string query_str = std::format("SELECT COUNT(*) FROM \"{}\";", GetTableName());
					SQLite::Statement query(m_database_connection, query_str);
					if (query.executeStep())
					{
						m_candle_count = query.getColumn(0).getInt64();
					}
				}
				

				UpdateSecuritySkeletonData();

				m_is_memory_in_sync = false;
				is_success = true;
			}
			catch (std::exception& e)
			{
				is_success = false;
				//Log::GetInstance().Write(std::format("ERROR: InsertRawDataToDatabase: SQLite exception: {}", e.what()));
				
				// Database might be locked by another thread. Wait for a bit and try again.
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}
	
	// Unload all the new downloaded data
	m_new_downloaded_data = std::move(CandlesData());

	{
		std::lock_guard<std::mutex> lock(m_security_mutex);
		m_is_inserting = false;
	}
}


void Security::LoadCandleDataToTempMemory()
{
	std::string query_str = std::format("SELECT * FROM \"{}\" ORDER BY Date ASC", GetTableName());
	SQLite::Statement query(m_database_connection, query_str);

	LoadCandleDataToMemoryFromQuery(query, m_new_downloaded_data);

	m_back_data_count = m_new_downloaded_data.m_dates.size();
}

void Security::LoadCandleDataToMemoryFromQuery(SQLite::Statement& query, CandlesData& candle_data, int64_t days)
{
	int64_t count = 0;

	while (query.executeStep())
	{
		// Date, Open, High, Low, Close, Volume, OpenInterest
		std::chrono::system_clock::rep time_count = query.getColumn(0).getInt64();
		std::chrono::system_clock::duration duration_since_epoch(time_count);
		std::chrono::system_clock::time_point date(duration_since_epoch);


		candle_data.m_dates.push_back(date);
		candle_data.m_opens.push_back(query.getColumn(1).getDouble());
		candle_data.m_highs.push_back(query.getColumn(2).getDouble());
		candle_data.m_lows.push_back(query.getColumn(3).getDouble());
		candle_data.m_closes.push_back(query.getColumn(4).getDouble());
		candle_data.m_volumes.push_back(query.getColumn(5).getInt64());
		candle_data.m_open_interests.push_back(query.getColumn(6).getInt64());

		++count;
		if (days >= 0 && count >= days)
		{
			break;
		}
	}
}



void Security::LoadCandleDataToMemory(int64_t days)
{
	if (days == 0) return;

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

			m_candles_data->GetAsyncDataCopy().Reserve(m_candle_count);
			
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			std::chrono::days delta_time;
			if (days < 0)
			{
				delta_time = std::chrono::days(50 * 365); // 50 years
			}
			else
			{
				delta_time = std::chrono::days(days);
			}

			std::chrono::system_clock::time_point load_from = now - delta_time;;
			ETrend previous_trend = ETrend::None;
			ETrend next_trend = ETrend::None;
			std::chrono::system_clock::time_point next_trend_date;
			std::string trends_query_str = std::format("SELECT \"Date\",\"Trend\" FROM \"Trends\" WHERE ISIN=\"{}\" ORDER BY Date ASC", ISIN_Number());
			SQLite::Statement trends_query(m_database_connection, trends_query_str);


			EPattern next_pattern = EPattern::None;
			std::chrono::system_clock::time_point next_pattern_date;
			std::string patterns_query_str = std::format("SELECT \"Date\",\"Patterns\" FROM \"Patterns\" WHERE ISIN=\"{}\" ORDER BY Date ASC", ISIN_Number());
			SQLite::Statement patterns_query(m_database_connection, patterns_query_str);


			int64_t next_strategy = 0;
			std::chrono::system_clock::time_point next_strategy_date;
			std::string strategies_query_str = std::format("SELECT \"Date\",\"Strategies\" FROM \"Strategies\" WHERE ISIN=\"{}\" ORDER BY Date ASC", ISIN_Number());
			SQLite::Statement strategies_query(m_database_connection, strategies_query_str);


			std::string query_str = std::format("SELECT * FROM \"{}\" ORDER BY Date ASC", GetTableName());
			SQLite::Statement query(m_database_connection, query_str);
			
			//LoadCandleDataToMemoryFromQuery(query, m_candles_data->GetAsyncDataCopy(), days);
			CandlesData& candle_data = m_candles_data->GetAsyncDataCopy();

			int64_t count = 0;

			while (query.executeStep())
			{
				// Date, Open, High, Low, Close, Volume, OpenInterest
				std::chrono::system_clock::rep time_count = query.getColumn(0).getInt64();
				std::chrono::system_clock::duration duration_since_epoch(time_count);
				std::chrono::system_clock::time_point date(duration_since_epoch);


				candle_data.m_dates.push_back(date);
				candle_data.m_opens.push_back(query.getColumn(1).getDouble());
				candle_data.m_highs.push_back(query.getColumn(2).getDouble());
				candle_data.m_lows.push_back(query.getColumn(3).getDouble());
				candle_data.m_closes.push_back(query.getColumn(4).getDouble());
				candle_data.m_volumes.push_back(query.getColumn(5).getInt64());
				candle_data.m_open_interests.push_back(query.getColumn(6).getInt64());

				if (next_trend_date < date)
				{
					while (!trends_query.isDone() && trends_query.executeStep())
					{
						std::chrono::system_clock::rep time_count = trends_query.getColumn(0);
						std::chrono::system_clock::duration duration_since_epoch(time_count);

						next_trend_date = std::chrono::system_clock::time_point(duration_since_epoch);
						next_trend = (ETrend)trends_query.getColumn(1).getInt64();
						if (next_trend_date >= date)
						{
							break;
						}
					}
				}

				if (next_trend_date == date)
				{
					previous_trend = next_trend;
				}

				candle_data.m_trends.push_back(previous_trend);


				if (next_pattern_date < date)
				{
					while (!patterns_query.isDone() && patterns_query.executeStep())
					{
						std::chrono::system_clock::rep time_count = patterns_query.getColumn(0);
						std::chrono::system_clock::duration duration_since_epoch(time_count);

						next_pattern = (EPattern)patterns_query.getColumn(1).getInt64();
						next_pattern_date = std::chrono::system_clock::time_point(duration_since_epoch);
						if (next_pattern_date >= date)
						{
							break;
						}
					}
				}

				if (next_pattern_date == date)
				{
					candle_data.m_patterns.push_back(next_pattern);	
				}
				else
				{
					candle_data.m_patterns.push_back(EPattern::None);
				}


				if (next_strategy_date < date)
				{
					while (!strategies_query.isDone() && strategies_query.executeStep())
					{
						std::chrono::system_clock::rep time_count = strategies_query.getColumn(0);
						std::chrono::system_clock::duration duration_since_epoch(time_count);

						next_strategy = strategies_query.getColumn(1).getInt64();
						next_strategy_date = std::chrono::system_clock::time_point(duration_since_epoch);

						if (next_strategy_date >= date)
						{
							break;
						}
					}
				}

				if (next_strategy_date == date)
				{
					candle_data.m_strategies.push_back(next_strategy);
				}
				else
				{
					candle_data.m_strategies.push_back(0);
				}
				

				++count;
				if (days >= 0 && count >= days)
				{
					break;
				}
			}




			m_candles_data->SetDataReady(true);

			m_is_memory_in_sync = true;
			is_success = true;
		}
		catch (std::exception& e)
		{
			is_success = false;
			Log::GetInstance().Write(std::format("ERROR: LoadCandleDataToMemory: SQLite exception: {}", e.what()));

			// Database might be locked by another thread. Wait for a bit and try again.
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}






void Security::LoadCandleDataToMemoryAsync()
{
	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	std::function<void()> load_candle_data = std::bind(&Security::LoadCandleDataToMemory, this, -1);
	std::function<void()> generate_news_points = std::bind(&Security::GenerateNewsPoints, this);

	owning_tradinator_core_thread->GetAsyncTaskManager()->AddTask(std::move(std::make_unique<AsyncTask>(
		std::format("Loading candle data for {}", m_symbol),
		load_candle_data,
		generate_news_points,
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
			//Log::GetInstance().Write(std::format("ERROR: UpdateSecuritySkeletonData: SQLite exception: {}", e.what()));

			// Database might be locked by another thread. Wait for a bit and try again.
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}




void Security::GenerateNewsPoints()
{
	m_news_points_data->SetDataReady(false);

	const CandlesData& candles_data = m_candles_data->GetData();

	for (uint64_t i = 0; i < candles_data.m_dates.size(); ++i)
	{
		EPattern pattern = TradinatorCoreSpace::Utils::GetPatternFrom(candles_data.m_patterns[i]);
		if (pattern != EPattern::None && pattern != EPattern::Max)
		{
			NewsPoint news_point(this->shared_from_this());
			news_point.m_date_range = Pattern::GetPatternRangeAt(pattern, i);
			news_point.m_pattern = pattern;

			m_news_points_data->GetAsyncDataCopy().emplace_back(std::move(news_point));
		}
	}

	m_news_points_data->SetDataReady(true);
}



std::unique_ptr<AsyncTask> Security::GetGenerateNewsPointsTask()
{
	std::function<void()> generate_news_points = [&]()
		{
			/*LoadCandleDataToMemory(50);

			const CandlesData& candles_data = m_candles_data->GetData();

			for (uint64_t i = 0; i < candles_data.m_dates.size(); ++i)
			{
				EPattern pattern = TradinatorCoreSpace::Utils::GetPatternFrom(candles_data.m_patterns[i]);
				if (pattern != EPattern::None && pattern != EPattern::Max)
				{
					NewsPoint news_point(this->shared_from_this());
					news_point.m_date_range = Pattern::GetPatternRangeAt(pattern, i);
					news_point.m_pattern = pattern;

					m_news_points_data->GetAsyncDataCopy().emplace_back(std::move(news_point));
				}
			}

			UnloadCandleDataFromMemory();*/

		};


	return std::make_unique<AsyncTask>(
		std::format("Analysing candle data of {}", Symbol()),
		[&]()
		{
			//m_news_points_data->SetDataReady(false);
		},
		generate_news_points,
		[&]()
		{
			//m_news_points_data->SetDataReady(true);
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