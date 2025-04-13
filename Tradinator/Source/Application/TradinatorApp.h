#pragma once

#include <map>

#include "imgui.h"

#include "TradinatorCore.h"

#include "AutoAnalysisUpdateWindow.h"
#include "DashboardWindow.h"
#include "Windows/MainWindow.h"
#include "Windows/CounterWindow.h"
#include "Components/SecuritiesSearchBar.h"
#include "Components/StatusBar.h"

class TradinatorApp final
{
public:
	TradinatorApp();

	void Init();
	void Begin();
	void ShowApp();
	void Shutdown();

private:
	void ShowMainMenu();
	void ShowMainMenu_File();


	AutoAnalysisUpdateWindow m_audo_analysis_update_window;
	DashboardWindow m_dashboard_window;

	SecuritiesSearchBar m_securities_search_bar;
	MainWindow m_main_windows;
	StatusBar m_status_bar;

	ImGuiID m_root_docksapce_id;
	ImGuiID m_dashboard_dockspace_id;
	ImGuiID m_notification_dockspace_id;

	ImGuiID m_main_window_dockspace_id;
	ImGuiID m_search_bar_dockspace_id;
	ImGuiID m_status_bar_dockspace_id;
	
	std::shared_ptr<TradinatorCore> m_tradinator_core;
	std::map<std::string, std::shared_ptr<CounterWindow>> m_counter_windows;
};