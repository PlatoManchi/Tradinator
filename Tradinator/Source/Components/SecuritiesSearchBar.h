#pragma once

#include <memory>

#include "Components/AutoCompleteInputText.h"

class Security;
class TradinatorCore;

class SecurityAutoCompleteItem final : public IAutoCommpleteItem
{
public:
	SecurityAutoCompleteItem(std::shared_ptr<Security> security);

	virtual bool CanPassFilter(const char* search_string) override;
	virtual const std::string& GetItemDescription() override;
	inline std::shared_ptr<Security> GetSecurity() const { return m_security; }

private:
	std::shared_ptr<Security> m_security;
	std::string m_cached_description;
};

class SecuritiesSearchBar
{
public:

	void Init(std::shared_ptr<TradinatorCore> tradinator_core);
	void Begin();
	std::shared_ptr<Security> Show();
	void Shutdown();

private:
	std::shared_ptr<TradinatorCore> m_tradinator_core;
	AutoCompleteInputText m_autocomplete;
};

