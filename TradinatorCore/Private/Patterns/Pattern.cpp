#include "Patterns/Pattern.h"


#include <cassert>



#include "Data/Candle.h"
#include "Utils/Utils.h"


/*

        |
       ---
       |:|      ->    Bearish Candle
       ---
        |

        |
       ---
       | |     ->    Bullish Candle
       ---
        |

        |
       ---
       |.|     ->     Either Bullish or Bearish Candle
       ---
        |

        |
       ---     ->     Doji
        |

       ---
       | |
       | |     -> Marubozu candle
       | |
       ---
*/


Pattern::Pattern()
{
    
}

std::string Pattern::Name() const
{
    return TradinatorCoreSpace::Utils::GetPatternShortDescription(PatternType());
}

std::vector<uint64_t> Pattern::GetPatternRangeAt(EPattern type, uint64_t at)
{
    switch (type)
    {
    case EPattern::None:
        assert("Invalid pattern");
        return {};

    case EPattern::Bullish_Hammer:
    case EPattern::Bearish_Hanging_Man:
    case EPattern::Dragon_Fly_Doji:
    case EPattern::Grave_Stone_Doji:
    case EPattern::Long_Leg_Doji:
        return { at };

    case EPattern::Bullish_Long_Legged_Doji:
    case EPattern::Bearish_Long_Legged_Doji:
        return { at + 1 };

    case EPattern::Bullish_Grave_Stone_Doji:
    case EPattern::Bullish_Harami:
    case EPattern::Bullish_Harami_Cross:
    case EPattern::Bullish_Matching_Low:
    case EPattern::Bullish_Kicking:
    case EPattern::Bullish_Meeting_Lines:
    case EPattern::Bullish_Inverted_Hammer:
    case EPattern::Bullish_Piercing:
    case EPattern::Bullish_Engulfing:
    case EPattern::Bearish_Grave_Stone_Doji:
    case EPattern::Bearish_Harami:
    case EPattern::Bearish_Harami_Cross:
    case EPattern::Bearish_Matching_High:
    case EPattern::Bearish_Kicking:
    case EPattern::Bearish_Meeting_Line:
    case EPattern::Bearish_Shooting_Star:
    case EPattern::Bearish_Piercing:
    case EPattern::Bearish_Engulfing:
        return { at, at + 1 };

    case EPattern::Bullish_Tri_Star:
    case EPattern::Bullish_Abandoned_Baby:
    case EPattern::Bullish_Morning_Star:
    case EPattern::Bullish_Morning_Star_Doji:
    case EPattern::Bullish_Three_Outside_Up:
    case EPattern::Bullish_Three_Inside_Up:
    case EPattern::Bullish_Three_White_Soldiers:
    case EPattern::Bearish_Tri_Star:
    case EPattern::Bearish_Abandoned_Baby:
    case EPattern::Bearish_Evening_Star:
    case EPattern::Bearish_Evening_Star_Doji:
    case EPattern::Bearish_Three_Outside_Down:
    case EPattern::Bearish_Three_Inside_Down:
    case EPattern::Bearish_Three_Black_Crow:
        return { at, at + 1, at + 2 };
    
    case EPattern::Max:
        assert("Invalid pattern");
        return {};

    default:
        assert("Pattern missing. Add missing pattern to switch.");
        return {};

    }

    assert("Invalid pattern");
    return {};
}


#pragma region BullishPatterns
/*******************************************************************************************
*                                     Bullish Long Legged Doji
********************************************************************************************/
bool BullishLongLeggedDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
        //&& at < candles_data.m_trends.size())
    {
        /*          Curr       Next                Confirmation
                      |
                Open ---                               |
                     |:|        |                     ---
                     |:|        |                     | |
                     |:|       ---  Open, Close       | |
                     |:|        |                     | |
                     |:|        |                     ---
                     |:|                               |
               Close ---
                      |
        */
        bool is_pattern_matched =
            //candles_data.m_trends[at] == ETrend::Down && // currently trend detection isn't stable
            candles_data.IsBearish(at) &&           // Prev is bear
            candles_data.IsLongLegDoji(at + 1) &&
            candles_data.IsBullish(at + 2) && candles_data.m_closes[at + 2] > candles_data.m_highs[at + 1]; // Confirmation with next candle

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}

/*******************************************************************************************
*                                     Bullish Tri Star
********************************************************************************************/
bool BullishTriStarPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
        //&& at < candles_data.m_trends.size())
    {
        /*
                        |        
                       ---              |
                        |              ---
                                        |
                               |
                              --- (Middle body below first and third bodies)
                               |
        */
        bool is_pattern_matched =
            //candles_data.m_trends[at] == ETrend::Down && // currently trend detection isn't stable
            candles_data.IsDoji(at + 2) &&
            candles_data.IsDoji(at + 1) &&
            candles_data.IsDoji(at) &&
            candles_data.m_closes[at + 1] < candles_data.m_closes[at] &&  // If middle doji body is below prev and curr
            candles_data.m_closes[at + 1] < candles_data.m_closes[at + 2];

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                     Bullish Abandoned Baby
********************************************************************************************/
bool BullishAbandonedBabyPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                 |
                ---
                |:|                 |
                |:|                --- Closes above 50% of first candle
                |:|                | |
                |:|                | |
                |:|                | |
                ---                | |
                 |                 ---
                                    |
                           | (Gap from Frist and Third candles)
                          ---
                           |
        */
        bool is_pattern_matched =
            candles_data.IsDoji(at + 1) &&          // Middle is doji
            candles_data.IsBearish(at) &&           // first candle is bearish
            candles_data.IsBullish(at + 2) &&       // third candle is bullish
            candles_data.IsLongCandle(at) &&        // Strong bearish first candle
            candles_data.IsLongCandle(at + 2) &&    // Strong bullish third candle
            candles_data.m_highs[at + 1] < candles_data.m_lows[at] &&         // gap down on from middle
            candles_data.m_highs[at + 1] < candles_data.m_lows[at + 2] &&     // gap down on from middle
            candles_data.m_closes[at + 2] > (candles_data.m_opens[at] + candles_data.m_closes[at]) / 2.0; // Current is above half of first candle

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                               Bullish Morning Star
********************************************************************************************/
bool BullishMorningStarPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                 |
                ---
                |:|                 |
                |:|                --- (Closes above 50% of first candle)
                |:|                | |
                |:|                | |
                |:|                | |
                |:|                | |
                |:|                --- (Third open above middle body top, shadows can overlap with middle)
                ---                 |
                 |         | (Middle shadow should overlap with third or first shadow)
                          ---
                          |.|
                          ---
                           |
        */

        double middle_top = candles_data.IsBullish(at + 1) ? candles_data.m_closes[at + 1] : candles_data.m_opens[at + 1];
        bool is_pattern_matched =
            candles_data.IsBearish(at) &&           // first candle is bearish
            candles_data.IsLongCandle(at) &&        // Strong bearish first candle
            candles_data.IsBullish(at + 2) &&       // third candle is bullish
            candles_data.IsLongCandle(at + 2) &&    // Strong bullish third candle
            (candles_data.m_highs[at + 1] > candles_data.m_lows[at] || 
                candles_data.m_highs[at + 1] > candles_data.m_lows[at + 2]) && // first candle or third candle overlapping shadow with middle candle
            middle_top < candles_data.m_closes[at] && middle_top < candles_data.m_opens[at + 2] && // middle body is below and not overlapping with shadow
            candles_data.m_opens[at + 2] > middle_top &&       // third body is above middle body 
            candles_data.m_closes[at + 2] > (candles_data.m_opens[at] + candles_data.m_closes[at]) / 2.0; // Third closes above half of first candle

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}

/*******************************************************************************************
*                                  Bullish Morning Star Doji
********************************************************************************************/
bool BullishMorningStarDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                 |
                ---
                |:|                 |
                |:|                ---
                |:|                | |
                |:|                | |
                |:|                | |
                |:|                | |
                |:|                ---
                ---                 |
                 |         |
                          ---
                           |
        */
        if (candles_data.IsDoji(at + 1))
        {
            return BullishMorningStarPattern::Check(at, candles_data);
        }
    }

    return false;
}

/*******************************************************************************************
*                                  Bullish Grave Stone Doji
********************************************************************************************/
bool BullishGraveStoneDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                 |                
                ---            Confirmation
                |:|                 |
                |:|                ---
                |:|                | |
                |:|                | | 
                |:|        |       | |
                |:|        |       | |
                |:|        |       ---
                ---        |        |
                 |         |
                          ---
                           |
        */
        bool is_pattern_matched =
            candles_data.IsBearish(at) &&
            candles_data.IsGravestoneDoji(at + 1) &&
            candles_data.IsBullish(at + 2);

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                     Bullish Harami
********************************************************************************************/
bool BullishHaramiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*          First      Middle
                      |                     Confirmation
                Open ---                         |
                     |:|        |               ---
                     |:|       ---  Close       | |
                     |:|       | |              | |
                     |:|       | |              | |
                     |:|       ---  Open        | |
                     |:|        |               ---
               Close ---                         |
                      |

        */

        bool is_pattern_matched = candles_data.IsBearish(at) &&           // First is bearish
            candles_data.IsBullish(at + 1) &&                                 // Middle is bullish    
            candles_data.m_opens[at] > candles_data.m_closes[at + 1] &&   // First open is above Middle close
            candles_data.m_closes[at] < candles_data.m_opens[at + 1] &&   // First close is below Middle open
            candles_data.IsBullish(at + 2);  // Confirmation

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}



/*******************************************************************************************
*                               Bullish Harami Cross
********************************************************************************************/
bool BullishHaramiCrossPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    std::vector<uint64_t> result;

    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*          Prev       Curr
                      |
                Open ---
                     |:|
                     |:|        |
                     |:|       ---  Open, Close
                     |:|        |
                     |:|
                     |:|
               Close ---
                      |
        */


        if (candles_data.IsDoji(at + 1))
        {
            return BullishHaramiPattern::Check(at, candles_data);
        }
    }

    return false;
}



/*******************************************************************************************
*                               Bullish Three Outside Up
********************************************************************************************/
bool BullishThreeOutsideUpPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*                  |
                      |    ---
                |    ---   | |
               ---   | |   | |
               |:|   | |   | |
               |:|   | |   ---
               |:|   | |    |
               ---   | |
                |    ---
                      |
        */

        bool is_pattern_matched =
            candles_data.IsBearish(at) &&
            candles_data.IsBullish(at + 1) &&
            candles_data.IsBullish(at + 2) &&
            candles_data.m_opens[at + 1] < candles_data.m_closes[at] &&
            candles_data.m_closes[at + 1] > candles_data.m_opens[at] &&
            candles_data.m_closes[at + 2] > candles_data.m_closes[at + 1];
        
        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                               Bullish Three Inside Up
********************************************************************************************/
bool BullishThreeInsideUpPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                                |
                               ---
                               | |
             |                 | |
            ---                | |
            |:|       |        | |
            |:|      ---       ---
            |:|      | |        |
            |:|      | |     
            |:|      | |      
            |:|      ---
            |:|       |
            ---    
             |     
        */
        bool is_pattern_matched = 
            candles_data.IsBearish(at) &&
            candles_data.IsLongCandle(at) &&
            candles_data.IsBullish(at + 1) &&
            candles_data.IsBullish(at + 2) &&
            candles_data.m_opens[at + 1] > candles_data.m_closes[at] &&
            candles_data.m_closes[at + 1] < candles_data.m_opens[at] &&
            candles_data.m_closes[at + 2] > candles_data.m_opens[at];

        if (is_pattern_matched)
        {
            return true;
        }
    }
    return false;
}

/*******************************************************************************************
*                               Bullish Matching Low
********************************************************************************************/
bool BullishMatchingLowPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*      |
               ---     |
               |:|    ---
               |:|    |:|
               |:|    |:|
               |:|    |:|
               ---    ---
                |      |
        */
        double longest_body = std::max(fabs(candles_data.m_closes[at] - candles_data.m_opens[at]), fabs(candles_data.m_closes[at + 1] - candles_data.m_opens[at + 1]));
        double small_threshold = longest_body * 0.05; // 5% threshold

        bool is_pattern_matched = 
            candles_data.IsBearish(at) &&
            !candles_data.IsDoji(at) &&
            candles_data.IsBearish(at + 1) &&
            !candles_data.IsDoji(at + 1) &&
            fabs(candles_data.m_closes[at] - candles_data.m_closes[at + 1]) <= small_threshold;
        
        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                               Bullish Kicking
********************************************************************************************/
bool BullishKickingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*
                      ---
                      | |
                      | |
                      | |
                      ---        

             ---
             |:|
             |:|
             |:|
             ---
        */
        bool is_pattern_matched = 
            candles_data.IsBearish(at) &&
            candles_data.IsMarubozu(at) &&
            candles_data.IsBullish(at + 1) &&
            candles_data.IsMarubozu(at + 1) &&
            candles_data.m_lows[at + 1] > candles_data.m_highs[at]; // gap up between first and second

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                               Bullish Three White Soldiers
********************************************************************************************/
bool BullishThreeWhiteSoldiersPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*                        |
                                 ---
                        |        | |
                       ---       | |
               |       | |       | |
              ---      | |       ---
              | |      | |        |
              | |      ---
              | |       |
              | |    
              ---    
               |     
        */
        bool is_pattern_matched = 
            candles_data.IsBullish(at) &&
            candles_data.IsLongCandle(at) &&
            candles_data.IsBullish(at + 1) &&
            candles_data.IsLongCandle(at + 1) &&
            candles_data.IsBullish(at + 2) &&
            candles_data.IsLongCandle(at + 2) &&
            candles_data.m_opens[at] > candles_data.m_opens[at + 1] &&
            candles_data.m_closes[at] > candles_data.m_closes[at + 1] &&
            candles_data.m_opens[at + 1] > candles_data.m_opens[at + 2] &&
            candles_data.m_closes[at + 1] > candles_data.m_closes[at + 2];

        if (is_pattern_matched)
        {
            return true;
        }
    }
    return false;
}


/*******************************************************************************************
*                               Bullish Meeting Lines
********************************************************************************************/
bool BullishMeetingLinesPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
              |
             ---
             |:|    --> First close and Second close are almost at same line
             |:|    |
             |:|    |
             |:|    |   |                     Confirmation
             ---.......---                         |
              |        | |                        ---
                       | |                        | |
                       | |                        | |
                       --- (Second open           ---
                        |   Below first close)     |
        */
        double longest_body = std::max(fabs(candles_data.m_closes[at] - candles_data.m_opens[at]), fabs(candles_data.m_closes[at + 1] - candles_data.m_opens[at + 1]));
        double small_threshold = longest_body * 0.05; // 5% threshold

        bool is_pattern_matched =
            candles_data.IsBearish(at) && 
            candles_data.IsLongCandle(at) &&
            candles_data.IsBullish(at + 1) && 
            candles_data.IsLongCandle(at + 1) &&
            candles_data.m_opens[at + 1] < candles_data.m_closes[at] &&
            fabs(candles_data.m_closes[at] - candles_data.m_closes[at + 1]) < small_threshold &&
            candles_data.IsBullish(at + 2);    // Option: Confirms the pattern

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}

/*******************************************************************************************
*                               Bullish Inverted Hammer
********************************************************************************************/
bool BullishInvertedHammerPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
               |
              ---
              |:|
              |:|        |       Confirmation
              |:|        |           |
              |:|        |          ---
              |:|       ---         | |
              ---       |.|         | |
               |        ---         ---
                                     |
        */                           
        bool is_pattern_matched = 
            candles_data.IsBearish(at) &&
            candles_data.IsInvertedHammer(at + 1) &&
            candles_data.IsBullish(at + 2);    // Confirmation

        if (is_pattern_matched)
        {
            return true;
        }
    }
    return false;
}

/*******************************************************************************************
*                                     Bullish Piercing
********************************************************************************************/
bool BullishPiercingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*         First       Second  
                      |      
                Open ---         |
                     |:|        --- Close (Above 50% of prev bearish candle)
                     |:|        | |
                     |:|        | |
                     |:|        | |
                     |:|        --- Open
               Close ---         |
                      |      
        */

        bool is_pattern_matched = 
            candles_data.IsBearish(at) && 
            candles_data.IsBullish(at + 1) &&
            candles_data.m_closes[at + 1] > (candles_data.m_opens[at] + candles_data.m_closes[at]) / 2.0;

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}

/*******************************************************************************************
*                                     Bullish Engulfing
********************************************************************************************/
bool BullishEngulfingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*         First     Second
                                |
                               ---  Close
                      |        | |
                Open ---       | |
                     |:|       | |
                     |:|       | |
               Close ---       | |
                      |        | |
                               ---  Open
                                |

        */
        bool is_pattern_matched = 
            candles_data.IsBearish(at) && 
            candles_data.IsBullish(at + 1) &&
            candles_data.m_closes[at + 1] > candles_data.m_opens[at] &&     // Second close is above First open
            candles_data.m_opens[at + 1] < candles_data.m_closes[at];       // Second open is below First close

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}

/*******************************************************************************************
*                                     Bullish Hammer
********************************************************************************************/
bool BullishHammerPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*
                  Confirmation
                       |
                      ---
              ---     | |
              |.|     ---
              ---      |
               |
               |
               |
               |
        */

        bool is_pattern_matched = 
            candles_data.IsHammer(at) &&
            candles_data.IsBullish(at + 1);

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}

#pragma endregion



#pragma region BearishPatterns
/*******************************************************************************************
*                                 Bearish Long Legged Doji
********************************************************************************************/
bool BearishLongLeggedDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
        //&& at < candles_data.m_trends.size())
    {
        /*          First                            
                      |                              Third
                Open ---      Middle                   |
                     | |        |                     ---
                     | |        |                     |:|
                     | |       ---  Open, Close       |:|
                     | |        |                     |:|
                     | |        |                     ---
                     | |                               |
               Close ---
                      |
        */
        bool is_pattern_matched =
            //candles_data.m_trends[at] == ETrend::Up && // currently trend detection isn't stable
            candles_data.IsBullish(at) &&           // First is bullish
            candles_data.IsLongLegDoji(at + 1) &&   // Middle is long legged doji
            candles_data.IsBearish(at + 2) && candles_data.m_closes[at + 2] < candles_data.m_lows[at + 1]; // Confirmation with next candle

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                 Bearish Tri Star
********************************************************************************************/
bool BearishTriStarPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
        //&& at < candles_data.m_trends.size())
    {
        /*
                                 | 
                                ---
                                 |

                        |
                       ---                |
                        |                ---
                                          |
        */
        bool is_pattern_matched =
            //candles_data.m_trends[at] == ETrend::Up && // currently trend detection isn't stable
            candles_data.IsDoji(at) &&
            candles_data.IsDoji(at + 1) &&
            candles_data.IsDoji(at + 2) &&
            candles_data.m_closes[at + 1] > candles_data.m_closes[at] &&  // If middle doji body is below prev and curr
            candles_data.m_closes[at + 1] > candles_data.m_closes[at + 2];

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                 Bearish Abandoned Baby
********************************************************************************************/
bool BearishAbandonedBabyPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                            | 
                           ---
                            | (Gap between first and third candles)
                 
                 |
                ---
                | |                 |
                | |                ---
                | |                |:|
                | |                |:|
                | |                |:|
                ---                |:|
                 |                 --- ( Closes below 50% of first candle body, Not required but strengthens signal)
                                    |
        */

        bool is_pattern_matched =
            candles_data.IsBullish(at) &&
            candles_data.IsLongCandle(at) &&
            candles_data.IsDoji(at + 1) &&
            candles_data.IsBearish(at + 2) &&
            candles_data.IsLongCandle(at + 2) &&
            candles_data.m_lows[at + 1] > candles_data.m_highs[at] &&         // gap up between middle and first candle
            candles_data.m_highs[at + 2] < candles_data.m_lows[at + 1] &&     // gap down between middle and third candle
            candles_data.m_closes[at + 2] < (candles_data.m_opens[at] + candles_data.m_closes[at]) / 2.0;
        
        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                               Bearish Evening Star
********************************************************************************************/
bool BearishEveningStarPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                          |
                         ---
                         |.|
                         ---
                          |        |
                                  ---
                 |                |:|
                ---               |:|
                | |               |:| 
                | |               |:|
                | |               ---
                | |                |
                | |                
                | |                
                | |                
                ---                
                 |       
        */
        double middle_bottom = candles_data.IsBullish(at + 1) ? candles_data.m_opens[at + 1] : candles_data.m_closes[at + 1];

        bool is_pattern_matched =
            candles_data.IsBullish(at) &&
            candles_data.IsLongCandle(at) &&
            candles_data.IsBearish(at + 2) &&
            candles_data.IsLongCandle(at + 2) &&
            (candles_data.m_lows[at + 1] < candles_data.m_highs[at] ||
                candles_data.m_lows[at + 1] < candles_data.m_highs[at + 2]) && // Middle shadow is overlapping with first or third candle
            middle_bottom > candles_data.m_closes[at] && middle_bottom > candles_data.m_opens[at + 2] &&// Middle body is not overlapping
            candles_data.m_closes[at + 2] < (candles_data.m_opens[at] + candles_data.m_closes[at]) / 2.0; // third close is below 50% of first candle

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}

/*******************************************************************************************
*                                 Bearish Evening Star Doji
********************************************************************************************/
bool BearishEveningStarDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                          |
                         ---
                          |        |
                                  ---
                 |                |:|
                ---               |:|
                | |               |:|
                | |               |:|
                | |               ---
                | |                |
                | |
                | |
                | |
                ---
                 |
        */
        if (candles_data.IsDoji(at + 1))
        {
            return BearishEveningStarPattern::Check(at, candles_data);
        }
    }

    return false;
}


/*******************************************************************************************
*                                 Bearish Grave Stone Doji
********************************************************************************************/
bool BearishGraveStoneDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*                |   
                          |   
                          |   
                          |   
                          |
                 |       ---
                ---       |
                | |             |
                | |            ---
                | |            |:|
                | |            |:|
                | |            |:|
                | |            |:|
                | |            ---
                ---             |
                 |         
        */

        bool is_pattern_matched = 
            candles_data.IsBullish(at) &&
            candles_data.IsGravestoneDoji(at + 1) &&
            candles_data.IsBearish(at + 2);

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                 Bearish Harami
********************************************************************************************/
bool BearishHaramiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /* 
                      |                     Confirmation
                     ---                         |
                     | |        |               ---
                     | |       ---              |:|
                     | |       |:|              |:|
                     | |       |:|              |:|
                     | |       ---              |:|
                     | |        |               ---
                     ---                         |
                      |

        */

        bool is_pattern_matched = 
            candles_data.IsBullish(at) &&
            candles_data.IsBearish(at + 1) &&
            candles_data.m_opens[at] > candles_data.m_closes[at + 1] &&   // prev open is above prev close
            candles_data.m_closes[at] < candles_data.m_opens[at + 1] &&   // prev close is below curropen
            candles_data.IsBearish(at + 2);

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                 Bearish Harami Cross
********************************************************************************************/
bool BearishHaramiCrossPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*
                      |                     Confirmation
                     ---                         |
                     | |                        ---
                     | |                        |:|
                     | |        |               |:|
                     | |       ---              |:|
                     | |        |               |:|
                     | |                        ---
                     ---                         |
                      |

        */
        if (candles_data.IsDoji(at + 1))
        {
            return BearishHaramiPattern::Check(at, candles_data);
        }
    }
    return false;
}


/*******************************************************************************************
*                              Bearish Three Outside Down
********************************************************************************************/
bool BearishThreeOutsideDownPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*                
                      |   
                |    ---  
               ---   |:|  
               | |   |:|  
               | |   |:|  
               | |   |:|    | 
               ---   |:|   ---
                |    ---   |:|
                      |    |:|
                           |:|
                           ---
                            |
        */

        bool is_pattern_matched = 
            candles_data.IsBullish(at) &&
            candles_data.IsBearish(at + 1) &&
            candles_data.IsBearish(at + 2) &&
            candles_data.m_opens[at + 1] > candles_data.m_closes[at] &&
            candles_data.m_closes[at + 1] < candles_data.m_opens[at] &&
            candles_data.m_closes[at + 2] < candles_data.m_closes[at + 1];

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                              Bearish Three Inside Down
********************************************************************************************/
bool BearishThreeInsideDownPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                      
                |    
               ---    |
               | |   ---
               | |   |:|
               | |   ---    |
               ---    |    ---
                |          |:|
                           |:|
                           |:|
                           ---
                            |
        */

        bool is_pattern_matched =
            candles_data.IsBullish(at) &&
            candles_data.IsBearish(at + 1) &&
            candles_data.IsBearish(at + 2) &&
            candles_data.m_opens[at + 1] < candles_data.m_closes[at] &&
            candles_data.m_closes[at + 1] > candles_data.m_opens[at] &&
            candles_data.m_closes[at + 2] < candles_data.m_closes[at + 1];

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                              Bearish Matching High
********************************************************************************************/
bool BearishMatchingHighPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*      |      |
               ---    ---
               |:|    |:|
               |:|    |:|
               |:|    |:|
               |:|    ---
               ---     |
                |      
        */
        double longest_body = std::max(fabs(candles_data.m_closes[at] - candles_data.m_opens[at]), fabs(candles_data.m_closes[at + 1] - candles_data.m_opens[at + 1]));
        double small_threshold = longest_body * 0.05; // 5% threshold

        bool is_pattern_matched =
            candles_data.IsBearish(at) &&
            !candles_data.IsDoji(at) &&
            candles_data.IsBearish(at + 1) &&
            !candles_data.IsDoji(at + 1) &&
            fabs(candles_data.m_opens[at] - candles_data.m_opens[at + 1]) <= small_threshold;

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                  Bearish Kicking
********************************************************************************************/
bool BearishKickingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*   ---
             | |
             | |
             | |
             ---     
                       ---
                       |:|
                       |:|
                       |:|
                       ---
        */

        bool is_pattern_matched = 
            candles_data.IsBullish(at) &&
            candles_data.IsMarubozu(at) &&
            candles_data.IsBearish(at + 1) &&
            candles_data.IsMarubozu(at + 1) &&
            candles_data.m_lows[at] > candles_data.m_highs[at + 1];

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                               Bearish Three Black Crow
********************************************************************************************/
bool BearishThreeBlackCrowPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*   |                      
            ---                     
            |:|                     
            |:|     |               
            |:|    ---           
            |:|    |:|           
            ---    |:|     |     
             |     |:|    ---    
                   |:|    |:|    
                   ---    |:|    
                    |     |:|    
                          |:| 
                          --- 
                           | 
        */

        bool is_pattern_matched = 
            candles_data.IsBearish(at) &&
            candles_data.IsLongCandle(at) &&
            candles_data.IsBearish(at + 1) &&
            candles_data.IsLongCandle(at + 1) &&
            candles_data.IsBearish(at + 2) &&
            candles_data.IsLongCandle(at + 2) &&
            candles_data.m_closes[at] > candles_data.m_closes[at + 1] &&
            candles_data.m_opens[at] > candles_data.m_opens[at + 1] &&
            candles_data.m_closes[at + 1] > candles_data.m_closes[at + 2] &&
            candles_data.m_opens[at + 1] > candles_data.m_opens[at + 2];

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                   Bearish Meeting Line
********************************************************************************************/
bool BearishMeetingLinePattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*
                        |
                       ---       Confirmation
                       |:|            |
                |      |:|           ---
               ---     ---           |:|
               | |      |            |:|
               | |                   |:|
               | |                   |:|
               | |                   ---
               ---                    |
                |
        */

        double longest_body = std::max(fabs(candles_data.m_closes[at] - candles_data.m_opens[at]), fabs(candles_data.m_closes[at + 1] - candles_data.m_opens[at + 1]));
        double small_threshold = longest_body * 0.05; // 5% threshold

        bool is_pattern_matched =
            candles_data.IsBullish(at) && 
            candles_data.IsLongCandle(at) &&
            candles_data.IsBearish(at + 1) && 
            candles_data.IsLongCandle(at + 1) &&
            candles_data.m_opens[at + 1] > candles_data.m_closes[at] &&
            fabs(candles_data.m_closes[at] - candles_data.m_closes[at + 1]) < small_threshold &&
            candles_data.IsBearish(at + 2);    // Option: Confirms the pattern

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                   Bearish Shooting Star
********************************************************************************************/
bool BearishShootingStarPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*                |
                          |
                          |
                         ---
                         | |
                 |       ---
                ---       |
                | |             |
                | |            ---
                | |            |:|
                | |            |:|
                | |            |:|
                | |            |:|
                | |            ---
                ---             |
                 |
        */

        bool is_pattern_matched =
            candles_data.IsBullish(at) &&
            candles_data.IsInvertedHammer(at + 1) &&
            candles_data.IsBearish(at + 2);

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                      Bearish Piercing
********************************************************************************************/
bool BearishPiercingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*
                       |
              |       ---
             ---      |:|
             | |      |:|
             | |      |:|
             | |      |:|
             | |      --- Closing below 50% of prev bullish candle
             | |       |
             ---
              |
        */

        bool is_pattern_matched = 
            candles_data.IsBullish(at) &&
            candles_data.IsBearish(at + 1) &&
            candles_data.m_closes[at + 1] < (candles_data.m_closes[at] + candles_data.m_opens[at]) / 2.0;

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                      Bearish Engulfing
********************************************************************************************/
bool BearishEngulfingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*         
                                |
                               ---  
                      |        |:|
                     ---       |:|
                     | |       |:|
                     | |       |:|
                     ---       |:|
                      |        |:|
                               ---  
                                |

        */
        bool is_pattern_matched = 
            candles_data.IsBullish(at) && 
            candles_data.IsBearish(at + 1) &&
            candles_data.m_opens[at + 1] > candles_data.m_closes[at] &&    // curr open is above prev close
            candles_data.m_closes[at + 1] < candles_data.m_opens[at];       // curr close is below prev open

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************************
*                                     Bearish Hanging Man
********************************************************************************************/
bool BearishHangingManPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 1)
    {
        /*
                  Confirmation
                       |
                      ---
              ---     |:|
              |.|     ---
              ---      |
               |
               |
               |
               |
        */

        bool is_pattern_matched =
            candles_data.IsHammer(at) &&
            candles_data.IsBearish(at + 1);

        if (is_pattern_matched)
        {
            return true;
        }
    }

    return false;
}

#pragma endregion



/*******************************************************************************************
*                               Dragon Fly Doji
********************************************************************************************/

bool DragonFlyDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    return candles_data.IsDragonflyDoji(at);
}



/*******************************************************************************************
*                               Grave Stone Doji
********************************************************************************************/
bool GraveStonrDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    return candles_data.IsGravestoneDoji(at);
}


/*******************************************************************************************
*                               Long Leg Doji
********************************************************************************************/
bool LongLegDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    return candles_data.IsLongLegDoji(at);
}