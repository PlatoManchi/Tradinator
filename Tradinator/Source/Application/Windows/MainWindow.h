#pragma once

#include "Components/AutoCompleteInputText.h"

class MainWindow
{
public:
	void Init();
	void Begin();
	void Show();
	void Shutdown();

private:
	AutoCompleteInputText m_autocomplete;
};