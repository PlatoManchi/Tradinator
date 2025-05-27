#pragma once

#include <chrono>

struct HilightsAnimationData
{
	float m_x_min = 0.0f;
	float m_x_max = 0.0f;
	ImVec4 m_color = ImVec4(0.4f, 0.4f, 0.4f, 0.0f);

	std::chrono::system_clock::time_point m_start_time;
};