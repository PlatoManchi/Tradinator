#pragma once

#include <cstdint>
#include <type_traits>

enum class EIndicatorSource
{
	E_HIGH,
	E_OPEN,
	E_LOW,
	E_CLOSE
};

enum class EIndicatorType
{
	MIN,

	E_SMA,
	E_WMA,
	E_EMA,
	E_BOLLINGER_BAND,
	E_ROC,
	E_RSI,
	E_OBV,
	E_MACD,
	E_ATR,
	E_TrendAnalysisDebug,


	MAX
};



// The order in which enum is arranged is also priority of patterns based on their importance
// NOTE: Sqlite can only store int64_t and not uint64_t. Hence using int64_t so that it can be converted from int64_t to EPattern
enum class EPattern : int64_t {
    None = 0,

    // Patterns with Doji
    Bullish_Long_Legged_Doji = 1ULL << 1,
    Bearish_Long_Legged_Doji = 1ULL << 2,
    Bullish_Tri_Star = 1ULL << 3,
    Bearish_Tri_Star = 1ULL << 4,
    Bullish_Abandoned_Baby = 1ULL << 5,
    Bearish_Abandoned_Baby = 1ULL << 6,
    Bullish_Morning_Star_Doji = 1ULL << 7,
    Bearish_Evening_Star_Doji = 1ULL << 8,
    Bullish_Grave_Stone_Doji = 1ULL << 9,
    Bearish_Grave_Stone_Doji = 1ULL << 10,
    Bullish_Harami_Cross = 1ULL << 11,
    Bearish_Harami_Cross = 1ULL << 12,

    // Patterns without Doji
    Bullish_Three_Outside_Up = 1ULL << 13,
    Bearish_Three_Outside_Down = 1ULL << 14,
    Bullish_Three_Inside_Up = 1ULL << 15,
    Bearish_Three_Inside_Down = 1ULL << 16,
    Bullish_Matching_Low = 1ULL << 17,
    Bearish_Matching_High = 1ULL << 18,
    Bullish_Kicking = 1ULL << 19,
    Bearish_Kicking = 1ULL << 20,
    Bullish_Three_White_Soldiers = 1ULL << 21,
    Bearish_Three_Black_Crow = 1ULL << 22,
    Bullish_Meeting_Lines = 1ULL << 23,
    Bearish_Meeting_Line = 1ULL << 24,
    Bullish_Morning_Star = 1ULL << 25,
    Bearish_Evening_Star = 1ULL << 26,
    Bullish_Inverted_Hammer = 1ULL << 27,
    Bearish_Shooting_Star = 1ULL << 28,
    Bullish_Harami = 1ULL << 29,
    Bearish_Harami = 1ULL << 30,
    Bullish_Piercing = 1ULL << 31,
    Bearish_Piercing = 1ULL << 32,
    Bullish_Engulfing = 1ULL << 33,
    Bearish_Engulfing = 1ULL << 34,
    Bullish_Hammer = 1ULL << 35,
    Bearish_Hanging_Man = 1ULL << 36,



    // Generic Patterns
    Dragon_Fly_Doji = 1ULL << 37,
    Grave_Stone_Doji = 1ULL << 38,
    Long_Leg_Doji = 1ULL << 39,

    Max = 1ULL << 40,
};

// Enable bitwise ops on enum class
inline EPattern operator|(EPattern a, EPattern b) {
    return static_cast<EPattern>(
        static_cast<std::underlying_type_t<EPattern>>(a) |
        static_cast<std::underlying_type_t<EPattern>>(b)
        );
}

inline EPattern operator&(EPattern a, EPattern b) {
    return static_cast<EPattern>(
        static_cast<std::underlying_type_t<EPattern>>(a) &
        static_cast<std::underlying_type_t<EPattern>>(b)
        );
}




const EPattern Bullish_Pattern_Type =
        EPattern::Bullish_Long_Legged_Doji |
        EPattern::Bullish_Tri_Star |
        EPattern::Bullish_Abandoned_Baby |
        EPattern::Bullish_Morning_Star_Doji |
        EPattern::Bullish_Grave_Stone_Doji |
        EPattern::Bullish_Harami_Cross |
        EPattern::Bullish_Three_Outside_Up |
        EPattern::Bullish_Three_Inside_Up |
        EPattern::Bullish_Matching_Low |
        EPattern::Bullish_Kicking |
        EPattern::Bullish_Three_White_Soldiers |
        EPattern::Bullish_Meeting_Lines |
        EPattern::Bullish_Morning_Star |
        EPattern::Bullish_Inverted_Hammer |
        EPattern::Bullish_Harami |
        EPattern::Bullish_Piercing |
        EPattern::Bullish_Engulfing |
        EPattern::Bullish_Hammer;



const EPattern Bearish_Pattern_Type =
        EPattern::Bearish_Long_Legged_Doji |
        EPattern::Bearish_Tri_Star |
        EPattern::Bearish_Abandoned_Baby |
        EPattern::Bearish_Evening_Star_Doji |
        EPattern::Bearish_Grave_Stone_Doji |
        EPattern::Bearish_Harami_Cross |
        EPattern::Bearish_Three_Outside_Down |
        EPattern::Bearish_Three_Inside_Down |
        EPattern::Bearish_Matching_High |
        EPattern::Bearish_Kicking |
        EPattern::Bearish_Three_Black_Crow |
        EPattern::Bearish_Meeting_Line |
        EPattern::Bearish_Evening_Star |
        EPattern::Bearish_Shooting_Star |
        EPattern::Bearish_Harami |
        EPattern::Bearish_Piercing |
        EPattern::Bearish_Engulfing |
        EPattern::Bearish_Hanging_Man;
