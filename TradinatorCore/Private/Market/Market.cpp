#include "Market/Market.h"

#include <filesystem>

#include "TradinatorCore.h"

Market::Market(std::shared_ptr<TradinatorCore> tradinator_core)
	: m_tradinator_core(tradinator_core)
{

}

void Market::CreateFolderStructure() const
{
	std::filesystem::create_directory(m_tradinator_core->GetDataFolderPath() + "/" + _RAW_DATA_FOLDER_);
	std::filesystem::create_directory(GetRawDataFolder());

	std::filesystem::create_directory(m_tradinator_core->GetDataFolderPath() + "/" + _PROCESSED_DATA_FOLDER_);
	std::filesystem::create_directory(GetProcessedDataFolder());
}

std::string Market::GetRawDataFolder() const
{
	return m_tradinator_core->GetDataFolderPath() + "/" + _RAW_DATA_FOLDER_ + "/" + GetMarketName();
}

std::string Market::GetProcessedDataFolder() const
{
	return m_tradinator_core->GetDataFolderPath() + "/" + _PROCESSED_DATA_FOLDER_ + "/" + GetMarketName();
}