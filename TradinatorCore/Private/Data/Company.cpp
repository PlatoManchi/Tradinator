#include "Data/Company.h"


bool Company::operator == (const Company& other) const
{
	return m_symbol == other.m_symbol;
}

bool Company::operator != (const Company& other) const
{
	return !(operator==(other));
}