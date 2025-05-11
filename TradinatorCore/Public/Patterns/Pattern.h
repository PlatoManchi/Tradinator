#pragma once

#include <cstdint>
#include <type_traits>
#include <bitset>

#include "Data/Candle.h"
#include "Utils/Utils.h"



enum class EPatternType : uint64_t {
    None = 0,

    // Bullish patterns with Doji
    Bullish_Long_Legged_Doji = 1ULL << 0,
    Bullish_Tri_Star = 1ULL << 1,
    Bullish_Abandoned_Baby = 1ULL << 2,
    Bullish_Morning_Star_Doji = 1ULL << 3,
    Bullish_Grave_Stone_Doji = 1ULL << 4,
    Bullish_Harami_Cross = 1ULL << 5,
    
    // Bullish patterns without Doji
    Bullish_Three_Outside_Up = 1ULL << 6,
    Bullish_Three_Inside_Up = 1ULL << 7,
    Bullish_Matching_Low = 1ULL << 8,
    Bullish_Kicking = 1ULL << 9,
    Bullish_Three_White_Soldiers = 1ULL << 10,
    Bullish_Meeting_Lines = 1ULL << 11,
    Bullish_Morning_Star = 1ULL << 12,
    Bullish_Inverted_Hammer = 1ULL << 13,
    Bullish_Harami = 1ULL << 14,
    Bullish_Piercing = 1ULL << 15,
    Bullish_Engulfing = 1ULL << 16,
    Bullish_Hammer = 1ULL << 17,
    
    
    // Bearish patterns with Doji
    Bearish_Long_Legged_Doji = 1ULL << 30,
    Bearish_Tri_Star = 1ULL << 31,
    Bearish_Abandoned_Baby = 1ULL << 32,
    Bearish_Evening_Star_Doji = 1ULL << 33,
    Bearish_Grave_Stone_Doji = 1ULL << 34,
    Bearish_Harami_Cross = 1ULL << 35,
    
    // Bullish pattern without Doji
    Bearish_Three_Outside_Down = 1ULL << 36,
    Bearish_Three_Inside_Down = 1ULL << 37,
    Bearish_Matching_High = 1ULL << 38,
    Bearish_Kicking = 1ULL << 39,
    Bearish_Three_Black_Crow = 1ULL << 40,
    Bearish_Meeting_Line = 1ULL << 41,
    Bearish_Evening_Star = 1ULL << 42,
    Bearish_Shooting_Star = 1ULL << 43,
    Bearish_Harami = 1ULL << 44,
    Bearish_Piercing = 1ULL << 45,
    Bearish_Engulfing = 1ULL << 46,
    Bearish_Hanging_Man = 1ULL << 47,
    
    // Generic Patterns
    Dragon_Fly_Doji = 1ULL << 61,
    Grave_Stone_Doji = 1ULL << 62,
    Long_Leg_Doji = 1ULL << 63
};

// Enable bitwise ops on enum class
inline EPatternType operator|(EPatternType a, EPatternType b) {
    return static_cast<EPatternType>(
        static_cast<std::underlying_type_t<EPatternType>>(a) |
        static_cast<std::underlying_type_t<EPatternType>>(b)
        );
}

inline EPatternType operator&(EPatternType a, EPatternType b) {
    return static_cast<EPatternType>(
        static_cast<std::underlying_type_t<EPatternType>>(a) &
        static_cast<std::underlying_type_t<EPatternType>>(b)
        );
}

/*constexpr size_t _NUM_OF_PATTERNS_ = 64;
typedef std::bitset<_NUM_OF_PATTERNS_> EPatternType;

struct EPatternTypeComparator {
    constexpr bool operator() (const EPatternType& b1, const EPatternType& b2) const {
        return b1.to_string() < b2.to_string();
    }
};

// Doji based bullish patterns
constexpr EPatternType Bullish_Long_Legged_Doji(1ULL << 0);
constexpr EPatternType Bullish_Tri_Star(1ULL << 1);
constexpr EPatternType Bullish_Abandoned_Baby(1ULL << 2);
constexpr EPatternType Bullish_Morning_Star_Doji(1ULL << 3);
constexpr EPatternType Bullish_Grave_Stone_Doji(1ULL << 4);
constexpr EPatternType Bullish_Harami_Cross(1ULL << 5);

// Non doji based bullish patterns
constexpr EPatternType Bullish_Three_Outside_Up(1ULL << 6);
constexpr EPatternType Bullish_Three_Inside_Up(1ULL << 7);
constexpr EPatternType Bullish_Matching_Low(1ULL << 8);
constexpr EPatternType Bullish_Kicking(1ULL << 9);
constexpr EPatternType Bullish_Three_White_Soldiers(1ULL << 10);
constexpr EPatternType Bullish_Meeting_Lines(1ULL << 11);
constexpr EPatternType Bullish_Morning_Star(1ULL << 12);
constexpr EPatternType Bullish_Inverted_Hammer(1ULL << 13);
constexpr EPatternType Bullish_Harami(1ULL << 14);
constexpr EPatternType Bullish_Piercing(1ULL << 15);
constexpr EPatternType Bullish_Engulfing (1ULL << 16);
constexpr EPatternType Bullish_Hammer(1ULL << 17);

// Doji based bearish pattern
constexpr EPatternType Bearish_Long_Legged_Doji(1ULL << 30);
constexpr EPatternType Bearish_Tri_Star(1ULL << 31);
constexpr EPatternType Bearish_Abandoned_Baby(1ULL << 32);
constexpr EPatternType Bearish_Evening_Star_Doji(1ULL << 33);
constexpr EPatternType Bearish_Bear_Stone_Doji(1ULL << 34);
constexpr EPatternType Bearish_Harami_Cross(1ULL << 35);

// Non doji based bullish pattern
constexpr EPatternType Bearish_Three_Outside_Down(1ULL << 36);
constexpr EPatternType Bearish_Three_Inside_Down(1ULL << 37);
constexpr EPatternType Bearish_Matching_High(1ULL << 38);
constexpr EPatternType Bearish_Kicking(1ULL << 39);
constexpr EPatternType Bearish_Three_Black_Crow(1ULL << 40);
constexpr EPatternType Bearish_Meeting_Line(1ULL << 41);
constexpr EPatternType Bearish_Evening_Star(1ULL << 42);
constexpr EPatternType Bullish_Shooting_Star(1ULL << 43);
constexpr EPatternType Bearish_Harami(1ULL << 44);
constexpr EPatternType Bearish_Piercing(1ULL << 45);
constexpr EPatternType Bearish_Engulfing(1ULL << 46);
constexpr EPatternType Bearish_Hanging_Man(1ULL << 47);

// Generic patterns
constexpr EPatternType Dragon_Fly_Doji(1ULL << 61);
constexpr EPatternType Grave_Stone_Doji(1ULL << 62);
constexpr EPatternType Long_Leg_Doji(1ULL << 63);*/





const EPatternType Bullish_Pattern_Type = 
    EPatternType::Bullish_Long_Legged_Doji | 
    EPatternType::Bullish_Tri_Star |
    EPatternType::Bullish_Abandoned_Baby |
    EPatternType::Bullish_Morning_Star_Doji |
    EPatternType::Bullish_Grave_Stone_Doji |
    EPatternType::Bullish_Harami_Cross |
    EPatternType::Bullish_Three_Outside_Up |
    EPatternType::Bullish_Three_Inside_Up |
    EPatternType::Bullish_Matching_Low |
    EPatternType::Bullish_Kicking |
    EPatternType::Bullish_Three_White_Soldiers |
    EPatternType::Bullish_Meeting_Lines |
    EPatternType::Bullish_Morning_Star |
    EPatternType::Bullish_Inverted_Hammer |
    EPatternType::Bullish_Harami |
    EPatternType::Bullish_Piercing |
    EPatternType::Bullish_Engulfing |
    EPatternType::Bullish_Hammer;



const EPatternType Bearish_Pattern_Type = 
    EPatternType::Bearish_Long_Legged_Doji |
    EPatternType::Bearish_Tri_Star |
    EPatternType::Bearish_Abandoned_Baby |
    EPatternType::Bearish_Evening_Star_Doji |
    EPatternType::Bearish_Grave_Stone_Doji |
    EPatternType::Bearish_Harami_Cross |
    EPatternType::Bearish_Three_Outside_Down |
    EPatternType::Bearish_Three_Inside_Down |
    EPatternType::Bearish_Matching_High |
    EPatternType::Bearish_Kicking |
    EPatternType::Bearish_Three_Black_Crow |
    EPatternType::Bearish_Meeting_Line |
    EPatternType::Bearish_Evening_Star |
    EPatternType::Bearish_Shooting_Star |
    EPatternType::Bearish_Harami |
    EPatternType::Bearish_Piercing |
    EPatternType::Bearish_Engulfing |
    EPatternType::Bearish_Hanging_Man;

class Pattern
{
public:
    Pattern();

    virtual EPatternType PatternType() const = 0;
    virtual bool IsDefaultVisible() const { return false; }

    /*
    * Returns the date range that satisfies the pattern.
    * Arranged in descending order. 
    * Latest date to oldest date
    */
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) = 0;


    std::string Name() const
    {
        return TradinatorCoreSpace::Utils::GetPatternShortDescription(PatternType());
    }
};


#pragma region BullishPatterns
/*******************************************************************************************
*                                     Bullish Long Legged Doji
********************************************************************************************/
class BullishLongLeggedDojiPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Long_Legged_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Tri Star
********************************************************************************************/
class BullishTriStarPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Tri_Star; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Abandoned Baby
********************************************************************************************/
class BullishAbandonedBabyPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Abandoned_Baby; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Morning Star
********************************************************************************************/
class BullishMorningStarPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Morning_Star; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                  Bullish Morning Star Doji
********************************************************************************************/
class BullishMorningStarDojiPattern : public BullishMorningStarPattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Morning_Star_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                  Bullish Grave Stone Doji
********************************************************************************************/
class BullishGraveStoneDojiPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Grave_Stone_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Harami
********************************************************************************************/
class BullishHaramiPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Harami; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Harami Cross
********************************************************************************************/
class BullishHaramiCrossPattern : public BullishHaramiPattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Harami_Cross; }
    virtual bool IsDefaultVisible() const { return true; }

    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Three Outside Up
********************************************************************************************/
class BullishThreeOutsideUpPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Three_Outside_Up; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Three Inside Up
********************************************************************************************/
class BullishThreeInsideUpPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Three_Inside_Up; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Matching Low
********************************************************************************************/
class BullishMatchingLowPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Matching_Low; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Kicking
********************************************************************************************/
class BullishKickingPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Kicking; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Three White Soldiers
********************************************************************************************/
class BullishThreeWhiteSoldiersPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Three_White_Soldiers; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Meeting Lines
********************************************************************************************/
class BullishMeetingLinesPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Meeting_Lines; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bullish Inverted Hammer
********************************************************************************************/
class BullishInvertedHammerPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Inverted_Hammer; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Engulfing
********************************************************************************************/
class BullishEngulfingPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Engulfing; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Piercing
********************************************************************************************/
class BullishPiercingPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Piercing; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bullish Hammer
********************************************************************************************/
class BullishHammerPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bullish_Hammer; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

#pragma endregion



#pragma region BearishPatterns
/*******************************************************************************************
*                                 Bearish Long Legged Doji
********************************************************************************************/
class BearishLongLeggedDojiPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Long_Legged_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Tri Star
********************************************************************************************/
class BearishTriStarPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Tri_Star; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Abandoned Baby
********************************************************************************************/
class BearishAbandonedBabyPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Abandoned_Baby; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bearish Evening Star
********************************************************************************************/
class BearishEveningStarPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Evening_Star; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Evening Star Doji
********************************************************************************************/
class BearishEveningStarDojiPattern : public BearishEveningStarPattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Evening_Star_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Grave Stone Doji
********************************************************************************************/
class BearishGraveStoneDojiPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Grave_Stone_Doji; }
    virtual bool IsDefaultVisible() const { return true; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};


/*******************************************************************************************
*                                 Bearish Harami
********************************************************************************************/
class BearishHaramiPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Harami; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                 Bearish Harami Cross
********************************************************************************************/
class BearishHaramiCrossPattern : public BearishHaramiPattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Harami_Cross; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                              Bearish Three Outside Down
********************************************************************************************/
class BearishThreeOutsideDownPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Three_Outside_Down; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                              Bearish Three Inside Down
********************************************************************************************/
class BearishThreeInsideDownPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Three_Inside_Down; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                              Bearish Matching High
********************************************************************************************/
class BearishMatchingHighPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Matching_High; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                  Bearish Kicking
********************************************************************************************/
class BearishKickingPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Kicking; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                               Bearish Three Black Crow
********************************************************************************************/
class BearishThreeBlackCrowPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Three_Black_Crow; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                   Bearish Meeting Line
********************************************************************************************/
class BearishMeetingLinePattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Meeting_Line; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                   Bearish Shooting Star
********************************************************************************************/
class BearishShootingStarPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Shooting_Star; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                      Bearish Piercing
********************************************************************************************/
class BearishPiercingPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Piercing; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                      Bearish Engulfing
********************************************************************************************/
class BearishEngulfingPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Engulfing; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

/*******************************************************************************************
*                                     Bearish Hanging Man
********************************************************************************************/
class BearishHangingManPattern : public Pattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Bearish_Hanging_Man; }
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};


#pragma endregion









/*******************************************************************************************
*                               Dragon Fly Doji
********************************************************************************************/
class DragonFlyDojiPattern : public BullishHaramiPattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Dragon_Fly_Doji; }
    
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override
    {
        if (candles_data.IsDragonflyDoji(at))
        {
            return { at };
        }

        return {};
    }
};



/*******************************************************************************************
*                               Grave Stone Doji
********************************************************************************************/
class GraveStonrDojiPattern : public BullishHaramiPattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Grave_Stone_Doji; }
    
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override
    {
        if (candles_data.IsGravestoneDoji(at))
        {
            return { at };
        }

        return {};
    }
};


/*******************************************************************************************
*                               Long Leg Doji
********************************************************************************************/
class LongLegDojiPattern : public BullishHaramiPattern
{
public:
    virtual EPatternType PatternType() const override { return EPatternType::Long_Leg_Doji; }
    
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override
    {
        if (candles_data.IsLongLegDoji(at))
        {
            return { at };
        }

        return {};
    }
};