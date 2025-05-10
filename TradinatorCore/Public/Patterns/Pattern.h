#pragma once

#include <cstdint>
#include <type_traits>
#include <bitset>

#include "Data/Candle.h"



/*enum class EPatternType : uint64_t {
    None = 0,
    Bullish_Harami = 1 << 0,
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
}*/

constexpr size_t _NUM_OF_PATTERNS_ = 64;
typedef std::bitset<_NUM_OF_PATTERNS_> EPatternType;

struct EPatternTypeComparator {
    constexpr bool operator() (const EPatternType& b1, const EPatternType& b2) const {
        return b1.to_string() < b2.to_string();
    }
};

constexpr EPatternType Bullish_Harami(1ULL << 0);
constexpr EPatternType Bullish_Harami_Cross(1ULL << 1);



const EPatternType Bullish_Pattern_Type = Bullish_Harami | Bullish_Harami_Cross;
const EPatternType Bearish_Pattern_Type;

class Pattern
{
public:
    Pattern();

    virtual std::string Name() const = 0;
    virtual EPatternType PatternType() const = 0;
    virtual bool IsDefaultVisible() const { return false; }

    /*
    * Returns the date range that satisfies the pattern.
    * Arranged in descending order. 
    * Latest date to oldest date
    */
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) = 0;
};

class BullishHaramiPattern : public Pattern
{
public:
    virtual std::string Name() const override { return "Bullish Harami"; }
    virtual EPatternType PatternType() const override { return Bullish_Harami; }
    
    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};

class BullishHaramiCrossPattern : public BullishHaramiPattern
{
public:
    virtual std::string Name() const override { return "Bullish Harami Cross"; }
    virtual EPatternType PatternType() const override { return Bullish_Harami_Cross; }
    virtual bool IsDefaultVisible() const { return true; }

    virtual std::vector<uint64_t> Check(uint64_t at, const CandlesData& candles_data) override;
};