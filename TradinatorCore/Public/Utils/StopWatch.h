#pragma once

#include <chrono>
#include <string>

class StopWatch final
{
public:
	StopWatch(std::string name);
	~StopWatch();

	void PrintStatus() const;
	std::chrono::nanoseconds GetElapsed() const;

private:
	std::string m_name;
	std::chrono::steady_clock::time_point m_start;
};

