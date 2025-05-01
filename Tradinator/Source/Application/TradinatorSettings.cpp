#include "Application/TradinatorSettings.h"

#include <fstream>

#include "Utils.h"

std::string TradinatorSettings::_SETTINGS_FILE_ = "Settings.json";
TradinatorSettings TradinatorSettings::_TRADINATOR_SETTINGS_ = TradinatorSettings();


TradinatorSettings& TradinatorSettings::Get()
{
	return _TRADINATOR_SETTINGS_;
}

TradinatorSettings::TradinatorSettings()
	: m_valid_settings(false)
{
	LoadSettings();
}

TradinatorSettings::~TradinatorSettings()
{
	SaveSettings();
}

bool TradinatorSettings::IsValid() const
{
	return m_valid_settings;
}

void TradinatorSettings::EvulateSettings()
{
	m_valid_settings = !m_settings.isNull() && TradinatorAppSpace::Utils::DoesDirectoryExist(GetWorkingFolder());
}

std::string TradinatorSettings::GetWorkingFolder() const
{
	return m_settings["WorkingFolder"].asString();
}

void TradinatorSettings::SetWorkingFolder(std::string working_folder)
{
	m_settings["WorkingFolder"] = working_folder;

	EvulateSettings();
}

void TradinatorSettings::LoadSettings()
{
	std::ifstream input_file(_SETTINGS_FILE_, std::ifstream::binary);
	if (input_file.good())
	{
		input_file >> m_settings;
		input_file.close();

		EvulateSettings();
	}
}

void TradinatorSettings::SaveSettings()
{
	std::ofstream output_file(_SETTINGS_FILE_);

	output_file << m_settings;
	output_file.close();
}

Json::Value TradinatorSettings::GetAllOpenedWindowsStatus() const
{
	return m_settings["OpenedWindows"];
}

void TradinatorSettings::SetAllOpenedWindowsStatus(Json::Value status)
{
	m_settings["OpenedWindows"] = status;
}