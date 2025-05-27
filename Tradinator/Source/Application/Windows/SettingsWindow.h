#pragma once

#include <memory>

#include "imgui.h"

class TradinatorCore;

class SettingsWindow
{
public:
	SettingsWindow();


	void Init(std::shared_ptr<TradinatorCore> tradinator_core);
	void Begin();
	bool Show();
	void Shutdown();

private:
	std::shared_ptr<TradinatorCore> m_tradinator_core;

	size_t m_max_parallel_downloads;
	size_t m_max_parallel_process;
	ImVec2 m_save_settings_button_size;
};

