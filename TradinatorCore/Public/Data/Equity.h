#pragma once

#include "Company.h"

#include <cstdint>
#include <functional>

class Market;

class Equity : public Company
{
public:
	Equity();

	// copy and move sementics
	Equity(const Equity& other);
	Equity(Equity&& other) noexcept;
	Equity& operator = (const Equity& other);
	Equity& operator = (Equity&& other) noexcept;

	void SetParentMarket(std::weak_ptr<Market> parent);
	void LoadEquityData(std::function<void()> callback);

	bool DoesRawHistoricalDataExist() const;

	// 
	void FromString(std::string str);

	inline std::string series() const { return m_series; }
	inline uint32_t paid_up_value() const { return m_paid_up_value; }
	inline uint32_t market_lot() const { return m_market_lot; }
	inline uint32_t face_value() const { return m_face_value; }

	std::string ToString() const;

protected:

	void ProcessDownloadedData(std::string tmp_file_path);

	std::string GetRawHistoricalDataFilePath() const;

	void ReadFromStream(std::istream& stream);
	void WriteToFile(std::ofstream& stream);

	std::string m_series;
	uint32_t m_paid_up_value;
	uint32_t m_market_lot;
	uint32_t m_face_value;
	
	std::weak_ptr<Market> m_parent_market;
	std::function<void()> m_callback;
	bool m_is_equity_data_loading;

	friend std::ofstream& operator << (std::ofstream& stream, Equity& equity);
	friend std::ostream& operator << (std::ostream& stream, Equity& equity);
	friend std::istream& operator >> (std::istream& stream, Equity& equity);
};

std::ofstream& operator << (std::ofstream& stream, Equity& equity);
std::ostream& operator << (std::ostream& stream, Equity& equity);
std::istream& operator >> (std::istream& stream, Equity& equity);