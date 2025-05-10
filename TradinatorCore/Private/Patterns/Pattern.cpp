#include "Patterns/Pattern.h"

#include <cassert>


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
*/


Pattern::Pattern()
{
    
}

/*******************************************************************************************
*                                     Bullish Long Legged Doji
********************************************************************************************/
std::vector<uint64_t> BullishLongLeggedDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 1 && at < candles_data.m_dates.size() - 1)
        //&& at < candles_data.m_trends.size())
    {
        /*          Prev       Curr                  Next
                      |
                Open ---                               |
                     |:|                              ---
                     |:|        |                     | |
                     |:|       ---  Open, Close       | |
                     |:|        |                     | |
                     |:|                              ---
                     |:|                               |
               Close ---
                      |
        */
        bool is_pattern_matched =
            //candles_data.m_trends[at] == ETrend::Down && // currently trend detection isn't stable
            candles_data.IsBearish(at - 1) &&           // Prev is bear
            candles_data.IsLongLegDoji(at) &&
            candles_data.IsBullish(at + 1) && candles_data.m_closes[at + 1] > candles_data.m_highs[at]; // Confirmation with next candle

        if (is_pattern_matched)
        {
            return { at };
        }
    }

    return {};
}

/*******************************************************************************************
*                                     Bullish Tri Star
********************************************************************************************/
std::vector<uint64_t> BullishTriStarPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 2 && at < candles_data.m_dates.size())
        //&& at < candles_data.m_trends.size())
    {
        /*
                        |
                       ---                            |
                        |                            ---
                                                      |
                                     |
                                    ---
                                     |
        */
        bool is_pattern_matched =
            //candles_data.m_trends[at] == ETrend::Down && // currently trend detection isn't stable
            candles_data.IsDoji(at) &&
            candles_data.IsDoji(at - 1) &&
            candles_data.IsDoji(at - 2) &&
            candles_data.m_highs[at - 1] < candles_data.m_lows[at - 2] &&  // If middle doji body is below prev and curr
            candles_data.m_highs[at - 1] < candles_data.m_lows[at];

        if (is_pattern_matched)
        {
            return { at - 2, at - 1, at };
        }
    }

    return {};
}


/*******************************************************************************************
*                                     Bullish Abandoned Baby
********************************************************************************************/
std::vector<uint64_t> BullishAbandonedBabyPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 2 && at < candles_data.m_dates.size())
    {
        /*
                 |
                ---
                |:|                 |
                |:|                ---
                |:|                | |
                |:|                | |
                |:|                | |
                ---                | |
                 |                 ---
                                    |
                           |
                          ---
                           |
        */
        bool is_pattern_matched =
            candles_data.IsDoji(at - 1) &&          // Middle is doji
            candles_data.IsBearish(at - 2) &&       // first candle is bearish
            candles_data.IsBullish(at) &&           // third candle is bullish
            candles_data.IsLongCandle(at - 2) &&    // Strong bearish first candle
            candles_data.IsLongCandle(at) &&        // Strong bullish third candle
            candles_data.m_highs[at - 1] < candles_data.m_lows[at - 2] && // gap down on from middle
            candles_data.m_highs[at - 1] < candles_data.m_lows[at] &&     // gap down on from middle
            candles_data.m_closes[at] > (candles_data.m_opens[at - 2] + candles_data.m_closes[at - 2]) / 2.0; // Current is above half of first candle

        if (is_pattern_matched)
        {
            return { at - 2, at - 1, at };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bullish Morning Star
********************************************************************************************/
std::vector<uint64_t> BullishMorningStarPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 2 && at < candles_data.m_dates.size())
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
                          |.|
                          ---
                           |
        */

        double middle_top = candles_data.IsBullish(at - 1) ? candles_data.m_closes[at - 1] : candles_data.m_opens[at - 1];
        bool is_pattern_matched =
            candles_data.IsBearish(at - 2) &&       // first candle is bearish
            candles_data.IsBullish(at) &&           // third candle is bullish
            candles_data.IsLongCandle(at - 2) &&    // Strong bearish first candle
            candles_data.IsLongCandle(at) &&        // Strong bullish third candle
            candles_data.m_highs[at - 1] > candles_data.m_lows[at - 2] && // first candle overlapping shadow with middle candle
            candles_data.m_opens[at] > middle_top &&       // curr body is above middle body 
            candles_data.m_closes[at] > (candles_data.m_opens[at - 2] + candles_data.m_closes[at - 2]) / 2.0; // Current is above half of first candle

        if (is_pattern_matched)
        {
            return { at - 2, at - 1, at };
        }
    }
    return {};
}

/*******************************************************************************************
*                                  Bullish Morning Star Doji
********************************************************************************************/
std::vector<uint64_t> BullishMorningStarDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 1 && at < candles_data.m_dates.size())
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
        if (candles_data.IsDoji(at - 1))
        {
            return BullishMorningStarPattern::Check(at, candles_data);
        }
    }

    return {};
}

/*******************************************************************************************
*                                  Bullish Grave Stone Doji
********************************************************************************************/
std::vector<uint64_t> BullishGraveStoneDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 1 && at < candles_data.m_dates.size() - 1)
    {
        /*
                 |
                ---
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
            candles_data.IsBearish(at - 1) &&
            candles_data.IsGravestoneDoji(at) &&
            candles_data.IsBullish(at + 1);

        if (is_pattern_matched)
        {
            return { at - 1, at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                     Bullish Harami
********************************************************************************************/
std::vector<uint64_t> BullishHaramiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at > 0 && at < candles_data.m_dates.size())
    {
        /*          Prev      Curr
                      |
                Open ---
                     |:|        |
                     |:|       ---  Close
                     |:|       | |
                     |:|       | |
                     |:|       ---  Open
                     |:|        |
               Close ---
                      |

        */

        bool is_pattern_matched = candles_data.IsBearish(at - 1) &&       // prev is bearish
            candles_data.IsBullish(at) &&                                 // curr is bullish    
            candles_data.m_opens[at - 1] > candles_data.m_closes[at] &&   // prev open is above prev close
            candles_data.m_closes[at - 1] < candles_data.m_opens[at];     // prev close is below curropen

        if (is_pattern_matched)
        {
            return { at - 1, at };
        }
    }

    return {};
}



/*******************************************************************************************
*                               Bullish Harami Cross
********************************************************************************************/
std::vector<uint64_t> BullishHaramiCrossPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    std::vector<uint64_t> result;

    if (at > 0 && at < candles_data.m_dates.size())
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


        if (candles_data.IsDoji(at))
        {
            return BullishHaramiPattern::Check(at, candles_data);
        }
    }

    return result;
}



/*******************************************************************************************
*                               Bullish Three Outside Up
********************************************************************************************/
std::vector<uint64_t> BullishThreeOutsideUpPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 2 && at < candles_data.m_dates.size())
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
            candles_data.IsBearish(at - 2) &&
            candles_data.IsBullish(at - 1) &&
            candles_data.IsBullish(at) &&
            candles_data.m_opens[at - 1] < candles_data.m_closes[at - 2] &&
            candles_data.m_closes[at - 1] > candles_data.m_opens[at - 2] &&
            candles_data.m_closes[at] > candles_data.m_closes[at - 1];
        
        if (is_pattern_matched)
        {
            return { at - 1, at - 1, at };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bullish Three Inside Up
********************************************************************************************/
std::vector<uint64_t> BullishThreeInsideUpPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 2 && at < candles_data.m_dates.size())
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
            candles_data.IsBearish(at - 2) &&
            candles_data.IsLongCandle(at - 2) &&
            candles_data.IsBullish(at - 1) &&
            candles_data.IsBullish(at) &&
            candles_data.m_opens[at - 1] > candles_data.m_closes[at - 2] &&
            candles_data.m_closes[at - 1] < candles_data.m_opens[at - 2] &&
            candles_data.m_closes[at] > candles_data.m_opens[at - 2];

        if (is_pattern_matched)
        {
            return { at - 2, at - 1, at };
        }
    }
    return{};
}

/*******************************************************************************************
*                               Bullish Matching Low
********************************************************************************************/
std::vector<uint64_t> BullishMatchingLowPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 1 && at < candles_data.m_dates.size())
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
        double longest_body = std::max(fabs(candles_data.m_closes[at - 1] - candles_data.m_opens[at - 1]), fabs(candles_data.m_closes[at] - candles_data.m_opens[at]));
        double small_threshold = longest_body * 0.01; // 1% threshold

        bool is_pattern_matched = 
            candles_data.IsBearish(at - 1) &&
            candles_data.IsBearish(at) &&
            fabs(candles_data.m_closes[at - 1] - candles_data.m_closes[at - 1]) <= small_threshold;
        
        if (is_pattern_matched)
        {
            return { at - 1, at };
        }
    }
    return {};
}


/*******************************************************************************************
*                               Bullish Kicking
********************************************************************************************/
std::vector<uint64_t> BullishKickingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 1 && at < candles_data.m_dates.size())
    {
        bool is_pattern_matched = 
            candles_data.IsBearish(at - 1) &&
            candles_data.IsMarubozu(at - 1) &&
            candles_data.IsBullish(at) &&
            candles_data.IsMarubozu(at) &&
            candles_data.m_opens[at - 1] > candles_data.m_highs[at];

        if (is_pattern_matched)
        {
            return { at - 1, at };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bullish Three White Soldiers
********************************************************************************************/
std::vector<uint64_t> BullishThreeWhiteSoldiersPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 2 && at < candles_data.m_dates.size())
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
            candles_data.IsBullish(at - 2) &&
            candles_data.IsLongCandle(at - 2) &&
            candles_data.IsBullish(at - 1) &&
            candles_data.IsLongCandle(at - 1) &&
            candles_data.IsBullish(at) &&
            candles_data.IsLongCandle(at) &&
            candles_data.m_opens[at - 1] > candles_data.m_opens[at - 2] &&
            candles_data.m_opens[at - 1] < candles_data.m_closes[at - 2] &&
            candles_data.m_opens[at] > candles_data.m_opens[at - 1] &&
            candles_data.m_opens[at] < candles_data.m_closes[at - 1];

        if (is_pattern_matched)
        {
            return { at - 2, at - 1, at };
        }
    }
    return {};
}


/*******************************************************************************************
*                               Bullish Meeting Lines
********************************************************************************************/
std::vector<uint64_t> BullishMeetingLinesPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 1 && at < candles_data.m_dates.size() - 1)
    {
        /*
              |
             ---
             |:|
             |:|
             |:|
             |:|        |
             ---       ---
              |        | |
                       | |
                       | |
                       ---
                        |
        */
        double longest_body = std::max(fabs(candles_data.m_closes[at - 1] - candles_data.m_opens[at - 1]), fabs(candles_data.m_closes[at] - candles_data.m_opens[at]));
        double small_threshold = longest_body * 0.01; // 1% threshold

        bool is_pattern_matched =
            candles_data.IsBearish(at - 1) && candles_data.IsLongCandle(at - 1) &&
            candles_data.IsBullish(at) && candles_data.IsLongCandle(at) &&
            candles_data.m_opens[at] < candles_data.m_closes[at - 1] &&
            fabs(candles_data.m_closes[at] - candles_data.m_closes[at - 1]) < small_threshold &&
            candles_data.IsBullish(at + 1);    // Option: Confirms the pattern

        if (is_pattern_matched)
        {
            return { at - 1, at, at + 1 };
        }
    }

    return {};
}

/*******************************************************************************************
*                               Bullish Inverted Hammer
********************************************************************************************/
std::vector<uint64_t> BullishInvertedHammerPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 1 && at < candles_data.m_dates.size() - 1)
    {
        /*
               |
              ---
              |:|
              |:|        |
              |:|        |
              |:|        |
              |:|       ---
              ---       |.|
               |        ---
        */
        bool is_pattern_matched = 
            candles_data.IsBearish(at - 1) &&
            candles_data.IsInvertedHammer(at) &&
            candles_data.IsBullish(at + 1);    // Confirmation

        if (is_pattern_matched)
        {
            return { at - 1, at, at + 1 };
        }
    }
    return {};
}


/*******************************************************************************************
*                                     Bullish Engulfing
********************************************************************************************/
std::vector<uint64_t> BullishEngulfingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    std::vector<uint64_t> result;

    if (at > 0 && at < candles_data.m_dates.size())
    {
        /*         Prev      Current
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
        bool is_pattern_matched = candles_data.IsBullish(at) && candles_data.IsBearish(at - 1) &&
            candles_data.m_closes[at] > candles_data.m_opens[at - 1]  &&    // curr close is above prev open
            candles_data.m_opens[at] < candles_data.m_closes[at - 1];       // curr open is below prev close

        if (is_pattern_matched)
        {
            result.push_back(at - 1);
            result.push_back(at);
        }
    }

    return result;
}

/*******************************************************************************************
*                                     Bullish Piercing
********************************************************************************************/
std::vector<uint64_t> BullishPiercingPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    std::vector<uint64_t> result;

    if (at > 0 && at < candles_data.m_dates.size())
    {
        /*         Prev         Curr  
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

        double prev_candle_body_size = candles_data.m_opens[at - 1] - candles_data.m_closes[at - 1];
        double half_point = candles_data.m_closes[at - 1] + prev_candle_body_size * 0.5;

        bool is_pattern_matched = candles_data.IsBearish(at - 1) && candles_data.IsBullish(at)
            && candles_data.m_closes[at] > half_point;

        if (is_pattern_matched)
        {
            result.push_back(at - 1);
            result.push_back(at);
        }
    }

    return result;
}

/*******************************************************************************************
*                                     Bullish Hammer
********************************************************************************************/
std::vector<uint64_t> BullishHammerPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at > 0 && at < candles_data.m_dates.size() - 1)
    {
        /*
        
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
            return { at, at + 1 };
        }
    }
    return {};
}