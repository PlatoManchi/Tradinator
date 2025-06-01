#include "Utils/Utils.h"

#include <filesystem>
#include <map>


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
#include "Strategy/Strategy.h"

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


std::unique_ptr<Indicator> TradinatorCoreSpace::Utils::GetIndicator(EIndicatorType type)
{
    switch (type)
    {
    case EIndicatorType::E_SMA:
        return std::make_unique<SMA>();
    case EIndicatorType::E_WMA:
        return std::make_unique<WMA>();
    case EIndicatorType::E_EMA:
        return std::make_unique<EMA>();
    case EIndicatorType::E_BOLLINGER_BAND:
        return std::make_unique<BollingerBand>();
    case EIndicatorType::E_ROC:
        return std::make_unique<ROC>();
    case EIndicatorType::E_RSI:
        return std::make_unique<RSI>();
    case EIndicatorType::E_OBV:
        return std::make_unique<OBV>();
    case EIndicatorType::E_MACD:
        return std::make_unique<MACD>();
    case EIndicatorType::E_ATR:
        return std::make_unique<ATR>();
    case EIndicatorType::E_TrendAnalysisDebug:
        return std::make_unique<TrendAnalysisDebug>();
    }

    return nullptr;
}

std::string TradinatorCoreSpace::Utils::GetIndicatorTypeStr(EIndicatorType type)
{
    static std::unordered_map<EIndicatorType, std::string> _INDICATOR_TO_STRING_ =
    {
        {EIndicatorType::E_SMA, "SMA"},
        {EIndicatorType::E_WMA, "WMA"},
        {EIndicatorType::E_EMA, "EMA"},
        {EIndicatorType::E_BOLLINGER_BAND, "Boolinger Band"},
        {EIndicatorType::E_ROC, "ROC"},
        {EIndicatorType::E_RSI, "RSI"},
        {EIndicatorType::E_OBV, "OBV"},
        {EIndicatorType::E_MACD, "MACD"},
        {EIndicatorType::E_ATR, "ATR"},
        {EIndicatorType::E_TrendAnalysisDebug, "Trend Analysis Debug"}
    };

    auto itr = _INDICATOR_TO_STRING_.find(type);
    if (itr != _INDICATOR_TO_STRING_.end())
    {
        return (*itr).second;
    }

    return "Fill out details at TradinatorCoreSpace::Utils::GetIndicatorTypeStr";
}

EIndicatorType TradinatorCoreSpace::Utils::GetIndicatorType(std::string type_str)
{
    static std::unordered_map<std::string, EIndicatorType> _STRING_TO_INDICATOR_ =
    {
        {"SMA", EIndicatorType::E_SMA},
        {"WMA", EIndicatorType::E_WMA},
        {"EMA", EIndicatorType::E_EMA},
        {"Boolinger Band", EIndicatorType::E_BOLLINGER_BAND},
        {"ROC", EIndicatorType::E_ROC},
        {"RSI", EIndicatorType::E_RSI},
        {"OBV", EIndicatorType::E_OBV},
        {"MACD", EIndicatorType::E_MACD},
        {"ATR", EIndicatorType::E_ATR},
        {"Trend Analysis Debug", EIndicatorType::E_TrendAnalysisDebug}
    };

    auto itr = _STRING_TO_INDICATOR_.find(type_str);
    if (itr != _STRING_TO_INDICATOR_.end())
    {
        return (*itr).second;
    }

    return EIndicatorType::None;
}

std::vector<std::unique_ptr<Pattern>> TradinatorCoreSpace::Utils::GetAvailablePatterns()
{
    std::vector<std::unique_ptr<Pattern>> result;

    // Order is important, priority patterns should be first
    result.emplace_back(std::make_unique<BullishLongLeggedDojiPattern>());
    result.emplace_back(std::make_unique<BullishTriStarPattern>());
    result.emplace_back(std::make_unique<BullishAbandonedBabyPattern>());
    result.emplace_back(std::make_unique<BullishMorningStarDojiPattern>());
    result.emplace_back(std::make_unique<BullishGraveStoneDojiPattern>());
    result.emplace_back(std::make_unique<BullishHaramiCrossPattern>());

    result.emplace_back(std::make_unique<BullishThreeOutsideUpPattern>());
    result.emplace_back(std::make_unique<BullishThreeInsideUpPattern>());
    result.emplace_back(std::make_unique<BullishMatchingLowPattern>());
    result.emplace_back(std::make_unique<BullishKickingPattern>());
    result.emplace_back(std::make_unique<BullishThreeWhiteSoldiersPattern>());
    result.emplace_back(std::make_unique<BullishMeetingLinesPattern>());
    result.emplace_back(std::make_unique<BullishMorningStarPattern>());
    result.emplace_back(std::make_unique<BullishInvertedHammerPattern>());
    result.emplace_back(std::make_unique<BullishHaramiPattern>());
    result.emplace_back(std::make_unique<BullishPiercingPattern>());
    result.emplace_back(std::make_unique<BullishEngulfingPattern>());
    result.emplace_back(std::make_unique<BullishHammerPattern>());


    result.emplace_back(std::make_unique<BearishLongLeggedDojiPattern>());
    result.emplace_back(std::make_unique<BearishTriStarPattern>());
    result.emplace_back(std::make_unique<BearishAbandonedBabyPattern>());
    result.emplace_back(std::make_unique<BearishEveningStarDojiPattern>());
    result.emplace_back(std::make_unique<BearishGraveStoneDojiPattern>());
    result.emplace_back(std::make_unique<BearishHaramiCrossPattern>());

    result.emplace_back(std::make_unique<BearishThreeOutsideDownPattern>());
    result.emplace_back(std::make_unique<BearishThreeInsideDownPattern>());
    result.emplace_back(std::make_unique<BearishMatchingHighPattern>());
    result.emplace_back(std::make_unique<BearishKickingPattern>());
    result.emplace_back(std::make_unique<BearishThreeBlackCrowPattern>());
    result.emplace_back(std::make_unique<BearishMeetingLinePattern>());
    result.emplace_back(std::make_unique<BearishEveningStarPattern>());
    result.emplace_back(std::make_unique<BearishShootingStarPattern>());
    result.emplace_back(std::make_unique<BearishHaramiPattern>());
    result.emplace_back(std::make_unique<BearishPiercingPattern>());
    result.emplace_back(std::make_unique<BearishEngulfingPattern>());
    result.emplace_back(std::make_unique<BearishHangingManPattern>());
    


    result.emplace_back(std::make_unique<DragonFlyDojiPattern>());
    result.emplace_back(std::make_unique<GraveStonrDojiPattern>());
    result.emplace_back(std::make_unique<LongLegDojiPattern>());

    return result;
}






std::vector<EPattern> TradinatorCoreSpace::Utils::GetAllPatternsFrom(EPattern patterns)
{
    std::vector<EPattern> result;

    for (int i = 0; i < 64; ++i)
    {
        uint64_t test = 1LL << i;
        if ((uint64_t)patterns & test)
        {
            EPattern matched_pattern = static_cast<EPattern>(test);
            if (matched_pattern != EPattern::None && matched_pattern != EPattern::Max)
            {
                result.push_back(static_cast<EPattern>(test));
            }
        }
    }

    return result;
}

EPattern TradinatorCoreSpace::Utils::GetPatternFrom(EPattern patterns)
{
    // The order in which enum is arranged is thebt priority of patterns in order of their importance
    for (int i = 0; i < 64; ++i)
    {
        int64_t test = 1LL << i;
        if ((int64_t)patterns & test)
        {
            EPattern matched_pattern = static_cast<EPattern>(test);
            if (matched_pattern != EPattern::None && matched_pattern != EPattern::Max)
            {
                return matched_pattern;
            }
        }
    }

    return EPattern::None;
}

std::string TradinatorCoreSpace::Utils::GetPatternShortDescription(EPattern pattern)
{
    static std::unordered_map<EPattern, std::string> _PATTERN_TO_STRING_ =
    {
            { EPattern::None, "None" },

            // Bullish patterns with Doji
            { EPattern::Bullish_Long_Legged_Doji, "Bullish Log Legged Doji" },
            { EPattern::Bullish_Tri_Star, "Bullish Tri Star" },
            { EPattern::Bullish_Abandoned_Baby, "Bullish Abandoned Baby" },
            { EPattern::Bullish_Morning_Star_Doji, "Bullish Morning Star Doji" },
            { EPattern::Bullish_Grave_Stone_Doji, "Bullish Grave Stone Doji" },
            { EPattern::Bullish_Harami_Cross, "Bullish Harami Cross" },

            // Bullish patterns without Doji
            { EPattern::Bullish_Three_Outside_Up, "Bullish Three Outside Up" },
            { EPattern::Bullish_Three_Inside_Up, "Bullish Three Inside Up" },
            { EPattern::Bullish_Matching_Low, "Bullish Matching Low" },
            { EPattern::Bullish_Kicking, "Bullish Kicking" },
            { EPattern::Bullish_Three_White_Soldiers, "Bullish Three White Soldiers" },
            { EPattern::Bullish_Meeting_Lines, "Bullish Meeting Lines" },
            { EPattern::Bullish_Morning_Star, "Bullish Morning Star" },
            { EPattern::Bullish_Inverted_Hammer, "Bullish Inverted Hammer" },
            { EPattern::Bullish_Harami, "Bullish Harami" },
            { EPattern::Bullish_Piercing, "Bullish Piercing" },
            { EPattern::Bullish_Engulfing, "Bullish Engulfing" },
            { EPattern::Bullish_Hammer, "Bullish Hammer" },

            // Bearish patterns with Doji
            { EPattern::Bearish_Long_Legged_Doji, "Bearish Long Legged Doji" },
            { EPattern::Bearish_Tri_Star, "Bearish Tri Star" },
            { EPattern::Bearish_Abandoned_Baby, "Bearish Abandoned Baby" },
            { EPattern::Bearish_Evening_Star_Doji, "Bearish Evening Star Doji" },
            { EPattern::Bearish_Grave_Stone_Doji, "Bearish Grave Stone Doji" },
            { EPattern::Bearish_Harami_Cross, "Bearish Harami Cross" },

            // Bullish pattern without Doji
            { EPattern::Bearish_Three_Outside_Down, "Bearish Three Outside Down" },
            { EPattern::Bearish_Three_Inside_Down, "Bearish Three Inside Down" },
            { EPattern::Bearish_Matching_High, "Bearish Matching High" },
            { EPattern::Bearish_Kicking, "Bearish Kicking" },
            { EPattern::Bearish_Three_Black_Crow, "Bearish Three Black Crow" },
            { EPattern::Bearish_Meeting_Line, "Bearish Meeting Line" },
            { EPattern::Bearish_Evening_Star, "Bearish Evening Star" },
            { EPattern::Bearish_Shooting_Star, "Bearish Shooting Star" },
            { EPattern::Bearish_Harami, "Bearish Harami" },
            { EPattern::Bearish_Piercing, "Bearish Piercing" },
            { EPattern::Bearish_Engulfing, "Bearish Engulfing" },
            { EPattern::Bearish_Hanging_Man, "Bearish Hanging Man" },


            // Generic Patterns
            { EPattern::Dragon_Fly_Doji, "Dragon Fly Doji" },
            { EPattern::Grave_Stone_Doji, "Grave Stone Doji" },
            { EPattern::Long_Leg_Doji, "Long Leg Doji" },
    };

    auto itr = _PATTERN_TO_STRING_.find(pattern);
    if (itr != _PATTERN_TO_STRING_.end())
    {
        return (*itr).second;
    }

    return "Fill out details at TradinatorCoreSpace::Utils::GetPatternShortDescription";
}

EPattern TradinatorCoreSpace::Utils::GetPatternFromShortDescription(const std::string& short_description)
{
    static std::unordered_map<std::string, EPattern> _STRING_TO_PATTERN_ =
    {
            { "None", EPattern::None},

            // Bullish patterns with Doji
            { "Bullish Log Legged Doji", EPattern::Bullish_Long_Legged_Doji },
            { "Bullish Tri Star", EPattern::Bullish_Tri_Star },
            { "Bullish Abandoned Baby", EPattern::Bullish_Abandoned_Baby},
            { "Bullish Morning Star Doji", EPattern::Bullish_Morning_Star_Doji },
            { "Bullish Grave Stone Doji", EPattern::Bullish_Grave_Stone_Doji },
            { "Bullish Harami Cross", EPattern::Bullish_Harami_Cross },

            // Bullish patterns without Doji
            { "Bullish Three Outside Up", EPattern::Bullish_Three_Outside_Up },
            { "Bullish Three Inside Up", EPattern::Bullish_Three_Inside_Up },
            { "Bullish Matching Low", EPattern::Bullish_Matching_Low },
            { "Bullish Kicking", EPattern::Bullish_Kicking },
            { "Bullish Three White Soldiers", EPattern::Bullish_Three_White_Soldiers },
            { "Bullish Meeting Lines", EPattern::Bullish_Meeting_Lines },
            { "Bullish Morning Star", EPattern::Bullish_Morning_Star },
            { "Bullish Inverted Hammer", EPattern::Bullish_Inverted_Hammer },
            { "Bullish Harami", EPattern::Bullish_Harami },
            { "Bullish Piercing", EPattern::Bullish_Piercing },
            { "Bullish Engulfing", EPattern::Bullish_Engulfing},
            { "Bullish Hammer", EPattern::Bullish_Hammer},


            // Bearish patterns with Doji
            { "Bearish Long Legged Doji", EPattern::Bearish_Long_Legged_Doji },
            { "Bearish Tri Star", EPattern::Bearish_Tri_Star },
            { "Bearish Abandoned Baby", EPattern::Bearish_Abandoned_Baby },
            { "Bearish Evening Star Doji", EPattern::Bearish_Evening_Star_Doji },
            { "Bearish Grave Stone Doji", EPattern::Bearish_Grave_Stone_Doji },
            { "Bearish Harami Cross", EPattern::Bearish_Harami_Cross },

            // Bullish pattern without Doji
            { "Bearish Three Outside Down", EPattern::Bearish_Three_Outside_Down },
            { "Bearish Three Inside Down", EPattern::Bearish_Three_Inside_Down },
            { "Bearish Matching High", EPattern::Bearish_Matching_High },
            { "Bearish Kicking", EPattern::Bearish_Kicking },
            { "Bearish Three Black Crow", EPattern::Bearish_Three_Black_Crow },
            { "Bearish Meeting Line", EPattern::Bearish_Meeting_Line },
            { "Bearish Evening Star", EPattern::Bearish_Evening_Star },
            { "Bearish Shooting Star", EPattern::Bearish_Shooting_Star },
            { "Bearish Harami", EPattern::Bearish_Harami },
            { "Bearish Piercing", EPattern::Bearish_Piercing },
            { "Bearish Engulfing", EPattern::Bearish_Engulfing },
            { "Bearish Hanging Man", EPattern::Bearish_Hanging_Man },


            // Generic Patterns
            { "Dragon Fly Doji", EPattern::Dragon_Fly_Doji },
            { "Grave Stone Doji", EPattern::Grave_Stone_Doji },
            { "Long Leg Doji", EPattern::Long_Leg_Doji },
    };


    auto itr = _STRING_TO_PATTERN_.find(short_description);
    if (itr != _STRING_TO_PATTERN_.end())
    {
        return (*itr).second;
    }

    return EPattern::None;
}


std::vector<std::unique_ptr<Strategy>> TradinatorCoreSpace::Utils::GetAvailableStrategies()
{
    std::vector<std::unique_ptr<Strategy>> result;

    result.emplace_back(std::make_unique<Long_Strategy_1>());
    result.emplace_back(std::make_unique<Short_Strategy_1>());

    return result;
}

std::vector<EStrategy> TradinatorCoreSpace::Utils::GetAllStrategiesFrom(EStrategy strategies)
{
    std::vector<EStrategy> result;

    for (int i = 0; i < 64; ++i)
    {
        int64_t test = 1LL << i;
        if ((int64_t)strategies & test)
        {
            EStrategy strategy = static_cast<EStrategy>(test);
            if (strategy != EStrategy::None && strategy != EStrategy::Max)
            {
                result.push_back(static_cast<EStrategy>(test));
            }
        }
    }

    return result;
}

std::unique_ptr<Strategy> TradinatorCoreSpace::Utils::GetStrategy(EStrategy strategy)
{
    switch (strategy)
    {
    case EStrategy::Long_Strategy_1:
        return std::make_unique<Long_Strategy_1>();
    case EStrategy::Short_Strategy_1:
        return std::make_unique<Short_Strategy_1>();
    }

    return nullptr;
}

std::string TradinatorCoreSpace::Utils::GetStrategyTypeStr(EStrategy type)
{
    static std::unordered_map<EStrategy, std::string> _STRATEGY_TO_STRING_ =
    {
        { EStrategy::Long_Strategy_1, "Long Strategy 1" },
        { EStrategy::Short_Strategy_1, "Short Strategy 1" }
    };

    auto itr = _STRATEGY_TO_STRING_.find(type);
    if (itr != _STRATEGY_TO_STRING_.end())
    {
        return (*itr).second;
    }

    return "Fill out details at TradinatorCoreSpace::Utils::GetStrategyTypeStr";
}

EStrategy TradinatorCoreSpace::Utils::GetStrategyType(std::string type_str)
{
    static std::unordered_map<std::string, EStrategy> _STRING_TO_STRATEGY_ =
    {
        { "Long Strategy 1", EStrategy::Long_Strategy_1 },
        { "Short Strategy 1", EStrategy::Short_Strategy_1 }
    };

    auto itr = _STRING_TO_STRATEGY_.find(type_str);
    if (itr != _STRING_TO_STRATEGY_.end())
    {
        return (*itr).second;
    }

    return EStrategy::None;
}

std::string TradinatorCoreSpace::Utils::GetStrategyDesc(EStrategy type)
{
    static std::unordered_map<EStrategy, std::string> _STRATEGY_TO_STRING_ =
    {
        {EStrategy::Long_Strategy_1,
            "Long Strategy 1\n\n" \
            "Rule 1: Short Term Moving Average crossing Long Term Moving Average from below to above.\n" \
            "Rule 2: RSI crossing 50 level from below to above.\n" \
            "Rule 3: MACD crossing signal line or zero line from below to above. MACD moving above zero is better than moving above signal line.\n\n" \
            "NOTE: Consolidation break out from resistance must happen for strategy to work."},
        {EStrategy::Short_Strategy_1,
            "Short Strategy 1\n\n"
            "Rule 1: Short Term Moving Average crossing Long Term Moving Average from above to below.\n" \
            "Rule 2: RSI crossing 50 level from above to below.\n" \
            "Rule 3: MACD crossing signal line or zero line from above to below. MACD moving below zero is better than moving below signal line.\n\n" \
            "NOTE: Consolidation break down from support must happen for strategy to work."},
    };

    auto itr = _STRATEGY_TO_STRING_.find(type);
    if (itr != _STRATEGY_TO_STRING_.end())
    {
        return (*itr).second;
    }

    return "Fill out details at TradinatorCoreSpace::Utils::GetStrategyDesc";
}