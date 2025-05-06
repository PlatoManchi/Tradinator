#include "Indicators/Indicator.h"

Indicator::Indicator()
	: m_length(0)
{

}

Indicator::Indicator(size_t length)
	: m_length(length)
{

}


Indicator::Indicator(std::weak_ptr<Security> security, size_t length)
	: Indicator(length)
{
	m_security = security;
}