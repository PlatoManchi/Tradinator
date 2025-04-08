#include "Data/Company.h"



Company::Company()
	: m_symbol()
	, m_name()
	, m_isin_number()
	, m_date_of_listing()
{

}

bool Company::operator == (const Company& other) const
{
	return m_symbol == other.m_symbol;
}

bool Company::operator != (const Company& other) const
{
	return !(operator==(other));
}