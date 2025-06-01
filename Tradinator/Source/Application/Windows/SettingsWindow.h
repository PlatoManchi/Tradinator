#pragma once

#include <memory>
#include <vector>

#include "imgui.h"

class TradinatorCore;
class Pattern;
class Strategy;

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

	std::vector<std::unique_ptr<Pattern>> m_patterns;
	std::vector<std::unique_ptr<Strategy>> m_strategies;

	size_t m_max_parallel_downloads;
	size_t m_max_parallel_process;
	ImVec2 m_save_settings_button_size;
};

