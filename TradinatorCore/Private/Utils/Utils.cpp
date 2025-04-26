#include "Utils/Utils.h"

#include <filesystem>

#include "Indicators/SMA.h"
#include "Indicators/WMA.h"
#include "Indicators/EMA.h"
#include "Indicators/BollingerBand.h"
#include "Indicators/ROC.h"
#include "Indicators/RSI.h"


std::string TradinatorCoreSpace::Utils::_DATA_FOLDER_PATH_;


std::vector<std::unique_ptr<Indicator>> TradinatorCoreSpace::Utils::GetAvailableIndicators()
{
    std::vector<std::unique_ptr<Indicator>> result;

    result.push_back(std::make_unique<SMA>(20));
    result.push_back(std::make_unique<WMA>(20));
    result.push_back(std::make_unique<EMA>(20));
    result.push_back(std::make_unique<BollingerBand>(20, 2));
    result.push_back(std::make_unique<ROC>(12));
    result.push_back(std::make_unique<RSI>(14));

    return result;
}



void TradinatorCoreSpace::Utils::SetTradinatorWorkingFolderPath(std::string path)
{
    _DATA_FOLDER_PATH_ = path;
}

std::string TradinatorCoreSpace::Utils::GetTradinatorDatabasePath()
{
    return std::format("{}/Data/Tradinator.db", _DATA_FOLDER_PATH_);
}


void TradinatorCoreSpace::Utils::SetupFolderStructure()
{
    std::filesystem::create_directory(_DATA_FOLDER_PATH_ + "/" + "Data");
}

bool TradinatorCoreSpace::Utils::DoesFileExist(const std::string& file_path)
{
    // extract folder path from file path
    std::size_t index = file_path.find_last_of('/');
    std::string folder_path = file_path.substr(0, index);

    for (const auto& entry : std::filesystem::directory_iterator(folder_path))
    {
        if (entry.path() == file_path)
        {
            return true;
        }
    }

    return false;
}
