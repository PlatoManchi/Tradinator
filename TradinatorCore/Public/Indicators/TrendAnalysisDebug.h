#pragma once

#if 0
#define _SAVITZKY_GOLAY_FILTER_ISPC_
#else
#endif

#include "Indicators/Indicator.h"

// Not an actual indicator, but this way its faster to draw the values on top of price chart
// for debugging

class TrendAnalysisDebug : public Indicator
{
public:
	TrendAnalysisDebug() : Indicator() {}

	TrendAnalysisDebug(size_t length) : Indicator(length) {};
	TrendAnalysisDebug(std::weak_ptr<Counter> counter, size_t length) : Indicator(counter, length) {};

	virtual std::vector<std::vector<IndicatorPoint>> Calculate() override;

	virtual std::string GetName() const override { return std::format("Trend Analysis Debug"); }
	virtual EIndicatorType IndicatorType() const override { return EIndicatorType::E_TrendAnalysisDebug; }
	virtual std::unique_ptr<Indicator> Clone() override
	{
		return std::make_unique<TrendAnalysisDebug>(*this);
	}

	inline uint16_t GetPolynomialOrder() const { return m_polynomial_order; }
	void SetPolynomialOrder(uint16_t order)
	{
		m_polynomial_order = order;
	}

	inline uint16_t GetDistance() const { return m_distance_btw_peaks; }
	void SetDistance(uint16_t distance)
	{
		m_distance_btw_peaks = distance;
	}

	inline uint16_t GetWidth() const { return m_width_for_peaks; }
	void SetWidth(uint16_t width)
	{
		m_width_for_peaks = width;
	}

	inline double GetRelativeWidth() const { return m_relative_height; }
	void SetRelativeWidth(double relative_width)
	{
		m_relative_height = relative_width;
	}

protected:
	double* VandermondeMatrix(uint16_t window_size, uint16_t polynomial_order) const;
	double* Transpose(double* matrix, uint16_t m, uint16_t n);
	double* Multiply(double* matrix_1, uint16_t row_1, uint16_t col_1, double* matrix_2, uint16_t row_2, uint16_t col_2);
	double* Inverse(double* matrix, uint64_t size);
	void Normalize(double* input, double* output, uint64_t size);

	void FindPeaks(const std::vector<double>& input_data, std::vector<size_t>& output_peaks_indices, std::vector<double> prominance = {}, uint64_t min_distance = 15, uint64_t min_width = 3, double relative_height = 0.5, int modifier = 1);

	uint16_t m_polynomial_order = 5;
	uint16_t m_distance_btw_peaks = 15;
	uint16_t m_width_for_peaks = 3;
	double m_relative_height = 0.5;
};

