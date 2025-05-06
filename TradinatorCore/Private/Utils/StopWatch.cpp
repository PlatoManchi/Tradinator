#include "Utils/StopWatch.h"

#include <iostream>

StopWatch::StopWatch(std::string name)
	: m_name(name)
{
	m_start = std::chrono::high_resolution_clock::now();
}

StopWatch::~StopWatch()
{
	std::chrono::nanoseconds elapsed = GetElapsed();

	//std::cout.imbue(std::locale(""));
	//std::cout << std::fixed << std::showpoint << std::setprecision(3);
	struct separated : std::numpunct<char>
	{
		char do_thousands_sep() const { return ','; }
		std::string do_grouping() const { return "\03"; }
	};

	std::locale our_local(std::cout.getloc(), new separated);
	std::cout.imbue(our_local);
	std::cout << m_name << " Took : " << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() << " nanosec" << std::endl;
}

void StopWatch::PrintStatus() const
{
	std::chrono::nanoseconds elapsed = GetElapsed();

	//std::cout.imbue(std::locale(""));
	//std::cout << std::fixed << std::showpoint << std::setprecision(3);
	struct separated : std::numpunct<char>
	{
		char do_thousands_sep() const { return ','; }
		std::string do_grouping() const { return "\03"; }
	};

	std::locale our_local(std::cout.getloc(), new separated);
	std::cout.imbue(our_local);
	std::cout << m_name << " Elapsed : " << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() << " nanosec" << std::endl;
}

std::chrono::nanoseconds StopWatch::GetElapsed() const
{
	return std::chrono::high_resolution_clock::now() - m_start;
}