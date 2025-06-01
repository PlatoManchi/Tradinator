#pragma once

#include <memory>

#include "Application/Windows/DashboardWindow.h"
#include "Application/Windows/SettingsWindow.h"
#include "Application/Windows/AutoAnalysisUpdateWindow.h"
#include "Application/Windows/PinnedSecuritiesWindow.h"
#include "Components/SecuritiesSearchBar.h"
#include "Components/StatusBar.h"

class TradinatorCore;
class TradinatorApp;
class Pattern;
class Strategy;


class MainWindow
{
public:
	MainWindow(TradinatorApp& tradinator_app);
	
	void Init(std::shared_ptr<TradinatorCore> tradinator_core);
	void Begin();
	bool Show();
	void Shutdown();

private:
	void ShowMainMenu();
	void ShowMainMenu_File();


	std::shared_ptr<TradinatorCore> m_tradinator_core;

	
	SecuritiesSearchBar m_securities_search_bar;
	DashboardWindow m_dashboard_window;
	PinnedSecuritiesWindow m_pinned_securities_window;
	AutoAnalysisUpdateWindow m_auto_analysis_update_window;
	StatusBar m_status_bar;
	SettingsWindow m_settings_window;

	bool m_show_settings_window = false;
	bool m_should_exit = false;

	std::vector<std::unique_ptr<Pattern>> m_patterns;
	std::vector<std::unique_ptr<Strategy>> m_strategies;

	TradinatorApp& m_tradinator_app;
};