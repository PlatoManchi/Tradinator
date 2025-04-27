#include "Utils.h"

#include <fstream>

#include "Indicators/SMA.h"
#include "Indicators/WMA.h"
#include "Indicators/EMA.h"
#include "Indicators/BollingerBand.h"
#include "Indicators/ROC.h"
#include "Indicators/RSI.h"
#include "Indicators/OBV.h"

#include "Components/IndicatorWrappers.h"

namespace TradinatorAppSpace
{
	ImFont* Utils::_BODY_FONT_ = nullptr;
	ImFont* Utils::_HEADING_FONT_ = nullptr;
	std::string Utils::_SETTINGS_FILE_ = "Settings.json";

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
		assert(type != EIndicatorType::MIN && type != EIndicatorType::MAX);

		switch (type)
		{
		case E_SMA:
			return ImVec4(0.01f, 0.49f, 1.0f, 1.0f);
		case E_WMA:
			return ImVec4(0.164f, 0.223f, 0.305f, 1.0f);
		case E_EMA:
			return ImVec4(0.4f, 1.0f, 0.2f, 1.0f);
		case E_BOLLINGER_BAND:
			return ImVec4(0.368f, 0.368f, 0.972f, 1.0f);
		case E_ROC:
			return ImVec4(1.0f, 0.501f, 0.0f, 1.0f);
		case E_RSI:
			return ImVec4(0.5f, 0.0f, 0.5f, 1.0f);
		case E_OBV:
			return ImVec4(0.298f, 0.686f, 0.314f, 1.0f);
		case E_MACD:
			if (index == 0)
			{
				return ImVec4(0.0f, 0.5f, 1.0f, 1.0f);
			}
			if (index == 1)
			{
				return ImVec4(0.0f, 0.607f, 0.0f, 1.0f);
			}
			if (index == 3)
			{
				return ImVec4(1.0f, 0.349f, 0.349f, 1.0f);
			}
			break;
		}

		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	std::string Utils::GetIndicatorTypeStr(EIndicatorType type)
	{
		switch (type)
		{
		case E_SMA:
			return "SMA";
		case E_WMA:
			return "WMA";
		case E_EMA:
			return "EMA";
		case E_BOLLINGER_BAND:
			return "Boolinger Band";
		case E_ROC:
			return "ROC";
		case E_RSI:
			return "RSI";
		case E_OBV:
			return "OBV";
		case E_MACD:
			return "MACD";
		}

		return "";
	}

	EIndicatorType Utils::GetIndicatorType(std::string type_str)
	{
		if (type_str == "SMA")
			return EIndicatorType::E_SMA;
		else if (type_str == "WMA")
			return EIndicatorType::E_WMA;
		else if (type_str == "EMA")
			return EIndicatorType::E_EMA;
		else if (type_str == "Boolinger Band")
			return EIndicatorType::E_BOLLINGER_BAND;
		else if (type_str == "ROC")
			return EIndicatorType::E_ROC;
		else if (type_str == "RSI")
			return EIndicatorType::E_RSI;
		else if(type_str == "OBV")
			return EIndicatorType::E_OBV;
		else if (type_str == "MACD")
			return EIndicatorType::E_MACD;

		return EIndicatorType::MAX;
	}

	std::unique_ptr<Indicator> Utils::GetIndicator(EIndicatorType type)
	{
		switch (type)
		{
		case E_SMA:
			return std::make_unique<SMA>();
		case E_WMA:
			return std::make_unique<WMA>();
		case E_EMA:
			return std::make_unique<EMA>();
		case E_BOLLINGER_BAND:
			return std::make_unique<BollingerBand>();
		case E_ROC:
			return std::make_unique<ROC>();
		case E_RSI:
			return std::make_unique<RSI>();
		case E_OBV:
			return std::make_unique<OBV>();
		case E_MACD:
			break;
		}

		return nullptr;
	}

	std::unique_ptr<IIndicatorWrapper> Utils::GetIndicatorWrapper(EIndicatorType type)
	{
		std::unique_ptr<IIndicatorWrapper> wrapper = nullptr;
		switch (type)
		{
		case E_SMA:
		case E_WMA:
		case E_EMA:
		case E_MACD:
			wrapper = std::move(std::make_unique<GenericIndicatorWrapper>());
			break;
		case E_ROC:
			wrapper = std::move(std::make_unique<ROCIndicatorWrapper>());
			break;
		case E_RSI:
			wrapper = std::move(std::make_unique<RSIIndicatorWrapper>());
			break;
		case E_BOLLINGER_BAND:
			wrapper = std::move(std::make_unique<BollingerBandIndicatorWrapper>());
			break;
		case E_OBV:
			wrapper = std::move(std::make_unique<OBVIndicatorWrapper>());
			break;
		}

		if (wrapper)
		{
			wrapper->SetIndicator(GetIndicator(type));
		}

		return wrapper;
	}

	std::unique_ptr<IIndicatorWrapper> Utils::GetIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Counter> counter)
	{
		std::unique_ptr<IIndicatorWrapper> wrapper = std::move(GetIndicatorWrapper(indicator->IndicatorType()));
		if (wrapper)
		{
			wrapper->SetIndicator(std::move(indicator));
			wrapper->SetCounter(counter);


			return wrapper;
		}
		
		return nullptr;
	}


	bool Utils::IsIndicatorOverlayable(EIndicatorType type)
	{
		return type == EIndicatorType::E_SMA ||
			type == EIndicatorType::E_WMA ||
			type == EIndicatorType::E_EMA ||
			type == EIndicatorType::E_BOLLINGER_BAND;
	}

	bool Utils::IsIndicatorEnvelopeType(EIndicatorType type)
	{
		return type == EIndicatorType::E_BOLLINGER_BAND;
	}

	void Utils::SaveWindowsStatus(Json::Value status)
	{
		std::ofstream output_file(_SETTINGS_FILE_);

		output_file << status;
		output_file.close();
	}

	Json::Value Utils::LoadWindowsStatus()
	{
		Json::Value result;

		std::ifstream input_file(_SETTINGS_FILE_, std::ifstream::binary);
		if (input_file.good())
		{
			input_file >> result;
			input_file.close();
		}
		
		return result;
	}
}
