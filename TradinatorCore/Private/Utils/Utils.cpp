#include "Utils/Utils.h"

#include <filesystem>
#include <map>
#include <unordered_map>

#include "Indicators/SMA.h"
#include "Indicators/WMA.h"
#include "Indicators/EMA.h"
#include "Indicators/BollingerBand.h"
#include "Indicators/ROC.h"
#include "Indicators/RSI.h"
#include "Indicators/OBV.h"
#include "Indicators/MACD.h"
#include "Indicators/ATR.h"
#include "Indicators/TrendAnalysisDebug.h"

#include "Patterns/Pattern.h"

std::string TradinatorCoreSpace::Utils::_DATA_FOLDER_PATH_;
size_t TradinatorCoreSpace::Utils::_MAX_PARALLEL_DOWNLOADS_ = 100;
size_t TradinatorCoreSpace::Utils::_MAX_PARALLEL_ANALYSIS_ = 100;
size_t TradinatorCoreSpace::Utils::_READ_WRITE_BATCH_SIZE_ = 30;

std::vector<std::unique_ptr<Indicator>> TradinatorCoreSpace::Utils::GetAvailableIndicators()
{
    std::vector<std::unique_ptr<Indicator>> result;

    result.emplace_back(std::make_unique<SMA>(20));
    result.emplace_back(std::make_unique<WMA>(20));
    result.emplace_back(std::make_unique<EMA>(20));
    result.emplace_back(std::make_unique<BollingerBand>(20, 2));
    result.emplace_back(std::make_unique<ROC>(12));
    result.emplace_back(std::make_unique<RSI>(14));
    result.emplace_back(std::make_unique<OBV>());
    result.emplace_back(std::make_unique<MACD>(12, 26, 9));
    result.emplace_back(std::make_unique<ATR>(14));
    result.emplace_back(std::make_unique<TrendAnalysisDebug>(14));

    // arrange them alphabet wise
    std::sort(result.begin(), result.end(),
        [](const std::unique_ptr<Indicator>& a, const std::unique_ptr<Indicator>& b) -> bool
        {
            return a->GetName() < b->GetName();
        });

    return result;
}


std::vector<std::unique_ptr<Pattern>> TradinatorCoreSpace::Utils::GetAvailablePatterns()
{
    std::vector<std::unique_ptr<Pattern>> result;

    // Order is important, priority patterns should be first
    result.emplace_back(std::make_unique<BullishHaramiCrossPattern>());
    result.emplace_back(std::make_unique<BullishHaramiPattern>());
    

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

    std::filesystem::path path_to_check(folder_path);
    if (std::filesystem::is_directory(path_to_check))
    {
        for (const auto& entry : std::filesystem::directory_iterator(folder_path))
        {
            if (entry.path() == file_path)
            {
                return true;
            }
        }
    }

    return false;
}


void TradinatorCoreSpace::Utils::SetMaxParallelDownloads(size_t max_parallel_downloads)
{
    if (max_parallel_downloads > 0)
    {
        _MAX_PARALLEL_DOWNLOADS_ = max_parallel_downloads;
    }
    else
    {
        _MAX_PARALLEL_DOWNLOADS_ = 100;
    }
}

void TradinatorCoreSpace::Utils::SetMaxParallelAnalysis(size_t max_parallel_analysis)
{
    if (max_parallel_analysis > 0)
    {
        _MAX_PARALLEL_ANALYSIS_ = max_parallel_analysis;
    }
    else
    {
        _MAX_PARALLEL_ANALYSIS_ = 100;
    }
}

void TradinatorCoreSpace::Utils::SetReadWriteBatchSize(size_t read_write_batch_size)
{
    if (read_write_batch_size > 0)
    {
        _READ_WRITE_BATCH_SIZE_ = read_write_batch_size;
    }
    else
    {
        _READ_WRITE_BATCH_SIZE_ = 10;
    }
}





std::string TradinatorCoreSpace::Utils::GetPatternDescription(EPatternType pattern)
{
    if ((pattern & Bullish_Harami).any())
    {
        return "Bullish Harami";
    }
    else if ((pattern & Bullish_Harami_Cross).any())
    {
        return "Bullish Harami Cross";
    }

    return "Fill out details at TradinatorCoreSpace::Utils::GetPatternDescription";
}

std::string TradinatorCoreSpace::Utils::GetPatternShortDescription(EPatternType pattern)
{
    static std::unordered_map<EPatternType, std::string> s_pattern_to_string
    {
        { Bullish_Harami, "BullishHarami" },
        { Bullish_Harami_Cross, "BullishHaramiCross" }
    };

    auto itr = s_pattern_to_string.find(pattern);
    if (itr != s_pattern_to_string.end())
    {
        return (*itr).second;
    }

    return "Fill out details at TradinatorCoreSpace::Utils::GetPatternShortDescription";
}

EPatternType TradinatorCoreSpace::Utils::GetPatternFromShortDescription(const std::string& short_description)
{
    EPatternType result(0);

    static std::unordered_map<std::string, EPatternType> s_string_to_pattern
    {
        { "BullishHarami", Bullish_Harami },
        { "BullishHaramiCross", Bullish_Harami_Cross }
    };

    auto itr = s_string_to_pattern.find(short_description);
    if (itr != s_string_to_pattern.end())
    {
        return (*itr).second;
    }

    return result;
}