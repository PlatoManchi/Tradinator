#include "Utils.h"


ImFont* Utils::_BODY_FONT_ = nullptr;
ImFont* Utils::_HEADING_FONT_ = nullptr;

void Utils::PushBodyFont()
{
	if (_BODY_FONT_)
	{
		ImGui::PushFont(_BODY_FONT_);
	}
}

void Utils::PushHeadingFont()
{
	if (_HEADING_FONT_)
	{
		ImGui::PushFont(_HEADING_FONT_);
	}
}

void Utils::PopFont()
{
	ImGui::PopFont();
}