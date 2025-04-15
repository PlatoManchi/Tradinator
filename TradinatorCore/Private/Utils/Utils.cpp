#include "Utils/Utils.h"

#include <filesystem>

std::string Utils::_DATA_FOLDER_PATH_;

void Utils::SetTradinatorWorkingFolderPath(std::string path)
{
    _DATA_FOLDER_PATH_ = path;
}

std::string Utils::GetTradinatorDatabasePath()
{
    return std::format("{}/Data/Tradinator.db", _DATA_FOLDER_PATH_);
}


void Utils::SetupFolderStructure()
{
    std::filesystem::create_directory(_DATA_FOLDER_PATH_ + "/" + "Data");
}

bool Utils::DoesFileExist(const std::string& file_path)
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