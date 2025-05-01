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

class Counter : public Company
{
public:
	Counter();

	// copy and move sementics
	Counter(const Counter& other) = default;
	Counter(Counter&& other) noexcept = default;
	Counter& operator = (const Counter& other) = default;
	Counter& operator = (Counter&& other) noexcept = default;

	bool IsHistoricalCandleDataOutDated() const;
	std::unique_ptr<AsyncTask> GetDownloadLatestCandleDataTask(std::function<void()> callback);
	void InsertRawDataToDatabase();

	void LoadCandleDataToMemory();
	void UnloadCandleDataFromMemory();

	void FromString(std::string str);

	inline std::string Series() const { return m_series; }
	inline uint32_t PaidUpValue() const { return m_paid_up_value; }
	inline uint32_t MarketLot() const { return m_market_lot; }
	inline uint32_t FaceValue() const { return m_face_value; }

	inline std::shared_ptr<const AsyncData<CandleDataMapType>> GetCandleData() const { return m_candle_data; }

	inline bool IsCandleDataReady() const { return m_candle_data->IsDataReady(); }
	inline void SetOwningMarket(std::weak_ptr<Market> parent) { m_owning_market = parent; }
	inline void SetOwningTradinatorCoreThread(std::weak_ptr<TradinatorCoreThread> owning_tradinator_core_thread) { m_owning_tradinator_core_thread = owning_tradinator_core_thread; }

	// true if candle data in memory is out of date with what is in local database.
	// this can happen if new data is downloaded while candle data is being used.
	inline bool IsMemoryInSync() const { return m_is_memory_in_sync; }

	std::string ToString() const;

	void SetCachedLatestCandleDate(std::chrono::system_clock::time_point time)
	{
		m_cached_latest_candle_date = time;
		m_is_latest_date_dirty = false;
	}
	

protected:
	// -----------------------------------------------
	inline std::string GetProcessedHistoricalDataFilePath() const;
	bool DoesProcessedHistoricalDataExist() const;
	
	std::chrono::system_clock::time_point GetLastCandleDataDate() const;
	
	void WriteCandleToDatabase(SQLite::Database& db, const Json::Value& candle);
	void WriteCandleToDatabaseAndLoadToMemory(SQLite::Database& db, const Json::Value& candle);

	void UpdateLatestCandleDataDate();

	inline std::string GetTableName() const { return std::format("{}_{}", m_symbol, m_isin_number); }
	// -----------------------------------------------
	
	
	std::string m_series;
	uint32_t m_paid_up_value;
	uint32_t m_market_lot;
	uint32_t m_face_value;
	
	// market this counter belongs to
	std::weak_ptr<Market> m_owning_market;

	std::weak_ptr<TradinatorCoreThread> m_owning_tradinator_core_thread;
	
	// Don't use this in other async/threads. SQLite works in multithread only when using different connections
	// per thread.
	SQLite::Database m_database_connection;

	// Candle data sorted from latest to oldest
	std::shared_ptr<AsyncData<CandleDataMapType>> m_candle_data;

	// Cached latest local candle date
	mutable std::chrono::system_clock::time_point m_cached_latest_candle_date;
	mutable bool m_is_latest_date_dirty;

	// true if candle data in memory is out of date with what is in local database.
	// this can happen if new data is downloaded while candle data is being used.
	bool m_is_memory_in_sync;

	// Check to make sure there are no double update tasks.
	// we want to be able to acecss previous historical data that is stored locally while
	// new data is being downloaded in parallel. 
	std::mutex m_counter_mutex;
	bool m_is_downloading = false;
	bool m_is_inserting = false;
};
