#pragma once

#include "Data/AsyncData.h"
#include "News/News.h"

class TradinatorApp;

class AutoAnalysisUpdateWindow
{
public:
	AutoAnalysisUpdateWindow(TradinatorApp& tradinator_app);

	void Init(std::shared_ptr<AsyncData<NewsPointVectorType>> news_points);
	void Begin();
	int64_t Show();
	void Shutdown();
	
private:
	std::shared_ptr<AsyncData<NewsPointVectorType>> m_news_points;
	TradinatorApp& m_tradinator_app;
};