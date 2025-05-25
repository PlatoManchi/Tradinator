#pragma once

#include <string>
#include <chrono>
#include <unordered_map>

//#include "Patterns/Pattern.h"

class Indicator;
class Pattern;
enum class EPattern : int64_t;

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

		static std::vector<std::unique_ptr<Indicator>> GetAvailableIndicators();
		static std::vector<std::unique_ptr<Pattern>> GetAvailablePatterns();

		static size_t GetMaxParallelDownloads() { return _MAX_PARALLEL_DOWNLOADS_; }
		static size_t GetMaxParallelAnalysis() { return _MAX_PARALLEL_ANALYSIS_; }
		static size_t GetReadWriteBatchSize() { return _READ_WRITE_BATCH_SIZE_; }

		static void SetMaxParallelDownloads(size_t max_parallel_downloads);
		static void SetMaxParallelAnalysis(size_t max_parallel_analysis);
		static void SetReadWriteBatchSize(size_t read_write_batch_size);


		static std::vector<EPattern> GetAllPatternsFrom(EPattern patterns);
		static EPattern GetPatternFrom(EPattern patterns);
		static std::string GetPatternShortDescription(EPattern pattern);
		static EPattern GetPatternFromShortDescription(const std::string& short_description);

	private:
		static std::string _DATA_FOLDER_PATH_;
		static size_t _MAX_PARALLEL_DOWNLOADS_;
		static size_t _MAX_PARALLEL_ANALYSIS_;
		static size_t _READ_WRITE_BATCH_SIZE_;

	};

}
