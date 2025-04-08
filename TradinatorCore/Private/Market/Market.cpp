#include "Market/Market.h"

#include <filesystem>

#include "TradinatorCore.h"

Market::Market(std::shared_ptr<TradinatorCore> tradinator_core)
	: m_tradinator_core(tradinator_core)
	, m_securities_async_data(m_tradinator_core->GetTradinatorCoreThreadID())
{

}

std::weak_ptr<Market> Market::GetMarket() const
{
	std::vector<std::shared_ptr<Market>> markets = m_tradinator_core->GetAllMarkets();

	for (std::shared_ptr<Market>& market : markets)
	{
		if (market.get() == this)
		{
			return market;
		}
	}

	return std::weak_ptr<Market>();
}

void Market::CreateFolderStructure() const
{
	std::filesystem::create_directory(m_tradinator_core->GetDataFolderPath() + "/" + _RAW_DATA_FOLDER_);
	std::filesystem::create_directory(GetRawDataFolderPath());

	std::filesystem::create_directory(m_tradinator_core->GetDataFolderPath() + "/" + _PROCESSED_DATA_FOLDER_);
	std::filesystem::create_directory(GetProcessedDataFolderPath());
}

std::string Market::GetRawDataFolderPath() const
{
	return m_tradinator_core->GetDataFolderPath() + "/" + _RAW_DATA_FOLDER_ + "/" + GetMarketCode();
}

std::string Market::GetProcessedDataFolderPath() const
{
	return m_tradinator_core->GetDataFolderPath() + "/" + _PROCESSED_DATA_FOLDER_ + "/" + GetMarketCode();
}

std::string Market::GetRawDataFilePath() const
{
	return GetRawDataFolderPath() + "/" + GetRawDataFilePathName();
}

std::string Market::GetProcessedDataFilePath() const
{
	return GetProcessedDataFolderPath() + "/" + GetProcessedDataFileName();
}