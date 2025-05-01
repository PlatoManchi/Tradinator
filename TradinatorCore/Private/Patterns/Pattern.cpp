#include "Patterns/Pattern.h"


std::vector<std::chrono::system_clock::time_point> BullishHaramiPattern::Check(CandleDataMapType::const_iterator current_candle_itr, CandleDataMapType::const_iterator begin, CandleDataMapType::const_iterator end)
{
    std::vector<std::chrono::system_clock::time_point> result;

    const Candle& curr = (*current_candle_itr).second;
    CandleDataMapType::const_iterator next_itr = std::next(current_candle_itr, 1);

    if (next_itr != end)
    {
        /*
                      |
                Open ---
                     |:|   |
                     |:|  ---  Close
                     |:|  | |
                     |:|  | |
                     |:|  ---  Open
                     |:|   |
               Close ---
                      |
        
        */

        const Candle& next = (*next_itr).second;
        bool is_pattern_matched = curr.m_close < curr.m_open && // curr is bearish
            next.m_close > next.m_open &&    // next is bullish    
            curr.m_open > next.m_close &&    // curr open is above next close
            curr.m_close < next.m_open;      // curr close is below next open

        if (is_pattern_matched)
        {
            result.push_back(curr.m_date);
            result.push_back(next.m_date);
        }
    }

    return result;
}


std::vector<std::chrono::system_clock::time_point> BullishHaramiCrossPattern::Check(CandleDataMapType::const_iterator current_candle_itr, CandleDataMapType::const_iterator begin, CandleDataMapType::const_iterator end)
{
    std::vector<std::chrono::system_clock::time_point> result;

    const Candle& curr = (*current_candle_itr).second;
    CandleDataMapType::const_iterator next_itr = std::next(current_candle_itr, 1);

    if (next_itr != end)
    {
        /*
                      |
                Open ---
                     |:|   
                     |:|   |
                     |:|  ---  Open, Close
                     |:|   |
                     |:|
                     |:|
               Close ---
                      |

        */

        const Candle& next = (*next_itr).second;
        if (next.IsDoji())
        {
            return BullishHaramiPattern::Check(current_candle_itr, begin, end);
        }
    }

    return result;
}