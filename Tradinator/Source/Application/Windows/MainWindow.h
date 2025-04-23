#pragma once

#include <memory>

#include "Components/AutoCompleteInputText.h"

class TradinatorCore;
class TradinatorApp;

class MainWindow
{
public:
	MainWindow(TradinatorApp& tradinator_app);
	
	void Init(std::shared_ptr<TradinatorCore> tradinator_core);
	void Begin();
	void Show();
	void Shutdown();

private:
	std::shared_ptr<TradinatorCore> m_tradinator_core;

	AutoCompleteInputText m_autocomplete;

	TradinatorApp& m_tradinator_app;
};