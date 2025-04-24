#include "Utils.h"

#include <fstream>

#include "Indicators/SMA.h"
#include "Indicators/WMA.h"
#include "Indicators/EMA.h"

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

	ImVec4 Utils::GetIndicatorColor(EIndicatorType type)
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
			break;
		case E_ROC:
			break;
		case E_RSI:
			break;
		case E_MACD:
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
		else if (type_str == "MACD")
			return EIndicatorType::E_MACD;

		EIndicatorType::MAX;
	}

	std::shared_ptr<Indicator> Utils::GetIndicator(EIndicatorType type)
	{
		switch (type)
		{
		case E_SMA:
			return std::make_shared<SMA>();
		case E_WMA:
			return std::make_shared<WMA>();
		case E_EMA:
			return std::make_shared<EMA>();
		case E_BOLLINGER_BAND:
			break;
		case E_ROC:
			break;
		case E_RSI:
			break;
		case E_MACD:
			break;
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
