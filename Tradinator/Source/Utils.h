#pragma once

#include <memory>

#include "imgui.h"

#include "json/json.h"

#include "TradinatorCoreTypes.h"
#include "Indicators/Indicator.h"
#include "Patterns/Pattern.h"
#include "Strategy/Strategy.h"

class Indicator;
class Security;
class IIndicatorWrapper;


namespace TradinatorAppSpace
{
	enum ENatureType
	{
		NONE,

		BULL,
		BEAR
	};

	class Utils
	{
	public:
		static void PushBodyFont();
		static void PushHeadingFont();
		static void PopFont();
		static ImVec4 GetIndicatorColor(EIndicatorType type, size_t index = 0);
		
		static std::unique_ptr<IIndicatorWrapper> GetIndicatorWrapper(EIndicatorType type);
		static std::unique_ptr<IIndicatorWrapper> GetIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security = nullptr);

		static ENatureType GetPatternNatureType(EPattern pattern);
		static ENatureType GetStrategyNatureType(EStrategy strategy);

		static void OpenURL(const std::string& url);

		static bool DoesDirectoryExist(const std::string& path_string);

		static ImFont* _BODY_FONT_;
		static ImFont* _HEADING_FONT_;

	};
}
