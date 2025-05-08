#include "News/News.h"


NewsPoint::NewsPoint()
	: m_security(nullptr)
	, m_date_range()
	, m_pattern()
{ }


NewsPoint::NewsPoint(std::shared_ptr<Security> security)
	: m_security(security)
	, m_date_range()
	, m_pattern()
{

}