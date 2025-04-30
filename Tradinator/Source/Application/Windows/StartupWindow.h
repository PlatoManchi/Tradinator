#pragma once

#include "imgui.h"

class StartupWindow
{
public:
	StartupWindow();


	void Init();
	void Begin();
	bool Show();
	void Shutdown();

private:
	bool m_working_folder_exists;
	bool m_nse_market_security_list_exist;

	bool m_all_settings_valid = false;

	ImVec2 m_save_settings_button_size;

	char m_working_folder_path[512];
};

