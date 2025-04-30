#pragma once

#include <map>

#include "imgui.h"

#include "TradinatorCore.h"

#include "Windows/MainWindow.h"
#include "Windows/StartupWindow.h"


class Counter;
class CounterWindow;

class TradinatorApp final
{
public:
	TradinatorApp();

	void Init();
	void Begin();
	bool ShowApp();
	void Shutdown();

	void ShowCounterWindow(std::shared_ptr<Counter> counter);

private:
	bool ShowMainWindow();

	void LoadWindowsState();
	void SaveWindowsState();

	MainWindow m_main_windows;
	StartupWindow m_startup_window;

	std::shared_ptr<TradinatorCore> m_tradinator_core;
	std::map<std::string, std::shared_ptr<CounterWindow>> m_counter_windows;
};