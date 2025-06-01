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
	None,

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


	Max
};



// The order in which enum is arranged is also priority of patterns based on their importance
// NOTE: Sqlite can only store int64_t and not uint64_t. Hence using int64_t so that it can be converted from int64_t to EPattern
enum class EPattern : int64_t 
{
    None = 0,

    // Patterns with Doji
    Bullish_Long_Legged_Doji = 1LL << 1,
    Bearish_Long_Legged_Doji = 1LL << 2,
    Bullish_Tri_Star = 1LL << 3,
    Bearish_Tri_Star = 1LL << 4,
    Bullish_Abandoned_Baby = 1LL << 5,
    Bearish_Abandoned_Baby = 1LL << 6,
    Bullish_Morning_Star_Doji = 1LL << 7,
    Bearish_Evening_Star_Doji = 1LL << 8,
    Bullish_Grave_Stone_Doji = 1LL << 9,
    Bearish_Grave_Stone_Doji = 1LL << 10,
    Bullish_Harami_Cross = 1LL << 11,
    Bearish_Harami_Cross = 1LL << 12,

    // Patterns without Doji
    Bullish_Three_Outside_Up = 1LL << 13,
    Bearish_Three_Outside_Down = 1LL << 14,
    Bullish_Three_Inside_Up = 1LL << 15,
    Bearish_Three_Inside_Down = 1LL << 16,
    Bullish_Matching_Low = 1LL << 17,
    Bearish_Matching_High = 1LL << 18,
    Bullish_Kicking = 1LL << 19,
    Bearish_Kicking = 1LL << 20,
    Bullish_Three_White_Soldiers = 1LL << 21,
    Bearish_Three_Black_Crow = 1LL << 22,
    Bullish_Meeting_Lines = 1LL << 23,
    Bearish_Meeting_Line = 1LL << 24,
    Bullish_Morning_Star = 1LL << 25,
    Bearish_Evening_Star = 1LL << 26,
    Bullish_Inverted_Hammer = 1LL << 27,
    Bearish_Shooting_Star = 1LL << 28,
    Bullish_Harami = 1LL << 29,
    Bearish_Harami = 1LL << 30,
    Bullish_Piercing = 1LL << 31,
    Bearish_Piercing = 1LL << 32,
    Bullish_Engulfing = 1LL << 33,
    Bearish_Engulfing = 1LL << 34,
    Bullish_Hammer = 1LL << 35,
    Bearish_Hanging_Man = 1LL << 36,



    // Generic Patterns
    Dragon_Fly_Doji = 1LL << 37,
    Grave_Stone_Doji = 1LL << 38,
    Long_Leg_Doji = 1LL << 39,

    Max = 1LL << 40,
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



enum class EStrategy : int64_t
{
    None = 0,

    Long_Strategy_1 = 1LL << 0,
    Short_Strategy_1 = 1LL << 1,

    Max = 1LL << 4,
};

// Enable bitwise ops on enum class
inline EStrategy operator|(EStrategy a, EStrategy b) {
    return static_cast<EStrategy>(
        static_cast<std::underlying_type_t<EStrategy>>(a) |
        static_cast<std::underlying_type_t<EStrategy>>(b)
        );
}

inline EStrategy operator&(EStrategy a, EStrategy b) {
    return static_cast<EStrategy>(
        static_cast<std::underlying_type_t<EStrategy>>(a) &
        static_cast<std::underlying_type_t<EStrategy>>(b)
        );
}