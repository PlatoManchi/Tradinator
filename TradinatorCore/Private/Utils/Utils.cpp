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
    result.emplace_back(std::make_unique<TrendAnalysisDebug>());

    // arrange them alphabet wise
    std::sort(result.begin(), result.end(),
        [](const std::unique_ptr<Indicator>& a, const std::unique_ptr<Indicator>& b) -> bool
        {
            return a->GetName() < b->GetName();
        });

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




std::vector<std::unique_ptr<Pattern>> TradinatorCoreSpace::Utils::GetAvailablePatterns()
{
    std::vector<std::unique_ptr<Pattern>> result;

    // Order is important, priority patterns should be first
    result.emplace_back(std::make_unique<BullishHaramiCrossPattern>());
    result.emplace_back(std::make_unique<BullishHaramiPattern>());
    result.emplace_back(std::make_unique<BullishPiercingPattern>());
    result.emplace_back(std::make_unique<BullishEngulfingPattern>());


    result.emplace_back(std::make_unique<DragonFlyDojiPattern>());
    result.emplace_back(std::make_unique<GraveStonrDojiPattern>());
    result.emplace_back(std::make_unique<LongLegDojiPattern>());

    return result;
}

/*std::string TradinatorCoreSpace::Utils::GetPatternDescription(EPatternType pattern)
{
    if (pattern & Bullish_Harami)
    {
        return "Bullish Harami";
    }
    else if ((pattern & Bullish_Harami_Cross).any())
    {
        return "Bullish Harami Cross";
    }

    return "Fill out details at TradinatorCoreSpace::Utils::GetPatternDescription";
}*/

std::string TradinatorCoreSpace::Utils::GetPatternShortDescription(EPatternType pattern)
{
    static std::unordered_map<EPatternType, std::string> s_pattern_to_string
    {
        // Bullish patterns with Doji
        { EPatternType::Bullish_Long_Legged_Doji, "Bullish Log Legged Doji" },
        { EPatternType::Bullish_Tri_Star, "Bullish Tri Star" },
        { EPatternType::Bullish_Abandoned_Baby, "Bullish Abandoned Baby" },
        { EPatternType::Bullish_Morning_Star_Doji, "Bullish Morning Star Doji" },
        { EPatternType::Bullish_Grave_Stone_Doji, "Bullish Grave Stone Doji" },
        { EPatternType::Bullish_Harami_Cross, "Bullish Harami Cross" },

        // Bullish patterns without Doji
        { EPatternType::Bullish_Three_Outside_Up, "Bullish Three Outside Up" },
        { EPatternType::Bullish_Three_Inside_Up, "Bullish Three Inside Up" },
        { EPatternType::Bullish_Matching_Low, "Bullish Matching Low" },
        { EPatternType::Bullish_Kicking, "Bullish Kicking" },
        { EPatternType::Bullish_Three_White_Soldiers, "Bullish Three White Soldiers" },
        { EPatternType::Bullish_Meeting_Lines, "Bullish Meeting Lines" },
        { EPatternType::Bullish_Morning_Star, "Bullish Morning Star" },
        { EPatternType::Bullish_Inverted_Hammer, "Bullish Inverted Hammer" },
        { EPatternType::Bullish_Harami, "Bullish Harami" },
        { EPatternType::Bullish_Piercing, "Bullish Piercing" },
        { EPatternType::Bullish_Engulfing, "Bullish Engulfing" },
        { EPatternType::Bullish_Hammer, "Bullish Hammer" },

        // Bearish patterns with Doji
        { EPatternType::Bearish_Long_Legged_Doji, "Bearish Long Legged Doji" },
        { EPatternType::Bearish_Tri_Star, "Bearish Tri Star" },
        { EPatternType::Bearish_Abandoned_Baby, "Bearish Abandoned Baby" },
        { EPatternType::Bearish_Evening_Star_Doji, "Bearish Evening Star Doji" },
        { EPatternType::Bearish_Bear_Stone_Doji, "Bearish Bear Stone Doji" },
        { EPatternType::Bearish_Harami_Cross, "Bearish Harami Cross" },

        // Bullish pattern without Doji
        { EPatternType::Bearish_Three_Outside_Down, "Bearish Three Outside Down" },
        { EPatternType::Bearish_Three_Inside_Down, "Bearish Three Inside Down" },
        { EPatternType::Bearish_Matching_High, "Bearish Matching High" },
        { EPatternType::Bearish_Kicking, "Bearish Kicking" },
        { EPatternType::Bearish_Three_Black_Crow, "Bearish Three Black Crow" },
        { EPatternType::Bearish_Meeting_Line, "Bearish Meeting Line" },
        { EPatternType::Bearish_Evening_Star, "Bearish Evening Star" },
        { EPatternType::Bullish_Shooting_Star, "Bullish Shooting Star" },
        { EPatternType::Bearish_Harami, "Bearish Harami" },
        { EPatternType::Bearish_Piercing, "Bearish Piercing" },
        { EPatternType::Bearish_Engulfing, "Bearish Engulfing" },
        { EPatternType::Bearish_Hanging_Man, "Bearish Hanging Man" },


        // Generic Patterns
        { EPatternType::Dragon_Fly_Doji, "Dragon Fly Doji" },
        { EPatternType::Grave_Stone_Doji, "Grave Stone Doji" },
        { EPatternType::Long_Leg_Doji, "Long Leg Doji" },
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
    EPatternType result = EPatternType::None;

    static std::unordered_map<std::string, EPatternType> s_string_to_pattern
    {
        // Bullish patterns with Doji
        { "Bullish Log Legged Doji", EPatternType::Bullish_Long_Legged_Doji },
        { "Bullish Tri Star", EPatternType::Bullish_Tri_Star },
        { "Bullish Abandoned Baby", EPatternType::Bullish_Abandoned_Baby},
        { "Bullish Morning Star Doji", EPatternType::Bullish_Morning_Star_Doji },
        { "Bullish Grave Stone Doji", EPatternType::Bullish_Grave_Stone_Doji },
        { "Bullish Harami Cross", EPatternType::Bullish_Harami_Cross },

        // Bullish patterns without Doji
        { "Bullish Three Outside Up", EPatternType::Bullish_Three_Outside_Up },
        { "Bullish Three Inside Up", EPatternType::Bullish_Three_Inside_Up },
        { "Bullish Matching Low", EPatternType::Bullish_Matching_Low },
        { "Bullish Kicking", EPatternType::Bullish_Kicking },
        { "Bullish Three White Soldiers", EPatternType::Bullish_Three_White_Soldiers },
        { "Bullish Meeting Lines", EPatternType::Bullish_Meeting_Lines },
        { "Bullish Morning Star", EPatternType::Bullish_Morning_Star },
        { "Bullish Inverted Hammer", EPatternType::Bullish_Inverted_Hammer },
        { "Bullish Harami", EPatternType::Bullish_Harami },
        { "Bullish Piercing", EPatternType::Bullish_Piercing },
        { "Bullish Engulfing", EPatternType::Bullish_Engulfing},
        { "Bullish Hammer", EPatternType::Bullish_Hammer},


        // Bearish patterns with Doji
        { "Bearish Long Legged Doji", EPatternType::Bearish_Long_Legged_Doji },
        { "Bearish Tri Star", EPatternType::Bearish_Tri_Star },
        { "Bearish Abandoned Baby", EPatternType::Bearish_Abandoned_Baby },
        { "Bearish Evening Star Doji", EPatternType::Bearish_Evening_Star_Doji },
        { "Bearish Bear Stone Doji", EPatternType::Bearish_Bear_Stone_Doji },
        { "Bearish Harami Cross", EPatternType::Bearish_Harami_Cross },

        // Bullish pattern without Doji
        { "Bearish Three Outside Down", EPatternType::Bearish_Three_Outside_Down },
        { "Bearish Three Inside Down", EPatternType::Bearish_Three_Inside_Down },
        { "Bearish Matching High", EPatternType::Bearish_Matching_High },
        { "Bearish Kicking", EPatternType::Bearish_Kicking },
        { "Bearish Three Black Crow", EPatternType::Bearish_Three_Black_Crow },
        { "Bearish Meeting Line", EPatternType::Bearish_Meeting_Line },
        { "Bearish Evening Star", EPatternType::Bearish_Evening_Star },
        { "Bullish Shooting Star", EPatternType::Bullish_Shooting_Star },
        { "Bearish Harami", EPatternType::Bearish_Harami },
        { "Bearish Piercing", EPatternType::Bearish_Piercing },
        { "Bearish Engulfing", EPatternType::Bearish_Engulfing },
        { "Bearish Hanging Man", EPatternType::Bearish_Hanging_Man },


        // Generic Patterns
        { "Dragon Fly Doji", EPatternType::Dragon_Fly_Doji },
        { "Grave Stone Doji", EPatternType::Grave_Stone_Doji },
        { "Long Leg Doji", EPatternType::Long_Leg_Doji },
    };

    auto itr = s_string_to_pattern.find(short_description);
    if (itr != s_string_to_pattern.end())
    {
        return (*itr).second;
    }

    return result;
}