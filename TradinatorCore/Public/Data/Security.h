#pragma once

#include "Company.h"

#include <cstdint>
#include <functional>
#include <map>
#include <chrono>

#include "json/json.h"
#include "SQLiteCpp/SQLiteCpp.h"

#include "Data/AsyncData.h"
#include "Data/Candle.h"
#include "News/News.h"


class Market;
class TradinatorCoreThread;
class AsyncTask;
class DownloadTask;
class Indicator;


/*
Candle data is stored in this format
Candles in ascending order

size_t - number of candles
1-1-2001 open high low close volume open_interest   
2-1-2001 open high low close volume open_interest
3-1-2001 open high low close volume open_interest
....

*/

class Security : public Company, public std::enable_shared_from_this<Security>
{
public:
	Security();

	// copy and move sementics
	Security(const Security& other) = default;
	Security(Security&& other) noexcept = default;
	Security& operator = (const Security& other) = default;
	Security& operator = (Security&& other) noexcept = default;

	bool IsHistoricalCandleDataOutDated() const;
	std::unique_ptr<AsyncTask> GetDownloadLatestCandleDataTask();
	std::unique_ptr<AsyncTask> GetGenerateNewsPointsTask();
	void ReadFromRawFileToMemory();
	void InsertRawDataToDatabase();

	void LoadCandleDataToMemoryAsync();
	void UnloadCandleDataFromMemory();

	void FromString(std::string str);

	inline std::string Series() const { return m_series; }
	inline uint32_t PaidUpValue() const { return m_paid_up_value; }
	inline uint32_t MarketLot() const { return m_market_lot; }
	inline uint32_t FaceValue() const { return m_face_value; }

	inline std::shared_ptr<const AsyncData<CandleDataMapType>> GetCandleData() const { return m_candle_data; }
	inline std::shared_ptr<const AsyncData<CandlesData>> GetCandlesData() const { return m_candles_data; }
	inline std::shared_ptr<const AsyncData<NewsPointMapType>> GetNewsPointsData() const { return m_news_points_data; }

	inline bool IsCandleDataReady() const { return m_candle_data->IsDataReady(); }
	inline void SetOwningMarket(std::weak_ptr<Market> parent) { m_owning_market = parent; }
	inline void SetOwningTradinatorCoreThread(std::weak_ptr<TradinatorCoreThread> owning_tradinator_core_thread) { m_owning_tradinator_core_thread = owning_tradinator_core_thread; }

	// true if candle data in memory is out of date with what is in local database.
	// this can happen if new data is downloaded while candle data is being used.
	inline bool IsMemoryInSync() const { return m_is_memory_in_sync; }

	// If true, will keep the candle data in memory even if requested to unload it.
	// Many tasks like pattern analysis will load the data and unload the data when they are done.
	// This will prevent the data from unloading if set to true;
	void SetLockInMemory(bool should_lock_in_memory) { m_lock_in_memory = should_lock_in_memory; }

	std::string ToString() const;

	void SetCachedLatestCandleDate(std::chrono::system_clock::time_point time)
	{
		m_cached_latest_candle_date = time;
		m_is_latest_date_dirty = false;
	}
	
	void SetCandleCount(size_t candle_count)
	{
		m_candle_count = candle_count;
	}

protected:
	// -----------------------------------------------
	inline std::string GetProcessedHistoricalDataFilePath() const;
	bool DoesProcessedHistoricalDataExist() const;
	
	std::chrono::system_clock::time_point GetLastCandleDataDate() const;

	void UpdateSecuritySkeletonData();
	void LoadCandleDataToMemory();
	std::unique_ptr<AsyncTask> LoadCandleDataToMemoryTask();
	inline std::string GetTableName() const { return std::format("{}_{}", m_symbol, m_isin_number); }
	// -----------------------------------------------
	
	
	std::string m_series;
	uint32_t m_paid_up_value;
	uint32_t m_market_lot;
	uint32_t m_face_value;

	// Used to pre reserve memory when needed
	size_t m_candle_count;

	// Raw json from reading the downloaded data
	Json::Value m_raw_downloaded_data;

	// market this security belongs to
	std::weak_ptr<Market> m_owning_market;

	std::weak_ptr<TradinatorCoreThread> m_owning_tradinator_core_thread;
	
	// Don't use this in other async/threads. SQLite works in multithread only when using different connections
	// per thread.
	SQLite::Database m_database_connection;

	// Candle data sorted from latest to oldest
	std::shared_ptr<AsyncData<CandleDataMapType>> m_candle_data;

	std::shared_ptr<AsyncData<CandlesData>> m_candles_data;

	// News points sorted from latest to oldest
	std::shared_ptr<AsyncData<NewsPointMapType>> m_news_points_data;

	// Cached latest local candle date
	mutable std::chrono::system_clock::time_point m_cached_latest_candle_date;
	mutable bool m_is_latest_date_dirty;

	// true if candle data in memory is out of date with what is in local database.
	// this can happen if new data is downloaded while candle data is being used.
	bool m_is_memory_in_sync;

	// If this is true, will keep candle data in memory
	bool m_lock_in_memory;

	// Check to make sure there are no double update tasks.
	// we want to be able to acecss previous historical data that is stored locally while
	// new data is being downloaded in parallel. 
	std::mutex m_security_mutex;
	bool m_is_downloading = false;
	bool m_is_inserting = false;
};
