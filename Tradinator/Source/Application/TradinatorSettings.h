#pragma once

#include <string>

#include "json/json.h"

class TradinatorSettings
{
public:
	static TradinatorSettings& Get();

	bool IsValid() const;
	
	std::string GetWorkingFolder() const;
	void SetWorkingFolder(std::string working_folder);
	
	Json::Value GetAllOpenedWindowsStatus() const;
	void SetAllOpenedWindowsStatus(Json::Value status);

private:
	TradinatorSettings();
	~TradinatorSettings();

	void LoadSettings();
	void SaveSettings();

	void EvulateSettings();

	Json::Value m_settings;
	bool m_valid_settings;
	
	std::string m_working_folder;



	static std::string _SETTINGS_FILE_;
	static TradinatorSettings _TRADINATOR_SETTINGS_;
};

