#pragma once

#include <memory>

#include "Components/AutoCompleteInputText.h"

class TradinatorCore;

class MainWindow
{
public:
	void Init(std::shared_ptr<TradinatorCore> tradinator_core);
	void Begin();
	void Show();
	void Shutdown();

private:
	std::shared_ptr<TradinatorCore> m_tradinator_core;

	AutoCompleteInputText m_autocomplete;
};