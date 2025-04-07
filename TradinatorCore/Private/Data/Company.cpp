#include "Data/Company.h"



Company::Company()
	: m_symbol()
	, m_name()
	, m_isin_number()
	, m_date_of_listing()
{

}

// copy and move sementics
Company::Company(const Company& other)
	: m_symbol(other.m_symbol)
	, m_name(other.m_name)
	, m_isin_number(other.m_isin_number)
	, m_date_of_listing(other.m_date_of_listing)
{
}

Company::Company(Company&& other) noexcept
	: m_symbol(std::move(other.m_symbol))
	, m_name(std::move(other.m_name))
	, m_isin_number(std::move(other.m_isin_number))
	, m_date_of_listing(std::move(other.m_date_of_listing))
{

}

Company& Company::operator = (const Company& other)
{
	m_symbol = other.m_symbol;
	m_name = other.m_name;
	m_isin_number = other.m_isin_number;
	m_date_of_listing = other.m_date_of_listing;

	return *this;
}

Company& Company::operator = (Company&& other) noexcept
{
	m_symbol = std::move(other.m_symbol);
	m_name = std::move(other.m_name);
	m_isin_number = std::move(other.m_isin_number);
	m_date_of_listing = std::move(other.m_date_of_listing);

	return *this;
}

bool Company::operator == (const Company& other) const
{
	return m_symbol == other.m_symbol;
}

bool Company::operator != (const Company& other) const
{
	return !(operator==(other));
}