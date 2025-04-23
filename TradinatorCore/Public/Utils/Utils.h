#pragma once

#include <string>
#include <chrono>

namespace TradinatorCoreSpace
{
	class Utils
	{
	public:
		static void SetTradinatorWorkingFolderPath(std::string path);
		static std::string GetTradinatorWorkingFolderPath() { return _DATA_FOLDER_PATH_; };

		static std::string GetTradinatorDatabasePath();

		static void SetupFolderStructure();

		static bool DoesFileExist(const std::string& file_path);

	private:
		static std::string _DATA_FOLDER_PATH_;
	};


}
