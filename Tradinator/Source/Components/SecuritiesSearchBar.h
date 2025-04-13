#pragma once

#include <memory>

#include "Components/AutoCompleteInputText.h"

class Counter;
class TradinatorCore;

class CounterAutoCompleteItem final : public IAutoCommpleteItem
{
public:
	CounterAutoCompleteItem(std::shared_ptr<Counter> counter);

	virtual bool CanPassFilter(const char* search_string) override;
	virtual const std::string& GetItemDescription() override;
	inline std::shared_ptr<Counter> GetCounter() const { return m_counter; }

private:
	std::shared_ptr<Counter> m_counter;
	std::string m_cached_description;
};

class SecuritiesSearchBar
{
public:

	void Init(std::shared_ptr<TradinatorCore> tradinator_core);
	void Begin();
	std::shared_ptr<Counter> Show();
	void Shutdown();

private:
	std::shared_ptr<TradinatorCore> m_tradinator_core;
	AutoCompleteInputText m_autocomplete;
};

