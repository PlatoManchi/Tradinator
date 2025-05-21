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

void TradinatorSettings::AddPinnedSecuritiesIsinNumber(PinnedSecurity security_details)
{
	m_pinned_securities_isin_numbers.push_back(security_details);
}

void TradinatorSettings::RemovePinnedSecuritiesIsinNumber(PinnedSecurity security_details)
{
	for (size_t i = 0; i < m_pinned_securities_isin_numbers.size(); ++i)
	{
		if (m_pinned_securities_isin_numbers[i].isin_number == security_details.isin_number)
		{
			m_pinned_securities_isin_numbers.erase(m_pinned_securities_isin_numbers.begin() + i);
			break;
		}
	}
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
			std::string pattern_name = pattern->Name();
			m_pattern_visbility[pattern->PatternType()] = m_settings["PatternVisibility"].find(pattern_name) ? m_settings["PatternVisibility"][pattern_name].asBool() : pattern->IsDefaultVisible();
		}

		m_pinned_securities_isin_numbers.clear();
		Json::Value::ArrayIndex count = m_settings["PinnedSecurities"].size();
		for (Json::Value::ArrayIndex i = 0; i < count; ++i)
		{
			m_pinned_securities_isin_numbers.push_back({ m_settings["PinnedSecurities"][i]["Symbol"].asString(), m_settings["PinnedSecurities"][i]["ISIN_Number"].asString() });
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
		std::string pattern_name = pattern->Name();
		m_settings["PatternVisibility"][pattern_name] = m_pattern_visbility[pattern->PatternType()];
	}

	m_settings["PinnedSecurities"].clear();
	for (PinnedSecurity& security : m_pinned_securities_isin_numbers)
	{
		Json::Value security_value;
		security_value["Symbol"] = security.symbol;
		security_value["ISIN_Number"] = security.isin_number;

		m_settings["PinnedSecurities"].append(security_value);
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


bool TradinatorSettings::GetPatternVisibility(EPattern type)
{
	auto itr = m_pattern_visbility.find(type);
	if (itr != m_pattern_visbility.end())
	{
		return (*itr).second;
	}

	return true;
}

void TradinatorSettings::SetPatternVisbility(EPattern type, bool is_visible)
{
	m_pattern_visbility[type] = is_visible;
}