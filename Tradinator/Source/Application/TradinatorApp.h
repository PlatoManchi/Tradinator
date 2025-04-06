#pragma once

#include "imgui.h"

#include "TradinatorCore.h"

#include "AutoAnalysisUpdateWindow.h"
#include "DashboardWindow.h"

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

	ImGuiID m_root_docksapce_id;
	ImGuiID m_dock_id_center;
	ImGuiID m_dock_id_right;

	std::shared_ptr<TradinatorCore> m_tradinator_core;
};