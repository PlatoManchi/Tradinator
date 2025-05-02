#pragma once

#include <string>
#include <unordered_map>

#include "json/json.h"

#include "Patterns/Pattern.h"

class TradinatorSettings
{
public:
	static TradinatorSettings& Get();

	bool IsValid() const;
	
	std::string GetWorkingFolder() const;
	void SetWorkingFolder(std::string working_folder);
	
	Json::Value GetAllOpenedWindowsStatus() const;
	void SetAllOpenedWindowsStatus(Json::Value status);

	bool GetPatternVisibility(EPatternType type);
	void SetPatternVisbility(EPatternType type, bool is_visible);

private:
	TradinatorSettings();
	~TradinatorSettings();

	void LoadSettings();
	void SaveSettings();

	void EvulateSettings();

	Json::Value m_settings;
	bool m_valid_settings;
	
	std::string m_working_folder;

	std::unordered_map<EPatternType, bool> m_pattern_visbility;

	static std::string _SETTINGS_FILE_;
	static TradinatorSettings _TRADINATOR_SETTINGS_;
};

