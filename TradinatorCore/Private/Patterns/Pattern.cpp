#include "Patterns/Pattern.h"

#include <cassert>

Pattern::Pattern()
{
    // in C++ 23, any is made as constexpr so can just check at compile time by using static_assert.
    // But since I'm compiling using C++20 this is a way to check atleast at compile time

    // Make sure that all the flags are unique
    assert((Bullish_Harami
        & Bullish_Harami_Cross).any());
}


std::vector<std::chrono::system_clock::time_point> BullishHaramiPattern::Check(CandleDataMapType::const_iterator current_candle_itr, CandleDataMapType::const_iterator begin, CandleDataMapType::const_iterator end)
{
    std::vector<std::chrono::system_clock::time_point> result;

    const Candle& curr = (*current_candle_itr).second;
    if (current_candle_itr != begin)
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


        // Remember the data is arranged in descending order of dates.
        // Latest data is at 0 and previous day is at 1 and on
        // so next day will be previous data in terms of iteration.
        CandleDataMapType::const_iterator next_itr = std::prev(current_candle_itr, 1);

        const Candle& next = (*next_itr).second;
        bool is_pattern_matched = curr.m_close < curr.m_open && // curr is bearish
            next.m_close > next.m_open &&    // next is bullish    
            curr.m_open > next.m_close &&    // curr open is above next close
            curr.m_close < next.m_open;      // curr close is below next open

        if (is_pattern_matched)
        {
            result.push_back(next.m_date);
            result.push_back(curr.m_date);
        }
    }

    return result;
}


std::vector<std::chrono::system_clock::time_point> BullishHaramiCrossPattern::Check(CandleDataMapType::const_iterator current_candle_itr, CandleDataMapType::const_iterator begin, CandleDataMapType::const_iterator end)
{
    std::vector<std::chrono::system_clock::time_point> result;

    const Candle& curr = (*current_candle_itr).second;
    if (current_candle_itr != begin)
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


        // Remember the data is arranged in descending order of dates.
        // Latest data is at 0 and previous day is at 1 and on
        // so next day will be previous data in terms of iteration.
        CandleDataMapType::const_iterator next_itr = std::prev(current_candle_itr, 1);

        const Candle& next = (*next_itr).second;
        if (next.IsDoji())
        {
            return BullishHaramiPattern::Check(current_candle_itr, begin, end);
        }
    }

    return result;
}