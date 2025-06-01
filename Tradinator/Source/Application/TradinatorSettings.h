#pragma once

#include <string>
#include <unordered_map>

#include "json/json.h"

#include "TradinatorCoreTypes.h"

class TradinatorSettings
{
public:
	struct PinnedSecurity
	{
		std::string symbol;
		std::string isin_number;
	};

	static TradinatorSettings& Get();

	bool IsValid() const;
	
	std::string GetWorkingFolder() const;
	void SetWorkingFolder(std::string working_folder);
	
	Json::Value GetAllOpenedWindowsStatus() const;
	void SetAllOpenedWindowsStatus(Json::Value status);

	bool GetPatternVisibility(EPattern type);
	void SetPatternVisbility(EPattern type, bool is_visible);

	bool GetStrategyVisibility(EStrategy strategy);
	void SetStrategyVisibility(EStrategy strategy, bool is_visible);

	void LoadSettings();
	void SaveSettings();

	const std::vector<PinnedSecurity>& GetPinnedSecuritiesIsinNumbers() const { return m_pinned_securities_isin_numbers; }
	void AddPinnedSecuritiesIsinNumber(PinnedSecurity isin_number);
	void RemovePinnedSecuritiesIsinNumber(PinnedSecurity isin_number);

private:
	TradinatorSettings();

	void EvulateSettings();

	Json::Value m_settings;
	bool m_valid_settings;
	
	std::string m_working_folder;
	std::vector<PinnedSecurity> m_pinned_securities_isin_numbers;

	std::unordered_map<EPattern, bool> m_pattern_visbility;
	std::unordered_map<EStrategy, bool> m_strategy_visbility;

	static std::string _SETTINGS_FILE_;
	static TradinatorSettings _TRADINATOR_SETTINGS_;
};

