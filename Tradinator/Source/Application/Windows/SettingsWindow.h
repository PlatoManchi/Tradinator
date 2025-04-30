#pragma once

#include "imgui.h"

class SettingsWindow
{
public:
	SettingsWindow();


	void Init();
	void Begin();
	bool Show();
	void Shutdown();

private:
	size_t m_max_parallel_downloads;
	size_t m_max_parallel_process;
	ImVec2 m_save_settings_button_size;
};

