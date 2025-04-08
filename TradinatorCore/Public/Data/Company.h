#pragma once

#include <string>
#include <chrono>

class Company
{
public:
	Company();

	// copy and move sementics
	Company(const Company& other) = default;
	Company(Company&& other) noexcept = default;
	Company& operator = (const Company& other) = default;
	Company& operator = (Company&& other) noexcept = default;

	inline std::string symbol() const { return m_symbol; }
	inline std::string name() const { return m_name; }
	inline std::string isin_number() const { return m_isin_number; }
	inline std::chrono::system_clock::time_point date_of_listing() const { return m_date_of_listing; }


	bool operator == (const Company& other) const;
	bool operator != (const Company& other) const;

protected:

	std::string m_symbol;
	std::string m_name;
	std::string m_isin_number;
	std::chrono::system_clock::time_point m_date_of_listing;
};

