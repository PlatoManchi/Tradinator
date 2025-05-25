#pragma once

#include <cstdint>
#include <type_traits>
#include <bitset>
#include <vector>




class CandlesData;

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

/*constexpr size_t _NUM_OF_PATTERNS_ = 64;
typedef std::bitset<_NUM_OF_PATTERNS_> EPattern;

struct EPatternTypeComparator {
    constexpr bool operator() (const EPattern& b1, const EPattern& b2) const {
        return b1.to_string() < b2.to_string();
    }
};

// Doji based bullish patterns
constexpr EPattern Bullish_Long_Legged_Doji(1ULL << 0);
constexpr EPattern Bullish_Tri_Star(1ULL << 1);
constexpr EPattern Bullish_Abandoned_Baby(1ULL << 2);
constexpr EPattern Bullish_Morning_Star_Doji(1ULL << 3);
constexpr EPattern Bullish_Grave_Stone_Doji(1ULL << 4);
constexpr EPattern Bullish_Harami_Cross(1ULL << 5);

// Non doji based bullish patterns
constexpr EPattern Bullish_Three_Outside_Up(1ULL << 6);
constexpr EPattern Bullish_Three_Inside_Up(1ULL << 7);
constexpr EPattern Bullish_Matching_Low(1ULL << 8);
constexpr EPattern Bullish_Kicking(1ULL << 9);
constexpr EPattern Bullish_Three_White_Soldiers(1ULL << 10);
constexpr EPattern Bullish_Meeting_Lines(1ULL << 11);
constexpr EPattern Bullish_Morning_Star(1ULL << 12);
constexpr EPattern Bullish_Inverted_Hammer(1ULL << 13);
constexpr EPattern Bullish_Harami(1ULL << 14);
constexpr EPattern Bullish_Piercing(1ULL << 15);
constexpr EPattern Bullish_Engulfing (1ULL << 16);
constexpr EPattern Bullish_Hammer(1ULL << 17);

// Doji based bearish pattern
constexpr EPattern Bearish_Long_Legged_Doji(1ULL << 30);
constexpr EPattern Bearish_Tri_Star(1ULL << 31);
constexpr EPattern Bearish_Abandoned_Baby(1ULL << 32);
constexpr EPattern Bearish_Evening_Star_Doji(1ULL << 33);
constexpr EPattern Bearish_Bear_Stone_Doji(1ULL << 34);
constexpr EPattern Bearish_Harami_Cross(1ULL << 35);

// Non doji based bullish pattern
constexpr EPattern Bearish_Three_Outside_Down(1ULL << 36);
constexpr EPattern Bearish_Three_Inside_Down(1ULL << 37);
constexpr EPattern Bearish_Matching_High(1ULL << 38);
constexpr EPattern Bearish_Kicking(1ULL << 39);
constexpr EPattern Bearish_Three_Black_Crow(1ULL << 40);
constexpr EPattern Bearish_Meeting_Line(1ULL << 41);
constexpr EPattern Bearish_Evening_Star(1ULL << 42);
constexpr EPattern Bullish_Shooting_Star(1ULL << 43);
constexpr EPattern Bearish_Harami(1ULL << 44);
constexpr EPattern Bearish_Piercing(1ULL << 45);
constexpr EPattern Bearish_Engulfing(1ULL << 46);
constexpr EPattern Bearish_Hanging_Man(1ULL << 47);

// Generic patterns
constexpr EPattern Dragon_Fly_Doji(1ULL << 61);
constexpr EPattern Grave_Stone_Doji(1ULL << 62);
constexpr EPattern Long_Leg_Doji(1ULL << 63);*/





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

class Pattern
{
public:
    Pattern();

    virtual EPattern PatternType() const = 0;
    virtual bool IsDefaultVisible() const { return false; }
    static std::vector<uint64_t> GetPatternRangeAt(EPattern type, uint64_t at);
    /*
    * Returns the date range that satisfies the pattern.
    * Arranged in descending order. 
    * Latest date to oldest date
    */
    virtual bool Check(uint64_t at, const CandlesData& candles_data) = 0;


    std::string Name() const;
};


#pragma region BullishPatterns
/*******************************************************************************************
*                                     Bullish Long Legged Doji
********************************************************************************************/
class BullishLongLeggedDojiPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Long_Legged_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Tri Star
********************************************************************************************/
class BullishTriStarPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Tri_Star; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Abandoned Baby
********************************************************************************************/
class BullishAbandonedBabyPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Abandoned_Baby; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Morning Star
********************************************************************************************/
class BullishMorningStarPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Morning_Star; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                  Bullish Morning Star Doji
********************************************************************************************/
class BullishMorningStarDojiPattern : public BullishMorningStarPattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Morning_Star_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                  Bullish Grave Stone Doji
********************************************************************************************/
class BullishGraveStoneDojiPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Grave_Stone_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Harami
********************************************************************************************/
class BullishHaramiPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Harami; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Harami Cross
********************************************************************************************/
class BullishHaramiCrossPattern : public BullishHaramiPattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Harami_Cross; }
    virtual bool IsDefaultVisible() const { return true; }

    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Three Outside Up
********************************************************************************************/
class BullishThreeOutsideUpPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Three_Outside_Up; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Three Inside Up
********************************************************************************************/
class BullishThreeInsideUpPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Three_Inside_Up; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Matching Low
********************************************************************************************/
class BullishMatchingLowPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Matching_Low; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Kicking
********************************************************************************************/
class BullishKickingPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Kicking; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Three White Soldiers
********************************************************************************************/
class BullishThreeWhiteSoldiersPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Three_White_Soldiers; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Meeting Lines
********************************************************************************************/
class BullishMeetingLinesPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Meeting_Lines; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Inverted Hammer
********************************************************************************************/
class BullishInvertedHammerPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Inverted_Hammer; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Engulfing
********************************************************************************************/
class BullishEngulfingPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Engulfing; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Piercing
********************************************************************************************/
class BullishPiercingPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Piercing; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Hammer
********************************************************************************************/
class BullishHammerPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bullish_Hammer; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

#pragma endregion



#pragma region BearishPatterns
/*******************************************************************************************
*                                 Bearish Long Legged Doji
********************************************************************************************/
class BearishLongLeggedDojiPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Long_Legged_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Tri Star
********************************************************************************************/
class BearishTriStarPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Tri_Star; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Abandoned Baby
********************************************************************************************/
class BearishAbandonedBabyPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Abandoned_Baby; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bearish Evening Star
********************************************************************************************/
class BearishEveningStarPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Evening_Star; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Evening Star Doji
********************************************************************************************/
class BearishEveningStarDojiPattern : public BearishEveningStarPattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Evening_Star_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Grave Stone Doji
********************************************************************************************/
class BearishGraveStoneDojiPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Grave_Stone_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};


/*******************************************************************************************
*                                 Bearish Harami
********************************************************************************************/
class BearishHaramiPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Harami; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Harami Cross
********************************************************************************************/
class BearishHaramiCrossPattern : public BearishHaramiPattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Harami_Cross; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                              Bearish Three Outside Down
********************************************************************************************/
class BearishThreeOutsideDownPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Three_Outside_Down; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                              Bearish Three Inside Down
********************************************************************************************/
class BearishThreeInsideDownPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Three_Inside_Down; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                              Bearish Matching High
********************************************************************************************/
class BearishMatchingHighPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Matching_High; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                  Bearish Kicking
********************************************************************************************/
class BearishKickingPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Kicking; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bearish Three Black Crow
********************************************************************************************/
class BearishThreeBlackCrowPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Three_Black_Crow; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                   Bearish Meeting Line
********************************************************************************************/
class BearishMeetingLinePattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Meeting_Line; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                   Bearish Shooting Star
********************************************************************************************/
class BearishShootingStarPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Shooting_Star; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                      Bearish Piercing
********************************************************************************************/
class BearishPiercingPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Piercing; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                      Bearish Engulfing
********************************************************************************************/
class BearishEngulfingPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Engulfing; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bearish Hanging Man
********************************************************************************************/
class BearishHangingManPattern : public Pattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Bearish_Hanging_Man; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};


#pragma endregion









/*******************************************************************************************
*                               Dragon Fly Doji
********************************************************************************************/
class DragonFlyDojiPattern : public BullishHaramiPattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Dragon_Fly_Doji; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};



/*******************************************************************************************
*                               Grave Stone Doji
********************************************************************************************/
class GraveStonrDojiPattern : public BullishHaramiPattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Grave_Stone_Doji; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};


/*******************************************************************************************
*                               Long Leg Doji
********************************************************************************************/
class LongLegDojiPattern : public BullishHaramiPattern
{
public:
    virtual EPattern PatternType() const override { return EPattern::Long_Leg_Doji; }
    virtual bool Check(uint64_t at, const CandlesData& candles_data) override;
};