#include "News/News.h"


NewsPoint::NewsPoint()
	: m_counter(nullptr)
	, m_date_range()
	, m_pattern()
{ }


NewsPoint::NewsPoint(std::shared_ptr<Counter> counter)
	: m_counter(counter)
	, m_date_range()
	, m_pattern()
{

}