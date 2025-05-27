#pragma once

#include <map>

#include "imgui.h"

#include "TradinatorCore.h"

#include "Windows/MainWindow.h"
#include "Windows/StartupWindow.h"


class Security;
class SecurityWindow;

class TradinatorApp final
{
public:
	TradinatorApp();

	void Init();
	void Begin();
	bool ShowApp();
	void Shutdown();

	std::shared_ptr<SecurityWindow> ShowSecurityWindow(std::shared_ptr<Security> security);

private:
	bool ShowMainWindow();

	void LoadWindowsState();
	void SaveWindowsState();

	MainWindow m_main_windows;
	StartupWindow m_startup_window;

	std::shared_ptr<TradinatorCore> m_tradinator_core;
	std::map<std::string, std::shared_ptr<SecurityWindow>> m_security_windows;
};