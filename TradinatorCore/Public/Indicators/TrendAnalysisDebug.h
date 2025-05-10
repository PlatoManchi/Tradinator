#pragma once

#if 1
#define _SAVITZKY_GOLAY_FILTER_ISPC_
#else
#endif

#include "Indicators/Indicator.h"

// Not an actual indicator, but this way its faster to draw the values on top of price chart
// for debugging

class TrendAnalysisDebug : public Indicator
{
public:
	TrendAnalysisDebug() : Indicator() 
	{
		m_length = 4;
	}

	TrendAnalysisDebug(size_t length) : Indicator(length) {};
	TrendAnalysisDebug(size_t length, std::weak_ptr<Security> security) : Indicator(length, security) {};

	virtual std::vector<std::vector<double>> Calculate() override;
	void CalculateRaw(
		const double* highs,
		const double* lows,
		const double* closes,
		double* a_buff,
		double* at_buff,
		double* ata_buff,
		double* ata_inv_buff,
		double* ata_inv_tmp_buff,
		double* ata_inv_at_buff,
		double* convolution_coefficient,
		double* convolution_coefficient_buff,
		double* savitzky_golay_output,
		double* atr_output,
		double* peaks_output,
		double* troughs_output,
		double* trend_output,
		uint64_t* peaks_count,
		uint64_t* troughs_count,
		uint64_t window_size,
		uint64_t polynomial_order,
		uint64_t distance_btw_peaks,
		uint64_t width_for_peaks,
		double relative_height_for_peaks,
		uint64_t history_length_for_trend_detection,
		uint64_t count);

	void SavitzkyGolayFilterRaw(
		const double* input,
		double* convolution_coefficient,
		double* convolution_coefficient_buff,
		double* output,
		uint64_t window_size,
		uint64_t polynomial_order,
		uint64_t count);

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
	void VandermondeMatrix(double* output, uint16_t window_size, uint16_t polynomial_order) const;
	void Transpose(double* matrix, double* output, uint16_t m, uint16_t n);
	void Multiply(double* matrix_1, uint16_t row_1, uint16_t col_1, double* matrix_2, uint16_t row_2, uint16_t col_2, double* output);
	void Inverse(const double* matrix, double* tmp_buffer, double* output, uint64_t size);
	void Normalize(double* input, double* output, uint64_t size);

	void FindPeaks(const double* input_data, uint64_t count, double* output_peaks_indices, uint64_t* peaks_count, double* prominences = nullptr, uint64_t min_distance = 15, uint64_t min_width = 3, double relative_height = 0.5, int input_modifier = 1);

	uint16_t m_polynomial_order = 49;
	uint16_t m_distance_btw_peaks = 15;
	uint16_t m_width_for_peaks = 3;
	double m_relative_height = 0.5;
};

