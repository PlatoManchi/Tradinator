#pragma once

#include <memory>

class TradinatorCore;
class TradinatorApp;

class DashboardWindow 
{
public:
	DashboardWindow(TradinatorApp& tradinator_app);

	void Init(std::shared_ptr<TradinatorCore> tradinator_core);
	void Begin();
	void Show();
	void Shutdown();

protected:
	std::shared_ptr<TradinatorCore> m_tradinator_core;
	TradinatorApp& m_tradinator_app;
};