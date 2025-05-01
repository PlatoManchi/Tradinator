#pragma once

#include <memory>

#include "imgui.h"

#include "json/json.h"

#include "Indicators/Indicator.h"

class Indicator;
class Counter;
class IIndicatorWrapper;


namespace TradinatorAppSpace
{
	class Utils
	{
	public:
		static void PushBodyFont();
		static void PushHeadingFont();
		static void PopFont();
		static ImVec4 GetIndicatorColor(EIndicatorType type, size_t index = 0);
		static std::string GetIndicatorTypeStr(EIndicatorType type);
		static EIndicatorType GetIndicatorType(std::string type_str);
		static std::unique_ptr<Indicator> GetIndicator(EIndicatorType type);
		static std::unique_ptr<IIndicatorWrapper> GetIndicatorWrapper(EIndicatorType type);
		static std::unique_ptr<IIndicatorWrapper> GetIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Counter> counter = nullptr);


		static void OpenURL(const std::string& url);

		static bool DoesDirectoryExist(const std::string& path_string);

		static ImFont* _BODY_FONT_;
		static ImFont* _HEADING_FONT_;

	};
}
