#include "Application/TradinatorSettings.h"

#include <fstream>

#include "Utils.h"
#include "Utils/Utils.h"
#include "Patterns/Pattern.h"

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

		TradinatorCoreSpace::Utils::SetMaxParallelDownloads(m_settings["MaxParallelDownloads"].asUInt64());
		TradinatorCoreSpace::Utils::SetMaxParallelAnalysis(m_settings["MaxParallelAnalysis"].asUInt64());
		TradinatorCoreSpace::Utils::SetReadWriteBatchSize(m_settings["ReadWriteBatchSize"].asUInt64());

		std::vector<std::unique_ptr<Pattern>> patterns = TradinatorCoreSpace::Utils::GetAvailablePatterns();
		for (std::unique_ptr<Pattern>& pattern : patterns)
		{
			std::string pattern_name = TradinatorCoreSpace::Utils::GetPatternShortDescription(pattern->PatternType());
			m_pattern_visbility[pattern->PatternType()] = m_settings["PatternVisibility"][pattern_name].asBool();
		}

		EvulateSettings();
	}
}

void TradinatorSettings::SaveSettings()
{
	std::ofstream output_file(_SETTINGS_FILE_);

	m_settings["MaxParallelDownloads"] = TradinatorCoreSpace::Utils::GetMaxParallelDownloads();
	m_settings["MaxParallelAnalysis"] = TradinatorCoreSpace::Utils::GetMaxParallelAnalysis();
	m_settings["ReadWriteBatchSize"] = TradinatorCoreSpace::Utils::GetReadWriteBatchSize();

	std::vector<std::unique_ptr<Pattern>> patterns = TradinatorCoreSpace::Utils::GetAvailablePatterns();
	for (std::unique_ptr<Pattern>& pattern : patterns)
	{
		std::string pattern_name = TradinatorCoreSpace::Utils::GetPatternShortDescription(pattern->PatternType());
		m_settings["PatternVisibility"][pattern_name] = m_pattern_visbility[pattern->PatternType()];
	}

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


bool TradinatorSettings::GetPatternVisibility(EPatternType type)
{
	auto itr = m_pattern_visbility.find(type);
	if (itr != m_pattern_visbility.end())
	{
		return (*itr).second;
	}

	return true;
}

void TradinatorSettings::SetPatternVisbility(EPatternType type, bool is_visible)
{
	m_pattern_visbility[type] = is_visible;
}