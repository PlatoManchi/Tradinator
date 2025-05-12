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

       ---
       | |
       | |     -> Marubozu candle
       | |
       ---
*/


Pattern::Pattern()
{
    
}

#pragma region BullishPatterns
/*******************************************************************************************
*                                     Bullish Long Legged Doji
********************************************************************************************/
std::vector<uint64_t> BullishLongLeggedDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at + 1 };
        }
    }

    return {};
}

/*******************************************************************************************
*                                     Bullish Tri Star
********************************************************************************************/
std::vector<uint64_t> BullishTriStarPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                     Bullish Abandoned Baby
********************************************************************************************/
std::vector<uint64_t> BullishAbandonedBabyPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bullish Morning Star
********************************************************************************************/
std::vector<uint64_t> BullishMorningStarPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }

    return {};
}

/*******************************************************************************************
*                                  Bullish Morning Star Doji
********************************************************************************************/
std::vector<uint64_t> BullishMorningStarDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
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

    return {};
}

/*******************************************************************************************
*                                  Bullish Grave Stone Doji
********************************************************************************************/
std::vector<uint64_t> BullishGraveStoneDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                     Bullish Harami
********************************************************************************************/
std::vector<uint64_t> BullishHaramiPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
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

    return result;
}



/*******************************************************************************************
*                               Bullish Three Outside Up
********************************************************************************************/
std::vector<uint64_t> BullishThreeOutsideUpPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bullish Three Inside Up
********************************************************************************************/
std::vector<uint64_t> BullishThreeInsideUpPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }
    return{};
}

/*******************************************************************************************
*                               Bullish Matching Low
********************************************************************************************/
std::vector<uint64_t> BullishMatchingLowPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bullish Kicking
********************************************************************************************/
std::vector<uint64_t> BullishKickingPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bullish Three White Soldiers
********************************************************************************************/
std::vector<uint64_t> BullishThreeWhiteSoldiersPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }
    return {};
}


/*******************************************************************************************
*                               Bullish Meeting Lines
********************************************************************************************/
std::vector<uint64_t> BullishMeetingLinesPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}

/*******************************************************************************************
*                               Bullish Inverted Hammer
********************************************************************************************/
std::vector<uint64_t> BullishInvertedHammerPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }
    return {};
}

/*******************************************************************************************
*                                     Bullish Piercing
********************************************************************************************/
std::vector<uint64_t> BullishPiercingPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}

/*******************************************************************************************
*                                     Bullish Engulfing
********************************************************************************************/
std::vector<uint64_t> BullishEngulfingPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}

/*******************************************************************************************
*                                     Bullish Hammer
********************************************************************************************/
std::vector<uint64_t> BullishHammerPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at };
        }
    }

    return {};
}

#pragma endregion



#pragma region BearishPatterns
/*******************************************************************************************
*                                 Bearish Long Legged Doji
********************************************************************************************/
std::vector<uint64_t> BearishLongLeggedDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at + 1 };
        }
    }
    return {};
}


/*******************************************************************************************
*                                 Bearish Tri Star
********************************************************************************************/
std::vector<uint64_t> BearishTriStarPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                 Bearish Abandoned Baby
********************************************************************************************/
std::vector<uint64_t> BearishAbandonedBabyPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bearish Evening Star
********************************************************************************************/
std::vector<uint64_t> BearishEveningStarPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }
    return {};
}

/*******************************************************************************************
*                                 Bearish Evening Star Doji
********************************************************************************************/
std::vector<uint64_t> BearishEveningStarDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
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
    return {};
}


/*******************************************************************************************
*                                 Bearish Grave Stone Doji
********************************************************************************************/
std::vector<uint64_t> BearishGraveStoneDojiPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            candles_data.IsBullish(at + 1) &&
            candles_data.IsGravestoneDoji(at + 1) &&
            candles_data.IsBearish(at + 2);

        if (is_pattern_matched)
        {
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                 Bearish Harami
********************************************************************************************/
std::vector<uint64_t> BearishHaramiPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }
    return {};
}


/*******************************************************************************************
*                                 Bearish Harami Cross
********************************************************************************************/
std::vector<uint64_t> BearishHaramiCrossPattern::Check(uint64_t at, const CandlesData& candles_data)
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
    return {};
}


/*******************************************************************************************
*                              Bearish Three Outside Down
********************************************************************************************/
std::vector<uint64_t> BearishThreeOutsideDownPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }
    return {};
}


/*******************************************************************************************
*                              Bearish Three Inside Down
********************************************************************************************/
std::vector<uint64_t> BearishThreeInsideDownPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1, at + 2 };
        }
    }
    return {};
}


/*******************************************************************************************
*                              Bearish Matching High
********************************************************************************************/
std::vector<uint64_t> BearishMatchingHighPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                  Bearish Kicking
********************************************************************************************/
std::vector<uint64_t> BearishKickingPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                               Bearish Three Black Crow
********************************************************************************************/
std::vector<uint64_t> BearishThreeBlackCrowPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    if (at >= 0 && at < candles_data.m_dates.size() - 2)
    {
        /*                      |
                               ---
                               |:|
                         |     |:|
                        ---    |:|
                        |:|    |:|
                 |      |:|    ---
                ---     |:|     |
                |:|     |:|
                |:|     ---
                |:|      |
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
            candles_data.m_closes[at + 1] > candles_data.m_closes[at] &&
            candles_data.m_opens[at + 1] > candles_data.m_opens[at] &&
            candles_data.m_closes[at + 2] > candles_data.m_closes[at + 1] &&
            candles_data.m_opens[at + 2] > candles_data.m_opens[at + 1];

        if (is_pattern_matched)
        {
            return { at, at + 1, at + 2 };
        }
    }
    return {};
}


/*******************************************************************************************
*                                   Bearish Meeting Line
********************************************************************************************/
std::vector<uint64_t> BearishMeetingLinePattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                   Bearish Shooting Star
********************************************************************************************/
std::vector<uint64_t> BearishShootingStarPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }
    return {};
}


/*******************************************************************************************
*                                      Bearish Piercing
********************************************************************************************/
std::vector<uint64_t> BearishPiercingPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                      Bearish Engulfing
********************************************************************************************/
std::vector<uint64_t> BearishEngulfingPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at, at + 1 };
        }
    }

    return {};
}


/*******************************************************************************************
*                                     Bearish Hanging Man
********************************************************************************************/
std::vector<uint64_t> BearishHangingManPattern::Check(uint64_t at, const CandlesData& candles_data)
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
            return { at };
        }
    }

    return {};
}

#pragma endregion
