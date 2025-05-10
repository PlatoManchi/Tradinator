#include "Patterns/Pattern.h"

#include <cassert>

Pattern::Pattern()
{
    
}


std::vector<uint64_t> BullishHaramiPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    std::vector<uint64_t> result;

    if (at < candles_data.m_dates.size() - 1)
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

        bool is_pattern_matched = candles_data.IsBearish(at) &&            // curr is bearish
            candles_data.IsBullish(at + 1) &&                              // next is bullish    
            candles_data.m_opens[at] > candles_data.m_closes[at + 1] &&    // curr open is above next close
            candles_data.m_closes[at] < candles_data.m_opens[at + 1];      // curr close is below next open

        if (is_pattern_matched)
        {
            result.push_back(at);
            result.push_back(at + 1);
        }
    }

    return result;
}


std::vector<uint64_t> BullishHaramiCrossPattern::Check(uint64_t at, const CandlesData& candles_data)
{
    std::vector<uint64_t> result;

    if (at < candles_data.m_dates.size() - 1)
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


        if (candles_data.IsDoji(at + 1))
        {
            return BullishHaramiPattern::Check(at, candles_data);
        }
    }

    return result;
}