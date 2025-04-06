#pragma once

#include "Company.h"

#include <cstdint>

class Equity : public Company
{
public:
	// 
	void FromString(std::string str);

	inline std::string series() const { return m_series; }
	inline uint32_t paid_up_value() const { return m_paid_up_value; }
	inline uint32_t market_lot() const { return m_market_lot; }
	inline uint32_t face_value() const { return m_face_value; }

	std::string ToString() const;
protected:
	void ReadFromStream(std::istream& stream);
	void WriteToFile(std::ofstream& stream);

	std::string m_series;
	uint32_t m_paid_up_value;
	uint32_t m_market_lot;
	uint32_t m_face_value;
	
	friend std::ofstream& operator << (std::ofstream& stream, Equity& equity);
	friend std::ostream& operator << (std::ostream& stream, Equity& equity);
	friend std::istream& operator >> (std::istream& stream, Equity& equity);
};

std::ofstream& operator << (std::ofstream& stream, Equity& equity);
std::ostream& operator << (std::ostream& stream, Equity& equity);
std::istream& operator >> (std::istream& stream, Equity& equity);