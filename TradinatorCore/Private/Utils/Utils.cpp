#include "Utils/Utils.h"

#include <filesystem>


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