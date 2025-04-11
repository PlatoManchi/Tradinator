#pragma once

#include "imgui.h"

class Utils
{
public:
	static void PushBodyFont();
	static void PushHeadingFont();
	static void PopFont();

	static ImFont* _BODY_FONT_;
	static ImFont* _HEADING_FONT_;

};

