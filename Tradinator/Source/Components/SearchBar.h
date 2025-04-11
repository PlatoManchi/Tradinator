#pragma once

#include "Components/AutoCompleteInputText.h"

class SearchBar
{
public:

	void Init();
	void Begin();
	void Show();
	void Shutdown();

private:
	AutoCompleteInputText m_autocomplete;
};

