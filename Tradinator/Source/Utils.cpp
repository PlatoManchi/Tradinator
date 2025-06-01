#include "Utils.h"

#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#endif

#include <fstream>
#include <cstdlib>
#include <filesystem>


#include "Indicators/SMA.h"
#include "Indicators/WMA.h"
#include "Indicators/EMA.h"
#include "Indicators/BollingerBand.h"
#include "Indicators/ROC.h"
#include "Indicators/RSI.h"
#include "Indicators/OBV.h"
#include "Indicators/MACD.h"
#include "Indicators/ATR.h"
#include "Indicators/TrendAnalysisDebug.h"
#include "Utils/Utils.h"

#include "Components/IndicatorWrappers.h"

namespace TradinatorAppSpace
{
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

	ImVec4 Utils::GetIndicatorColor(EIndicatorType type, size_t index)
	{
		assert(type != EIndicatorType::None && type != EIndicatorType::Max);

		switch (type)
		{
		case EIndicatorType::E_SMA:
			return ImVec4(0.01f, 0.49f, 1.0f, 1.0f);
		case EIndicatorType::E_WMA:
			return ImVec4(0.298f, 0.458f, 0.678f, 1.0f);
		case EIndicatorType::E_EMA:
			return ImVec4(0.4f, 1.0f, 0.2f, 1.0f);
		case EIndicatorType::E_BOLLINGER_BAND:
			return ImVec4(0.368f, 0.368f, 0.972f, 1.0f);
		case EIndicatorType::E_ROC:
			return ImVec4(1.0f, 0.501f, 0.0f, 1.0f);
		case EIndicatorType::E_RSI:
			return ImVec4(0.870f, 0.0f, 0.870f, 1.0f);
		case EIndicatorType::E_OBV:
			return ImVec4(0.298f, 0.686f, 0.314f, 1.0f);
		case EIndicatorType::E_MACD:
			if (index == 0)
			{
				return ImVec4(0.0f, 0.5f, 1.0f, 1.0f);
			}
			if (index == 1)
			{
				return ImVec4(0.0f, 0.607f, 0.0f, 1.0f);
			}
			if (index == 2)
			{
				return ImVec4(1.0f, 0.349f, 0.349f, 1.0f);
			}
			break;
		case EIndicatorType::E_ATR:
			return ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
		}

		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	

	std::unique_ptr<IIndicatorWrapper> Utils::GetIndicatorWrapper(EIndicatorType type)
	{
		std::unique_ptr<IIndicatorWrapper> wrapper = nullptr;
		switch (type)
		{
		case EIndicatorType::E_SMA:
		case EIndicatorType::E_WMA:
		case EIndicatorType::E_EMA:
			wrapper = std::move(std::make_unique<GenericIndicatorWrapper>());
			break;

		case EIndicatorType::E_ATR:
			wrapper = std::move(std::make_unique<GenericChartIndicatorWrapper>());
			break;
		case EIndicatorType::E_ROC:
			wrapper = std::move(std::make_unique<ROCIndicatorWrapper>());
			break; 
		case EIndicatorType::E_RSI:
			wrapper = std::move(std::make_unique<RSIIndicatorWrapper>());
			break;
		case EIndicatorType::E_BOLLINGER_BAND:
			wrapper = std::move(std::make_unique<BollingerBandIndicatorWrapper>());
			break;
		case EIndicatorType::E_OBV:
			wrapper = std::move(std::make_unique<OBVIndicatorWrapper>());
			break;
		case EIndicatorType::E_MACD:
			wrapper = std::move(std::make_unique<MACDIndicatorWrapper>());
			break;
		case EIndicatorType::E_TrendAnalysisDebug:
			wrapper = std::move(std::make_unique<TrendAnalysisDebugWrapper>());
		}

		if (wrapper)
		{
			wrapper->SetIndicator(TradinatorCoreSpace::Utils::GetIndicator(type));
		}

		return wrapper;
	}

	std::unique_ptr<IIndicatorWrapper> Utils::GetIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
	{
		std::unique_ptr<IIndicatorWrapper> wrapper = std::move(GetIndicatorWrapper(indicator->IndicatorType()));
		if (wrapper)
		{
			wrapper->SetIndicator(std::move(indicator));
			wrapper->SetSecurity(security);


			return wrapper;
		}
		
		return nullptr;
	}


	ENatureType Utils::GetPatternNatureType(EPattern pattern)
	{
		if ((pattern & Bullish_Pattern_Type) != EPattern::None)
		{
			return ENatureType::BULL;
		}
		else if ((pattern & Bearish_Pattern_Type) != EPattern::None)
		{
			return ENatureType::BEAR;
		}

		return ENatureType::NONE;
	}

	ENatureType Utils::GetStrategyNatureType(EStrategy strategy)
	{
		switch (strategy)
		{
		case EStrategy::Long_Strategy_1:
			return ENatureType::BULL;
		case EStrategy::Short_Strategy_1:
			return ENatureType::BEAR;
		}


		return ENatureType::NONE;
	}

	void Utils::OpenURL(const std::string& url)
	{
#if defined(_WIN32)
		ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
		std::string command = "open ";
		command += url;
		system(command.c_str());
#elif defined(__linux__)
		std::string command = "xdg-open ";
		command += url;
		system(command.c_str());
#endif
	}

	bool Utils::DoesDirectoryExist(const std::string& path_string) {
		std::filesystem::path path_to_check(path_string);
		return std::filesystem::is_directory(path_to_check);
	}
}

