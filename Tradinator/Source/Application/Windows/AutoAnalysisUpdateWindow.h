#pragma once

#include "Data/AsyncData.h"
#include "News/News.h"

class TradinatorApp;

class AutoAnalysisUpdateWindow
{
public:
	AutoAnalysisUpdateWindow();

	void Init(std::shared_ptr<AsyncData<NewsPointVectorType>> news_points);
	void Begin();
	int64_t Show();
	void Shutdown();
	
private:
	std::shared_ptr<AsyncData<NewsPointVectorType>> m_news_points;
	uint64_t m_id;
	static uint64_t _ID_;
};