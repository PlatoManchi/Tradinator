#pragma once

#include <memory>
#include <vector>

#include "json/json.h"
#include "imgui.h"
#include "implot.h"

#include "Indicators/Indicator.h"

class IIndicatorWrapper
{
public:
	IIndicatorWrapper();

	IIndicatorWrapper(std::unique_ptr<Indicator> indicator);
	IIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security);
	
	IIndicatorWrapper(const IIndicatorWrapper& other);
	IIndicatorWrapper& operator=(const IIndicatorWrapper& other);
	IIndicatorWrapper(IIndicatorWrapper&& other) noexcept = default;
	IIndicatorWrapper& operator=(IIndicatorWrapper&& other) noexcept = default;

	virtual void SetIndicator(std::unique_ptr<Indicator> indicator);
	virtual void SetSecurity(std::shared_ptr<Security> security);
	

	virtual bool DrawAsAvailableIndicator() = 0;
	virtual bool DrawAsAppliedIndicator() = 0;

	// Anything that should be drawn before candles are drawn
	virtual void PlotPreCandle(ImVec4 bull_color, ImVec4 bear_color) = 0;

	// Anything that should be drawn after candles are drawn
	virtual void PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color) = 0;
	virtual bool IsIndicatorOverlayable() = 0;

	virtual void Calculate() = 0;

	virtual std::unique_ptr<IIndicatorWrapper> Clone() = 0;

	virtual void FromJson(Json::Value value) = 0;
	virtual Json::Value ToJson() const = 0;

	virtual std::string GetHumanReadableValueAt(size_t index) const = 0;

	inline size_t GetID() const { return m_id; }
	inline EIndicatorType IndicatorType() const { return m_indicator->IndicatorType(); }
	inline bool IsSingleInstanceType() const { return m_indicator->IsSingleInstanceType(); }
	inline bool ShouldShow() const { return m_show; }
	inline std::shared_ptr<Security> GetSecurity() const { return m_security; }
	inline const std::vector<std::vector<double>>& GetPointsList() const { return m_points_list; }

	struct PlotPointGetterData
	{
		PlotPointGetterData() : m_indicator_wrapper(nullptr), m_points_index(-1) {}
		PlotPointGetterData(IIndicatorWrapper* wrapper, int32_t index) : m_indicator_wrapper(wrapper), m_points_index(index) {}

		IIndicatorWrapper* m_indicator_wrapper;
		int32_t m_points_index;
	};

protected:
	std::unique_ptr<Indicator> m_indicator;
	std::shared_ptr<Security> m_security;

	std::vector<std::vector<double>> m_points_list;
	std::vector<ImVec4> m_colors_list;

	bool m_show = true;
	bool m_is_hovered = false;
	size_t m_id = 0;

	static size_t _INCREMENTAL_WRAPPER_ID_;;
};

/*********************************************************************************
*                                Generic Wrapper
**********************************************************************************/

class GenericIndicatorWrapper : public IIndicatorWrapper
{
public:
	GenericIndicatorWrapper() : IIndicatorWrapper() {}
	GenericIndicatorWrapper(std::unique_ptr<Indicator> indicator)
		: IIndicatorWrapper(std::move(indicator)) { }

	GenericIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
		: IIndicatorWrapper(std::move(indicator), security) { }

	GenericIndicatorWrapper(const GenericIndicatorWrapper& other) = default;
	GenericIndicatorWrapper& operator=(const GenericIndicatorWrapper& other) = default;
	GenericIndicatorWrapper(GenericIndicatorWrapper&& other) noexcept = default;
	GenericIndicatorWrapper& operator=(GenericIndicatorWrapper&& other) noexcept = default;

	virtual bool DrawAsAvailableIndicator() override;
	virtual bool DrawAsAppliedIndicator() override;
	virtual void PlotPreCandle(ImVec4 bull_color, ImVec4 bear_color) override;
	virtual void PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color) override;
	virtual void Calculate() override;
	virtual bool IsIndicatorOverlayable() override { return true; }

	virtual std::unique_ptr<IIndicatorWrapper> Clone() override
	{
		return std::make_unique<GenericIndicatorWrapper>(*this);;
	}


	virtual void FromJson(Json::Value value) override;
	virtual Json::Value ToJson() const override;

	virtual std::string GetHumanReadableValueAt(size_t index) const override;
protected:
	
};

/*********************************************************************************
*                                Generic Chart Wrapper
**********************************************************************************/

class GenericChartIndicatorWrapper : public GenericIndicatorWrapper
{
public:
	GenericChartIndicatorWrapper() : GenericIndicatorWrapper() {}
	GenericChartIndicatorWrapper(std::unique_ptr<Indicator> indicator)
		: GenericIndicatorWrapper(std::move(indicator)) { }

	GenericChartIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
		: GenericIndicatorWrapper(std::move(indicator), security) { }

	GenericChartIndicatorWrapper(const GenericChartIndicatorWrapper& other) = default;
	GenericChartIndicatorWrapper& operator=(const GenericChartIndicatorWrapper& other) = default;
	GenericChartIndicatorWrapper(GenericChartIndicatorWrapper&& other) noexcept = default;
	GenericChartIndicatorWrapper& operator=(GenericChartIndicatorWrapper&& other) noexcept = default;

	virtual void Calculate() override;
	void CalculateLabelWidth();
	virtual void DrawCustomChart(double chart_height, ImPlotAxisFlags x_axis_flags, ImPlotAxisFlags y_axis_flags, ImPlotRect& shared_limits, bool& is_any_plot_hovered, bool show_highlight, ImPlotPoint& hovered_mouse_point, float hover_highlight_l, float hover_highlight_r, ImVec4 bull_color, ImVec4 bear_color);
	
	virtual std::unique_ptr<IIndicatorWrapper> Clone() override
	{
		return std::make_unique<GenericChartIndicatorWrapper>(*this);
	}

	virtual bool IsIndicatorOverlayable() override { return false; }

	inline float GetLabelWidth() const { return m_label_width; }

protected:
	virtual void PlotItems(ImVec4 bull_color, ImVec4 bear_color);


	ImPlotRect m_chart_limits;
	float m_label_width = 0.0f;

	double m_x_axis_min = DBL_MAX;
	double m_x_axis_max = -DBL_MAX;

private:
	virtual void PlotPreCandle(ImVec4 bull_color, ImVec4 bear_color) override;
	virtual void PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color) override;
};


/*********************************************************************************
*                                Bollinger Band
**********************************************************************************/

class BollingerBandIndicatorWrapper : public GenericIndicatorWrapper
{
public:
	BollingerBandIndicatorWrapper() : GenericIndicatorWrapper() {}
	BollingerBandIndicatorWrapper(std::unique_ptr<Indicator> indicator)
		: GenericIndicatorWrapper(std::move(indicator)) { }

	BollingerBandIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
		: GenericIndicatorWrapper(std::move(indicator), security) { }

	BollingerBandIndicatorWrapper(const BollingerBandIndicatorWrapper& other) = default;
	BollingerBandIndicatorWrapper& operator=(const BollingerBandIndicatorWrapper& other) = default;
	BollingerBandIndicatorWrapper(BollingerBandIndicatorWrapper&& other) noexcept = default;
	BollingerBandIndicatorWrapper& operator=(BollingerBandIndicatorWrapper&& other) noexcept = default;

	virtual bool DrawAsAvailableIndicator() override;
	virtual bool DrawAsAppliedIndicator() override;
	virtual void PlotPreCandle(ImVec4 bull_color, ImVec4 bear_color) override;
	virtual void PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color) override;
	virtual void Calculate() override;
	virtual bool IsIndicatorOverlayable() override { return true; }

	virtual std::unique_ptr<IIndicatorWrapper> Clone() override
	{
		return std::make_unique<BollingerBandIndicatorWrapper>(*this);
	}

	virtual std::string GetHumanReadableValueAt(size_t index) const override;

	virtual void FromJson(Json::Value value) override;
	virtual Json::Value ToJson() const override;
};


/*********************************************************************************
*                                ROC
**********************************************************************************/
class ROCIndicatorWrapper : public GenericChartIndicatorWrapper
{
public:
	ROCIndicatorWrapper() : GenericChartIndicatorWrapper() {}
	ROCIndicatorWrapper(std::unique_ptr<Indicator> indicator)
		: GenericChartIndicatorWrapper(std::move(indicator)) {
	}

	ROCIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
		: GenericChartIndicatorWrapper(std::move(indicator), security) {
	}

	ROCIndicatorWrapper(const ROCIndicatorWrapper& other) = default;
	ROCIndicatorWrapper& operator=(const ROCIndicatorWrapper& other) = default;
	ROCIndicatorWrapper(ROCIndicatorWrapper&& other) noexcept = default;
	ROCIndicatorWrapper& operator=(ROCIndicatorWrapper&& other) noexcept = default;

	virtual std::unique_ptr<IIndicatorWrapper> Clone() override
	{
		return std::make_unique<ROCIndicatorWrapper>(*this);
	}

	virtual bool IsIndicatorOverlayable() override { return false; }

protected:
	virtual void PlotItems(ImVec4 bull_color, ImVec4 bear_color) override;
};

/*********************************************************************************
*                                RSI
**********************************************************************************/
class RSIIndicatorWrapper : public GenericChartIndicatorWrapper
{
public:
	RSIIndicatorWrapper() : GenericChartIndicatorWrapper() {}
	RSIIndicatorWrapper(std::unique_ptr<Indicator> indicator)
		: GenericChartIndicatorWrapper(std::move(indicator)) {
	}

	RSIIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
		: GenericChartIndicatorWrapper(std::move(indicator), security) {
	}

	RSIIndicatorWrapper(const RSIIndicatorWrapper& other) = default;
	RSIIndicatorWrapper& operator=(const RSIIndicatorWrapper& other) = default;
	RSIIndicatorWrapper(RSIIndicatorWrapper&& other) noexcept = default;
	RSIIndicatorWrapper& operator=(RSIIndicatorWrapper&& other) noexcept = default;

	virtual std::unique_ptr<IIndicatorWrapper> Clone() override
	{
		return std::make_unique<RSIIndicatorWrapper>(*this);
	}

	virtual bool IsIndicatorOverlayable() override { return false; }

protected:
	virtual void PlotItems(ImVec4 bull_color, ImVec4 bear_color) override;
};



/*********************************************************************************
*                                     OBV
**********************************************************************************/

class OBVIndicatorWrapper : public GenericChartIndicatorWrapper
{
public:
	OBVIndicatorWrapper() : GenericChartIndicatorWrapper() {}
	OBVIndicatorWrapper(std::unique_ptr<Indicator> indicator)
		: GenericChartIndicatorWrapper(std::move(indicator)) { }

	OBVIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
		: GenericChartIndicatorWrapper(std::move(indicator), security) { }

	OBVIndicatorWrapper(const OBVIndicatorWrapper& other) = default;
	OBVIndicatorWrapper& operator=(const OBVIndicatorWrapper& other) = default;
	OBVIndicatorWrapper(OBVIndicatorWrapper&& other) noexcept = default;
	OBVIndicatorWrapper& operator=(OBVIndicatorWrapper&& other) noexcept = default;

	virtual bool DrawAsAvailableIndicator() override;
	virtual bool DrawAsAppliedIndicator() override;
	virtual bool IsIndicatorOverlayable() override { return false; }

	virtual std::unique_ptr<IIndicatorWrapper> Clone() override
	{
		return std::make_unique<OBVIndicatorWrapper>(*this);
	}
};



/*********************************************************************************
*                                     MACD
**********************************************************************************/

class MACDIndicatorWrapper : public GenericChartIndicatorWrapper
{
public:
	MACDIndicatorWrapper() : GenericChartIndicatorWrapper() {}
	MACDIndicatorWrapper(std::unique_ptr<Indicator> indicator)
		: GenericChartIndicatorWrapper(std::move(indicator)) { }

	MACDIndicatorWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
		: GenericChartIndicatorWrapper(std::move(indicator), security) {	}

	MACDIndicatorWrapper(const MACDIndicatorWrapper& other) = default;
	MACDIndicatorWrapper& operator=(const MACDIndicatorWrapper& other) = default;
	MACDIndicatorWrapper(MACDIndicatorWrapper&& other) noexcept = default;
	MACDIndicatorWrapper& operator=(MACDIndicatorWrapper&& other) noexcept = default;

	virtual void SetIndicator(std::unique_ptr<Indicator> indicator) override;
	virtual bool DrawAsAvailableIndicator() override;
	virtual bool DrawAsAppliedIndicator() override;
	virtual bool IsIndicatorOverlayable() override { return false; }
	
	virtual std::string GetHumanReadableValueAt(size_t index) const override;
	virtual void FromJson(Json::Value value) override;
	virtual Json::Value ToJson() const override;

	virtual std::unique_ptr<IIndicatorWrapper> Clone() override
	{
		return std::make_unique<MACDIndicatorWrapper>(*this);
	}

protected:
	virtual void PlotItems(ImVec4 bull_color, ImVec4 bear_color) override;
};




/*********************************************************************************
*                                Savitzky Golay Filter
**********************************************************************************/

class TrendAnalysisDebugWrapper : public GenericIndicatorWrapper
{
public:
	TrendAnalysisDebugWrapper() : GenericIndicatorWrapper() {}
	TrendAnalysisDebugWrapper(std::unique_ptr<Indicator> indicator)
		: GenericIndicatorWrapper(std::move(indicator)) {}

	TrendAnalysisDebugWrapper(std::unique_ptr<Indicator> indicator, std::shared_ptr<Security> security)
		: GenericIndicatorWrapper(std::move(indicator), security) {}

	TrendAnalysisDebugWrapper(const TrendAnalysisDebugWrapper& other) = default;
	TrendAnalysisDebugWrapper& operator=(const TrendAnalysisDebugWrapper& other) = default;
	TrendAnalysisDebugWrapper(TrendAnalysisDebugWrapper&& other) noexcept = default;
	TrendAnalysisDebugWrapper& operator=(TrendAnalysisDebugWrapper&& other) noexcept = default;

	virtual bool DrawAsAvailableIndicator() override;
	virtual bool DrawAsAppliedIndicator() override;
	virtual void PlotPostCandle(ImVec4 bull_color, ImVec4 bear_color) override;

	virtual void FromJson(Json::Value value) override;
	virtual Json::Value ToJson() const override;

	virtual std::string GetHumanReadableValueAt(size_t index) const override;

	virtual std::unique_ptr<IIndicatorWrapper> Clone() override
	{
		return std::make_unique<TrendAnalysisDebugWrapper>(*this);
	}
};