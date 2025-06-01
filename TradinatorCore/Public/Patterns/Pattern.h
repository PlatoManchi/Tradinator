#pragma once

#include <vector>
#include <string>

#include "TradinatorCoreTypes.h"


class CandlesData;




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