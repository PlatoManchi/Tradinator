#pragma once

#include "Company.h"

#include <cstdint>
#include <functional>
#include <map>

#include "Data/AsyncData.h"
#include "Data/Candle.h"

typedef AsyncData<std::map<std::chrono::system_clock::time_point, Candle, std::greater<std::chrono::system_clock::time_point>>> AsyncCandleData;

class Market;

class Security : public Company
{
public:
	Security();

	// copy and move sementics
	Security(const Security& other) = default;
	Security(Security&& other) noexcept = default;
	Security& operator = (const Security& other) = default;
	Security& operator = (Security&& other) noexcept = default;

	void SetParentMarket(std::weak_ptr<Market> parent);
	void DownloadSecurityData(std::function<void()> callback);

	bool DoesRawHistoricalDataExist() const;
	inline std::string GetRawHistoricalDataFilePath() const;
	// 
	void FromString(std::string str);

	inline std::string series() const { return m_series; }
	inline uint32_t paid_up_value() const { return m_paid_up_value; }
	inline uint32_t market_lot() const { return m_market_lot; }
	inline uint32_t face_value() const { return m_face_value; }

	std::string ToString() const;

protected:
	void LoadHistoricalDataIfExists();
	// tmp_file_path - save downloaded data at this location for processing
	void DownloadDailyData(std::string tmp_file_path);

	void ProcessDownloadedData(std::string tmp_file_path);


	void ReadFromStream(std::istream& stream);
	void WriteToFile(std::ofstream& stream);

	std::string m_series;
	uint32_t m_paid_up_value;
	uint32_t m_market_lot;
	uint32_t m_face_value;
	
	std::weak_ptr<Market> m_parent_market;
	std::function<void()> m_callback;
	
	// Candle data sorted from latest to oldest
	AsyncCandleData m_candle_data;



	// overloaded stream operator
	friend std::ofstream& operator << (std::ofstream& stream, Security& security);
	friend std::ostream& operator << (std::ostream& stream, Security& security);
	friend std::istream& operator >> (std::istream& stream, Security& security);
};

std::ofstream& operator << (std::ofstream& stream, Security& security);
std::ostream& operator << (std::ostream& stream, Security& security);
std::istream& operator >> (std::istream& stream, Security& security);