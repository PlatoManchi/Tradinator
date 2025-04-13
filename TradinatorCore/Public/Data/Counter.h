#pragma once

#include "Company.h"

#include <cstdint>
#include <functional>
#include <map>

#include "Data/AsyncData.h"
#include "Data/Candle.h"

typedef AsyncData<std::map<std::chrono::system_clock::time_point, Candle, std::greater<std::chrono::system_clock::time_point>>> AsyncCandleData;

class Market;
class TradinatorCoreThread;


class Counter : public Company
{
public:
	Counter();

	// copy and move sementics
	Counter(const Counter& other) = default;
	Counter(Counter&& other) noexcept = default;
	Counter& operator = (const Counter& other) = default;
	Counter& operator = (Counter&& other) noexcept = default;

	void LoadCandleData();
	void UnloadCandleData();
	
	void DownloadCounterData(std::function<void()> callback);

	bool DoesRawHistoricalDataExist() const;
	inline std::string GetRawHistoricalDataFilePath() const;
	// 
	void FromString(std::string str);

	inline std::string Series() const { return m_series; }
	inline uint32_t PaidUpValue() const { return m_paid_up_value; }
	inline uint32_t MarkerLot() const { return m_market_lot; }
	inline uint32_t FaceValue() const { return m_face_value; }

	inline std::shared_ptr<const AsyncCandleData> GetCandleData() const { return m_candle_data; }

	inline bool IsCandleDataReady() const { return m_candle_data->IsDataReady(); }
	inline void SetOwningMarket(std::weak_ptr<Market> parent) { m_owning_market = parent; }
	inline void SetOwningTradinatorCoreThread(std::weak_ptr<TradinatorCoreThread> owning_tradinator_core_thread) { m_owning_tradinator_core_thread = owning_tradinator_core_thread; }

	std::string ToString() const;

protected:
	void LoadHistoricalDataIfExists();
	// tmp_file_path - save downloaded data at this location for processing
	void DownloadDailyData(std::string tmp_file_path, std::function<void()> callback);

	void ProcessDownloadedData(std::string tmp_file_path, std::function<void()> callback);


	void ReadFromStream(std::istream& stream);
	void WriteToFile(std::ofstream& stream);

	std::string m_series;
	uint32_t m_paid_up_value;
	uint32_t m_market_lot;
	uint32_t m_face_value;
	
	// market this counter belongs to
	std::weak_ptr<Market> m_owning_market;

	std::weak_ptr<TradinatorCoreThread> m_owning_tradinator_core_thread;
	
	// Candle data sorted from latest to oldest
	std::shared_ptr<AsyncCandleData> m_candle_data;



	// overloaded stream operator
	friend std::ofstream& operator << (std::ofstream& stream, Counter& counter);
	friend std::ostream& operator << (std::ostream& stream, Counter& counter);
	friend std::istream& operator >> (std::istream& stream, Counter& counter);
};

std::ofstream& operator << (std::ofstream& stream, Counter& counter);
std::ostream& operator << (std::ostream& stream, Counter& counter);
std::istream& operator >> (std::istream& stream, Counter& counter);