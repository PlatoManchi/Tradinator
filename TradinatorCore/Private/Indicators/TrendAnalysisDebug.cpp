#include "Indicators/TrendAnalysisDebug.h"

#include <iostream>

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"

#include "Utils/StopWatch.h"

#include "matrix_helper_ispc.h"
#include "indicator_helper_ispc.h"

void PrintMatrix(int64_t* mat, uint64_t row, uint64_t col) 
{
	for (int y = 0; y < row; ++y)
	{
		for (int16_t x = 0; x <col; x++)
		{
			uint32_t index = (y) * col + x;
			if (mat[index] < 10)
			{
				std::cout << "     " << mat[index];
			}
			else if (mat[index] < 100)
			{
				std::cout << "    " << mat[index];
			}
			else
			{
				std::cout << "   " << mat[index];
			}
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
}

void PrintMatrix(double* mat, uint64_t row, uint64_t col)
{
	for (int y = 0; y < row; ++y)
	{
		for (int16_t x = 0; x < col; x++)
		{
			uint32_t index = (y) * col + x;
			if (mat[index] < 10)
			{
				std::cout << "     " << mat[index];
			}
			else if (mat[index] < 100)
			{
				std::cout << "    " << mat[index];
			}
			else
			{
				std::cout << "   " << mat[index];
			}
		}
		std::cout << std::endl;
	}


	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
}

std::vector<std::vector<IndicatorPoint>> TrendAnalysisDebug::Calculate()
{
	std::vector<std::vector<IndicatorPoint>> result;

	if (m_length <= 0 || m_polynomial_order % 2 == 0 || m_distance_btw_peaks == 0 || m_width_for_peaks == 0 || m_relative_height == 0)
	{
		return result;
	}
	
	std::shared_ptr<Security> security = m_security.lock();

	if (security)
	{
		const std::shared_ptr<const AsyncData<CandleDataMapType>>& candle_data = security->GetCandleData();
		bool is_ready = candle_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candle_data->IsDataReady();
		}

		StopWatch stop_watch(GetName());

		size_t count = candle_data->GetData().size();
		if (count == 0) return result;

		std::vector<IndicatorPoint> trend_analysis_debug;
		trend_analysis_debug.reserve(count);

		std::vector<double> highs;
		std::vector<double> lows;
		std::vector<double> closes;
		highs.reserve(count);
		lows.reserve(count);
		closes.reserve(count);

#ifdef _SAVITZKY_GOLAY_FILTER_ISPC_
		std::vector<double> a(m_polynomial_order * m_length);
		std::vector<double> at(m_polynomial_order * m_length);
		std::vector<double> ata(m_length * m_length);
		std::vector<double> ata_inv(m_length * m_length);
		std::vector<double> ata_inv_at(m_length * m_polynomial_order);
		std::vector<double> convolution_coefficient(m_polynomial_order);
		std::vector<double> convolution_coefficient_tmp(m_polynomial_order);

		std::vector<double> ispc_input;
		ispc_input.reserve(count);
		std::vector<double> ispc_output(count);

		const CandleDataMapType& data = candle_data->GetData();
		for (auto& pair : data)
		{
			ispc_input.emplace_back(pair.second.m_close);
		}

		ispc::calculate_trend_analysis_debug(
			ispc_input.data(), 
			count, 
			a.data(), 
			at.data(), 
			ata.data(), 
			ata_inv.data(), 
			ata_inv_at.data(), 
			convolution_coefficient.data(), 
			convolution_coefficient_tmp.data(), 
			m_polynomial_order, 
			m_length, 
			ispc_output.data());

		auto itr = candle_data->GetData().begin();
		for (double sma_value : ispc_output)
		{
			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = sma_value;

			trend_analysis_debug.emplace_back(std::move(point));

			std::advance(itr, 1);
		}

#else
		double* a = VandermondeMatrix(m_polynomial_order, m_length);
		//PrintMatrix(a, m_polynomial_order, m_length);
		double* at = Transpose(a, m_polynomial_order, m_length);
		double* ata = Multiply(at, m_length, m_polynomial_order, a, m_polynomial_order, m_length);
		double* ata_inv = Inverse(ata, m_length);
		double* ata_inv_at = Multiply(ata_inv, m_length, m_length, at, m_length, m_polynomial_order);

		/*std::cout << "Convultion Coefficients: " << std::endl;
		for (int i = 0; i < m_length; ++i)
		{
			for (int j = 0; j < m_polynomial_order; ++j)
			{
				std::cout << ata_inv_at[i * m_polynomial_order + j] << ",  ";
			}

			std::cout << std::endl;
		}*/

		double* coefficients = new double[m_polynomial_order];
		for (uint64_t i = 0; i < m_polynomial_order; ++i)
		{
			coefficients[i] = ata_inv_at[i];
		}

		// will use this in boundry conditions
		double* coefficients_tmp = new double[m_polynomial_order];
		for (uint64_t i = 0; i < m_polynomial_order; ++i)
		{
			coefficients_tmp[i] = coefficients[i];
		}

		uint16_t half_poly = m_polynomial_order / 2;
		std::vector<double> smoothed_values;
		smoothed_values.reserve(count);

		auto itr = candle_data->GetData().begin();
		for (uint64_t i = 0; i < count; ++i)
		{
			size_t effective_window_size = m_polynomial_order;
			double* coefficients_to_use = coefficients;
			size_t left = i - half_poly;
			size_t right = i + half_poly;

			// At boundary conditions, normalize only th epart of coefficients that we use.
			if (i < half_poly)
			{
				left = 0;
				right = i + half_poly;

				effective_window_size = half_poly + i + 1;

				for (uint64_t j = 0; j < m_polynomial_order; ++j)
				{
					coefficients_tmp[j] = coefficients[j];
				}
				coefficients_to_use = coefficients_tmp + half_poly - i;
				Normalize(coefficients + half_poly - i, coefficients_to_use, effective_window_size);
			}
			if (i >= count - half_poly - 1)
			{
				left = i - half_poly;
				right = count - 1;

				effective_window_size = half_poly + 1 + ((count - 1) - i);

				for (uint64_t j = 0; j < m_polynomial_order; ++j)
				{
					coefficients_tmp[j] = coefficients[j];
				}
				coefficients_to_use = coefficients_tmp;
				Normalize(coefficients, coefficients_to_use, effective_window_size);
			}

			double smoothed_value = 0;
			size_t coefficient_index = 0;

			auto itr_tmp = std::prev(itr, i - left);
			for (size_t j = left; j <= right; ++j)
			{
				double coeff = coefficients_to_use[coefficient_index];
				smoothed_value += (coeff * (*itr_tmp).second.m_close);

				std::advance(itr_tmp, 1);
				coefficient_index++;
			}

			smoothed_values.push_back(smoothed_value);
			highs.push_back((*itr).second.m_high);
			lows.push_back((*itr).second.m_low);
			closes.push_back((*itr).second.m_close);

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = smoothed_value;

			trend_analysis_debug.emplace_back(std::move(point));

			std::advance(itr, 1);
		}

		delete[] a;
		delete[] at;
		delete[] ata;
		delete[] ata_inv;
		delete[] ata_inv_at;
		delete[] coefficients;
		delete[] coefficients_tmp;


		std::vector<double> true_ranges(count);
		std::vector<double> average_true_ranges(count);
		ispc::calculate_atr(highs.data(), lows.data(), closes.data(), true_ranges.data(), average_true_ranges.data(), count, 30);

		std::vector<size_t> peaks;
		std::vector<size_t> troughs;

		FindPeaks(smoothed_values, peaks, average_true_ranges, m_distance_btw_peaks, m_width_for_peaks, m_relative_height);
		FindPeaks(smoothed_values, troughs, average_true_ranges, m_distance_btw_peaks, m_width_for_peaks, m_relative_height, -1);

		std::vector<IndicatorPoint> peak_points;
		peak_points.reserve(count);
		std::vector<IndicatorPoint> trough_points;
		trough_points.reserve(count);

		for (size_t peak : peaks)
		{
			auto itr = candle_data->GetData().begin();
			std::advance(itr, peak);

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = smoothed_values[peak];

			peak_points.emplace_back(std::move(point));
		}

		for (size_t trough : troughs)
		{
			auto itr = candle_data->GetData().begin();
			std::advance(itr, trough);

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = smoothed_values[trough];

			trough_points.emplace_back(std::move(point));
		}

		// Reversing becuase fuck my decision to have data in ascending order
		//std::reverse(peaks.begin(), peaks.end());
		//std::reverse(troughs.begin(), troughs.end());
		std::vector<IndicatorPoint> trend_points;
		trend_points.reserve(count);
		
		itr = candle_data->GetData().begin();
		for (size_t i = 0; i < count; ++i)
		{
			size_t peak_index = 0;
			for (size_t j = 0; j < peaks.size(); ++j)
			{
				if (peaks[j] > i)
				{
					peak_index = j;
					break;
				}
			}

			size_t trough_index = 0;
			for (size_t j = 0; j < troughs.size(); ++j)
			{
				if (troughs[j] > i)
				{
					trough_index = j;
					break;
				}
			}

			//  0 - none
			//  1 - up trend
			// -1 - down trend
			double trend = 0;
			size_t history_length = 2;
			if (peak_index + history_length < peaks.size() &&
				trough_index + history_length < troughs.size())
			{
				size_t peaks_up_count = 0;
				size_t troughs_up_count = 0;
				size_t peaks_down_count = 0;
				size_t troughs_down_count = 0;

				double threshold = 0.001;
				for (int j = peak_index; j < peak_index + history_length; ++j)
				{
					if (fabs(smoothed_values[peaks[j]] - smoothed_values[peaks[j + 1]]) < smoothed_values[peaks[j + 1]] * threshold ||
						smoothed_values[peaks[j]] > smoothed_values[peaks[j + 1]])
					{
						peaks_up_count++;
					}
					if (fabs(smoothed_values[peaks[j]] - smoothed_values[peaks[j + 1]]) < smoothed_values[peaks[j + 1]] * threshold || 
						smoothed_values[peaks[j]] < smoothed_values[peaks[j + 1]])
					{
						peaks_down_count++;
					}
				}
				for (int j = trough_index; j < trough_index + history_length; ++j)
				{
					if (fabs(smoothed_values[peaks[j]] - smoothed_values[peaks[j + 1]]) < smoothed_values[peaks[j + 1]] * threshold || 
						smoothed_values[troughs[j]] > smoothed_values[troughs[j + 1]])
					{
						troughs_up_count++;
					}
					if (fabs(smoothed_values[peaks[j]] - smoothed_values[peaks[j + 1]]) < smoothed_values[peaks[j + 1]] * threshold || 
						smoothed_values[troughs[j]] < smoothed_values[troughs[j + 1]])
					{
						troughs_down_count++;
					}
				}

				if (peaks_up_count == history_length && troughs_up_count == history_length)
				{
					trend = 1.0f;
				}
				else if (peaks_down_count == history_length && troughs_down_count == history_length)
				{
					trend = -1.0f;
				}
			}

			IndicatorPoint point;
			point.date = (*itr).first;
			point.value = trend;

			trend_points.emplace_back(std::move(point));

			std::advance(itr, 1);
		}

#endif // _SAVITZKY_GOLAY_FILTER_ISPC_


		result.emplace_back(std::move(trend_analysis_debug));
		result.emplace_back(std::move(peak_points));
		result.emplace_back(std::move(trough_points));
		result.emplace_back(std::move(trend_points));
	}
	

	return result;
}






double find_local_min_left(const std::vector<double>& input_data, size_t at_index, int modifier)
{
	for (size_t i = at_index; i > 0; --i)
	{
		if (input_data[i] * modifier < input_data[i - 1] * modifier)
			return input_data[i] * modifier;
	}

	return input_data[0] * modifier;
}
	
double find_local_min_right(const std::vector<double>& input_data, size_t at_index, int modifier)
{
	size_t count = input_data.size();
	for (size_t i = at_index; i < count - 1; ++i)
	{
		if (input_data[i] * modifier < input_data[i + 1] * modifier)
		{
			return input_data[i] * modifier;
		}
	}

	return input_data[count - 1] * modifier;
}

size_t find_crossing_left(const std::vector<double>& input_data, size_t at_index, double height, int modifier)
{
	for (int64_t i = at_index; i >= 0; --i)
	{
		if (input_data[i] * modifier < height)
		{
			return i;
		}
		else if (input_data[i] * modifier > input_data[at_index] * modifier)
		{
			return at_index;
		}
	}

	return 0;
}

size_t find_crossing_right(const std::vector<double>& input_data, size_t at_index, double height, int modifier)
{
	size_t count = input_data.size();
	for (size_t i = at_index; i < count; ++i)
	{
		if (input_data[i] * modifier < height)
		{
			return i;
		}
		else if (input_data[i] * modifier > input_data[at_index] * modifier)
		{
			return at_index;
		}
	}

	return count - 1;
}

size_t find_next_peak(const std::vector<double>& input_data, const std::vector<size_t>& all_peaks, size_t start, uint64_t min_distance, int modifier)
{
	size_t count = all_peaks.size();
	if (start >= count)
	{
		return -1;
	}

	if (start == count - 1 ||
		all_peaks[start + 1] - all_peaks[start] > min_distance)
	{
		return start;
	}

	size_t result = start;
	for (size_t i = start; i < count - 1; ++i)
	{
		if (all_peaks[i] - all_peaks[start] <= min_distance)
		{
			if (input_data[all_peaks[i]] * modifier > input_data[all_peaks[result]] * modifier)
			{
				result = i;
			}
		}
		else
		{
			break;
		}
	}

	return result;
}

void TrendAnalysisDebug::FindPeaks(const std::vector<double>& input_data, std::vector<size_t>& output_peaks_indices, std::vector<double> prominences, uint64_t min_distance, uint64_t min_width, double relative_height, int input_modifier)
{
	output_peaks_indices.clear();
	std::vector<size_t> all_peaks;

	size_t count = input_data.size();

	for (size_t i = 1; i < count - 2; ++i)
	{
		if (input_data[i] * input_modifier > input_data[i - 1] * input_modifier && input_data[i] * input_modifier > input_data[i + 1] * input_modifier)
		{
			double peak_height = input_data[i] * input_modifier;

			// Step 1: Estimate prominence
			double prominence = 0;
			if (prominences.size() >= count)
			{
				prominence = prominences[i];
			}
			else
			{
				// Estimate prominence
				double left_min = find_local_min_left(input_data, i, input_modifier);
				double right_min = find_local_min_right(input_data, i, input_modifier);
				double base_height = std::max(left_min, right_min);
				prominence = peak_height - base_height;
			}

			// Step 2: Compute height at which to measure width
			double height_at_width = peak_height - (prominence * relative_height);

			// Step 3: Walk left and right from peak to find crossing points
			size_t left_index = find_crossing_left(input_data, i, height_at_width, input_modifier);
			size_t right_index = find_crossing_right(input_data, i, height_at_width, input_modifier);

			// Step 4: Measure width
			size_t width = right_index - left_index;
			
			// Step 5: Filter by width
			if (width >= min_width && left_index != i && right_index != i)
			{
				all_peaks.push_back(i);
			}
		}
	}

	// Step 6: Now we have all the peaks in the data. Filter the peaks based on distance
	//std::reverse(all_peaks.begin(), all_peaks.end());

	//output_peaks_indices = all_peaks;

	bool is_done = false;
	size_t start = 0;
	while (!is_done)
	{
		size_t index = find_next_peak(input_data, all_peaks, start, min_distance, input_modifier);
		if (index == -1)
		{
			is_done = true;
			break;
		}

		output_peaks_indices.push_back(all_peaks[index]);

		if (index == all_peaks.size() - 1)
		{
			is_done = true;
			break;
		}
		
		bool found_next = false;
		for (size_t i = index; i < all_peaks.size(); ++i)
		{
			if (all_peaks[i] - all_peaks[index] > min_distance)
			{
				found_next = true;
				start = i;
				break;
			}
		}

		if (!found_next)
		{
			is_done = false;
			break;
		}
	}
}


















void TrendAnalysisDebug::Normalize(double* input, double* output, uint64_t size)
{
	double total = 0;
	for (uint64_t i = 0; i < size ; ++i)
	{
		total += input[i];
	}
	total = fabs(total);

	for (uint64_t i = 0; i < size; ++i)
	{
		output[i] = input[i] / total;
	}
}

double* TrendAnalysisDebug::Transpose(double* matrix, uint16_t row, uint16_t col)
{
	double* result = new double[col * row];

	for (uint16_t y = 0; y < row; ++y)
	{
		for (uint16_t x = 0; x < col; ++x)
		{
			uint32_t index_1 = y * col + x;
			uint32_t index_2 = x * row + y;

			result[index_2] = matrix[index_1];
		}
	}

	return result;
}

double* TrendAnalysisDebug::Multiply(double* matrix_1, uint16_t row_1, uint16_t col_1, double* matrix_2, uint16_t row_2, uint16_t col_2)
{
	if (col_1 != row_2) return nullptr;
	double* result = new double[row_1 * col_2];

	memset(result, 0, sizeof(double) * row_1 * col_2);


	for (int i = 0; i < row_1; i++) {
		for (int j = 0; j < col_2; j++) {
			for (int k = 0; k < col_1; k++) {
				uint16_t result_index = i * col_2 + j;
				uint16_t mat_1_index = i * col_1 + k;
				uint16_t mat_2_index = k * col_2 + j;

				result[result_index] += matrix_1[mat_1_index] * matrix_2[mat_2_index];
			}
		}
	}

	return result;
}

double* TrendAnalysisDebug::VandermondeMatrix(uint16_t polynomial_order, uint16_t window_size) const
{
	if (polynomial_order % 2 == 0) return nullptr;

	double* result = new double[window_size * polynomial_order];
	int16_t half_poly = polynomial_order / 2;
	
	uint16_t row = 0;
	for (int poly = -half_poly; poly <= half_poly; ++poly)
	{
		for (int16_t x = 0; x < window_size; x++)
		{
			uint32_t index = row * window_size + x;
			result[index] = pow(poly, x);
		}
		row++;
	}
	

	return result;
}


double* TrendAnalysisDebug::Inverse(double* input, uint64_t size)
{
	double* input_tmp = new double[size * size];
	memcpy(input_tmp, input, sizeof(double) * size * size);
	double* output = new double[size * size];
	
	// make it a identity matrix
	for (uint64_t i = 0; i < size; ++i)
	{
		for (uint64_t j = 0; j < size; ++j)
		{
			output[i * size + j] = (i == j) ? 1.0 : 0.0;
		}
	}

	// Gaussian–Jordan row elimination method for calculating inverse of matrix
	// https://youtu.be/vKBNzM3V-Rc?si=qcLywp_BXTuHxQp1

	for (uint64_t i = 0; i < size; ++i)
	{
		int64_t pivot = input_tmp[i * size + i];
		for (uint64_t row = 0; row < size; ++row)
		{
			if (row != i)
			{
				double multiplier = -((double)input_tmp[row * size + i]/(double)pivot);
				for (uint64_t col = 0; col < size; ++col)
				{
					input_tmp[row * size + col] = input_tmp[row * size + col] + input_tmp[i * size + col] * multiplier;
					output[row * size + col] = output[row * size + col] + output[i * size + col] * multiplier;
				}
			}
		}
	}
	

	for (uint64_t row = 0; row < size; ++row)
	{
		int64_t pivot = input_tmp[row * size + row];
		for (uint64_t col = 0; col < size; ++col)
		{
			output[row * size + col] = output[row * size + col] / pivot;
		}
	}

	delete[] input_tmp;

	return output;
}