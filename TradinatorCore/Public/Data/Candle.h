#pragma once

#include <string>
#include <vector>

struct Candle
{
public:
	std::string date;
	double open;
	double high;
	double low;
	double close;
	long volume;
	long open_interest;
};

struct Data
{
	std::vector<Candle> candles;
};

struct JsonTmp
{
	std::string status;
	Data data;
};