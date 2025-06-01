#include "Indicators/Indicator.h"

Indicator::Indicator()
	: m_source(EIndicatorSource::E_CLOSE)
	, m_length(0)
{

}

Indicator::Indicator(EIndicatorSource source)
	: m_source(source)
	, m_length(0)
{

}

Indicator::Indicator(uint64_t length)
	: m_source(EIndicatorSource::E_CLOSE)
	, m_length(length)
{

}

Indicator::Indicator(EIndicatorSource source, uint64_t length)
	: m_source(source)
	, m_length(length)
{

}

Indicator::Indicator(uint64_t length, std::weak_ptr<Security> security)
	: m_source(EIndicatorSource::E_CLOSE)
	, m_length(length)
	, m_security(security)
{
	
}

Indicator::Indicator(EIndicatorSource source, uint64_t length, std::weak_ptr<Security> security)
	: m_source(source)
	, m_length(length)
	, m_security(security)
{

}


bool Indicator::operator==(const Indicator& other) const
{
	return
		m_source == other.m_source &&
		m_length == other.m_length &&
		IndicatorType() == other.IndicatorType();
}