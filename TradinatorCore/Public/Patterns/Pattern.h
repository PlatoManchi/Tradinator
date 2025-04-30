#pragma once

#include <cstdint>
#include <type_traits>
#include <bitset>

#include "Data/Counter.h"

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

constexpr EPatternType Bullish_Harami(1ULL << 0);
constexpr EPatternType Bullish_Harami_Cross(1ULL << 1);

class Pattern
{
public:
        virtual std::string Name() const = 0;
        virtual EPatternType PatternType() const = 0;
        virtual std::vector<std::chrono::system_clock::time_point> Check(AsyncCandleData::const_iterator current_candle, AsyncCandleData::const_iterator begin, AsyncCandleData::const_iterator end) = 0;
};

class BullishHaramiPattern : public Pattern
{
public:
    virtual std::string Name() const override { return "Bullish Harami"; }
    virtual EPatternType PatternType() const override { return Bullish_Harami; }

    virtual std::vector<std::chrono::system_clock::time_point> Check(AsyncCandleData::const_iterator current_candle_itr, AsyncCandleData::const_iterator begin, AsyncCandleData::const_iterator end) override;
};

class BullishHaramiCrossPattern : public BullishHaramiPattern
{
public:
    virtual std::string Name() const override { return "Bullish Harami Cross"; }
    virtual EPatternType PatternType() const override { return Bullish_Harami_Cross; }

    virtual std::vector<std::chrono::system_clock::time_point> Check(AsyncCandleData::const_iterator current_candle_itr, AsyncCandleData::const_iterator begin, AsyncCandleData::const_iterator end) override;
};