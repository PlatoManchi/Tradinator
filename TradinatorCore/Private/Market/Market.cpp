#include "Market/Market.h"

#include <filesystem>

#include "TradinatorCore.h"

Market::Market()
	: m_securities_async_data()
{

}

void Market::CreateFolderStructure() const
{
	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	std::filesystem::create_directory(owning_tradinator_core_thread->GetDataFolderPath() + "/" + _RAW_DATA_FOLDER_);
	std::filesystem::create_directory(GetRawDataFolderPath());

	std::filesystem::create_directory(owning_tradinator_core_thread->GetDataFolderPath() + "/" + _PROCESSED_DATA_FOLDER_);
	std::filesystem::create_directory(GetProcessedDataFolderPath());
}

std::string Market::GetRawDataFolderPath() const
{
	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	return owning_tradinator_core_thread->GetDataFolderPath() + "/" + _RAW_DATA_FOLDER_ + "/" + GetMarketCode();
}

std::string Market::GetProcessedDataFolderPath() const
{
	std::shared_ptr<TradinatorCoreThread> owning_tradinator_core_thread = m_owning_tradinator_core_thread.lock();
	assert(owning_tradinator_core_thread);

	return owning_tradinator_core_thread->GetDataFolderPath() + "/" + _PROCESSED_DATA_FOLDER_ + "/" + GetMarketCode();
}

std::string Market::GetRawDataFilePath() const
{
	return GetRawDataFolderPath() + "/" + GetRawDataFilePathName();
}

std::string Market::GetProcessedDataFilePath() const
{
	return GetProcessedDataFolderPath() + "/" + GetProcessedDataFileName();
}