#include "Indicators/TrendAnalysisDebug.h"

#include <iostream>

#include <iostream>

#include "Data/Security.h"
#include "Data/AsyncData.h"

#include  "Indicators/ATR.h"
#include "Utils/StopWatch.h"

#ifdef _SAVITZKY_GOLAY_FILTER_ISPC_
#include "utils_ispc.h"
#endif // _SAVITZKY_GOLAY_FILTER_ISPC_


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



bool TrendAnalysisDebug::operator==(const TrendAnalysisDebug& other) const
{
	return m_source == other.m_source &&
		m_length == other.m_length &&
		IndicatorType() == other.IndicatorType() &&
		m_polynomial_order == other.m_polynomial_order &&
		m_distance_btw_peaks == other.m_distance_btw_peaks &&
		m_width_for_peaks == other.m_width_for_peaks &&
		m_relative_height == other.m_relative_height;
}



std::vector<std::vector<double>> TrendAnalysisDebug::Calculate()
{
	std::vector<std::vector<double>> result;

	if (m_length <= 0 || m_polynomial_order % 2 == 0 || m_distance_btw_peaks == 0 || m_width_for_peaks == 0 || m_relative_height == 0)
	{
		return result;
	}

	std::shared_ptr<Security> security = m_security.lock();

	if (security)
	{
		const std::shared_ptr<const AsyncData<CandlesData>>& candles_data = security->GetCandlesData();
		bool is_ready = candles_data->IsDataReady();
		while (!is_ready)
		{
			is_ready = candles_data->IsDataReady();
		}

		StopWatch stop_watch(GetName());

		const CandlesData& data = candles_data->GetData();
		uint64_t count = data.m_dates.size();

		if (count == 0) return result;

		std::vector<double> a(m_polynomial_order * m_length, 0.0);
		std::vector<double> at(m_polynomial_order * m_length, 0.0);
		std::vector<double> ata(m_length * m_length, 0.0);
		std::vector<double> ata_inv(m_length * m_length, 0.0);
		std::vector<double> ata_inv_tmp(m_length * m_length, 0.0);// buffer to calculate inverse
		std::vector<double> ata_inv_at(m_length * m_polynomial_order, 0.0);
		std::vector<double> convolution_coefficient(m_polynomial_order, 0.0);
		std::vector<double> convolution_coefficient_tmp(m_polynomial_order, 0.0);

		std::vector<double> savitzky_golay_output(count, 0.0);

		std::vector<double> average_true_ranges(count, 0.0);

		// max number of peaks possible will be count / m_distance_btw_peaks
		std::vector<double> peaks(count / m_distance_btw_peaks, 0.0);
		std::vector<double> troughs(count / m_distance_btw_peaks, 0.0);

		uint64_t peaks_count = 0;
		uint64_t troughs_count = 0;

		std::vector<double> trend_points(count, 0.0);

#ifdef _SAVITZKY_GOLAY_FILTER_ISPC_
		std::vector<int8_t> peaks_and_trough_tmp_buff(count, 0);

		ispc::calculate_trend_analysis_debug(
			data.m_highs.data(),
			data.m_lows.data(),
			data.m_closes.data(),
			a.data(),
			at.data(),
			ata.data(),
			ata_inv.data(),
			ata_inv_tmp.data(),
			ata_inv_at.data(),
			convolution_coefficient.data(),
			convolution_coefficient_tmp.data(),
			savitzky_golay_output.data(),
			average_true_ranges.data(),
			peaks.data(),
			troughs.data(),
			peaks_and_trough_tmp_buff.data(),
			trend_points.data(),
			&peaks_count,
			&troughs_count,
			m_length,
			30,
			m_polynomial_order,
			m_distance_btw_peaks,
			m_width_for_peaks,
			m_relative_height,
			2,
			count);
#else
		CalculateRaw(
			data.m_highs.data(),
			data.m_lows.data(),
			data.m_closes.data(),
			a.data(),
			at.data(),
			ata.data(),
			ata_inv.data(),
			ata_inv_tmp.data(),
			ata_inv_at.data(),
			convolution_coefficient.data(),
			convolution_coefficient_tmp.data(),
			savitzky_golay_output.data(),
			average_true_ranges.data(),
			peaks.data(),
			troughs.data(),
			trend_points.data(),
			&peaks_count,
			&troughs_count,
			m_length,
			30,
			m_polynomial_order,
			m_distance_btw_peaks,
			m_width_for_peaks,
			m_relative_height,
			2,
			count
		);
#endif // _SAVITZKY_GOLAY_FILTER_ISPC_

		//std::cout << "peaks_count : " << peaks_count << std::endl;
		//std::cout << "troughs_count : " << troughs_count << std::endl;
		peaks.resize(peaks_count);
		troughs.resize(troughs_count);

		result.emplace_back(std::move(savitzky_golay_output));
		result.emplace_back(std::move(peaks));
		result.emplace_back(std::move(troughs));
		result.emplace_back(std::move(trend_points));
	}

	
	

	return result;
}

void TrendAnalysisDebug::CalculateRaw(
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
	uint64_t atr_window_size,
	uint64_t polynomial_order,
	uint64_t distance_btw_peaks,
	uint64_t width_for_peaks,
	double relative_height_for_peaks,
	uint64_t history_length_for_trend_detection,
	uint64_t count)
{
	VandermondeMatrix(a_buff, polynomial_order, window_size);
	Transpose(a_buff, at_buff, polynomial_order, window_size);
	Multiply(at_buff, window_size, polynomial_order, a_buff, polynomial_order, window_size, ata_buff);
	Inverse(ata_buff, ata_inv_tmp_buff, ata_inv_buff, window_size);
	Multiply(ata_inv_buff, window_size, window_size, at_buff, window_size, polynomial_order, ata_inv_at_buff);

	// first row will become the convolution coefficient
	for (uint64_t i = 0; i < polynomial_order; ++i)
	{
		convolution_coefficient[i] = ata_inv_at_buff[i];
	}


	SavitzkyGolayFilterRaw(closes, convolution_coefficient, convolution_coefficient_buff, savitzky_golay_output, window_size, polynomial_order, count);

	ATR atr;
	atr.CalculateRaw(highs, lows, closes, atr_output, atr_window_size, count);



	FindPeaks(savitzky_golay_output, count, peaks_output, peaks_count, atr_output, distance_btw_peaks, width_for_peaks, relative_height_for_peaks);
	FindPeaks(savitzky_golay_output, count, troughs_output, troughs_count, atr_output, distance_btw_peaks, width_for_peaks, relative_height_for_peaks, -1);

	for (uint64_t i = 0; i < count; ++i)
	{
		uint64_t peak_index = 0;
		for (int64_t j = *peaks_count - 1; j >= 0; --j)
		{
			if (peaks_output[j] < i)
			{
				peak_index = j;
				break;
			}
		}

		uint64_t trough_index = 0;
		for (int64_t j = *troughs_count - 1; j >= 0; --j)
		{
			if (troughs_output[j] < i)
			{
				trough_index = j;
				break;
			}
		}

		//  0 - none
		//  1 - up trend
		// -1 - down trend
		double trend = 0;
		
		if (peak_index - history_length_for_trend_detection >= 0 &&
			trough_index - history_length_for_trend_detection >= 0)
		{
			uint64_t peaks_up_count = 0;
			uint64_t troughs_up_count = 0;
			uint64_t peaks_down_count = 0;
			uint64_t troughs_down_count = 0;
			uint64_t perfect_peaks_up_count = 0;
			uint64_t perfect_troughs_up_count = 0;
			uint64_t perfect_peaks_down_count = 0;
			uint64_t perfect_troughs_down_count = 0;

			double threshold = 0.01;
			for (int64_t j = peak_index; j > peak_index - history_length_for_trend_detection && j >= 0; --j)
			{
				uint64_t curr_peak_index = (uint64_t)peaks_output[j];
				uint64_t prev_peak_index = (uint64_t)peaks_output[j - 1];
				if (fabs(savitzky_golay_output[curr_peak_index] - savitzky_golay_output[prev_peak_index]) < savitzky_golay_output[prev_peak_index] * threshold ||
					savitzky_golay_output[curr_peak_index] > savitzky_golay_output[prev_peak_index])
				{
					++peaks_up_count;
				}
				if (savitzky_golay_output[curr_peak_index] > savitzky_golay_output[prev_peak_index])
				{
					++perfect_peaks_up_count;
				}
				if (fabs(savitzky_golay_output[curr_peak_index] - savitzky_golay_output[prev_peak_index]) < savitzky_golay_output[prev_peak_index] * threshold ||
					savitzky_golay_output[curr_peak_index] < savitzky_golay_output[prev_peak_index])
				{
					++peaks_down_count;
				}
				if (savitzky_golay_output[curr_peak_index] < savitzky_golay_output[prev_peak_index])
				{
					++perfect_peaks_down_count;
				}
			}
			for (int j = trough_index; j > trough_index - history_length_for_trend_detection && j >= 0; --j)
			{
				uint64_t curr_trough_index = (uint64_t)troughs_output[j];
				uint64_t prev_trough_index = (uint64_t)troughs_output[j - 1];
				if (fabs(savitzky_golay_output[curr_trough_index] - savitzky_golay_output[prev_trough_index]) < savitzky_golay_output[prev_trough_index] * threshold ||
					savitzky_golay_output[curr_trough_index] > savitzky_golay_output[prev_trough_index])
				{
					++troughs_up_count;
				}
				if (savitzky_golay_output[curr_trough_index] > savitzky_golay_output[prev_trough_index])
				{
					++perfect_troughs_up_count;
				}
				if (fabs(savitzky_golay_output[curr_trough_index] - savitzky_golay_output[prev_trough_index]) < savitzky_golay_output[prev_trough_index] * threshold ||
					savitzky_golay_output[curr_trough_index] < savitzky_golay_output[prev_trough_index])
				{
					++troughs_down_count;
				}
				if (savitzky_golay_output[curr_trough_index] < savitzky_golay_output[prev_trough_index])
				{
					++perfect_troughs_down_count;
				}
			}

			if (peaks_up_count == history_length_for_trend_detection && troughs_up_count == history_length_for_trend_detection)
			{
				trend = 1.0f;
			}
			else if (peaks_down_count == history_length_for_trend_detection && troughs_down_count == history_length_for_trend_detection)
			{
				trend = -1.0f;
			}
		}

		trend_output[i] = trend;
	}
}

















void TrendAnalysisDebug::SavitzkyGolayFilterRaw(
	const double* input,
	double* convolution_coefficient,
	double* convolution_coefficient_buff,
	double* output,
	uint64_t window_size,
	uint64_t polynomial_order,
	uint64_t count)
{
	/*VandermondeMatrix(a_buff, polynomial_order, window_size);
	Transpose(a_buff, at_buff, polynomial_order, window_size);
	Multiply(at_buff, window_size, polynomial_order, a_buff, polynomial_order, window_size, ata_buff);
	Inverse(ata_buff, ata_inv_tmp_buff, ata_inv_buff, window_size);
	Multiply(ata_inv_buff, window_size, window_size, at_buff, window_size, polynomial_order, ata_inv_at_buff);*/

	/*std::cout << "Convultion Coefficients: " << std::endl;
	for (int i = 0; i < m_length; ++i)
	{
		for (int j = 0; j < m_polynomial_order; ++j)
		{
			std::cout << ata_inv_at[i * m_polynomial_order + j] << ",  ";
		}

		std::cout << std::endl;
	}*/

	// will use this in boundry conditions to renormalize based on the windows
	for (uint64_t i = 0; i < polynomial_order; ++i)
	{
		convolution_coefficient_buff[i] = convolution_coefficient[i];
	}

	uint16_t half_poly = polynomial_order / 2;
	std::vector<double> smoothed_values;
	smoothed_values.reserve(count);

	for (uint64_t i = 0; i < count; ++i)
	{
		size_t effective_window_size = polynomial_order;
		double* coefficients_to_use = convolution_coefficient;
		size_t left = i - half_poly;
		size_t right = i + half_poly;

		// At boundary conditions, normalize only th epart of coefficients that we use.
		if (i < half_poly)
		{
			left = 0;
			right = i + half_poly;

			effective_window_size = half_poly + i + 1;

			for (uint64_t j = 0; j < polynomial_order; ++j)
			{
				convolution_coefficient_buff[j] = convolution_coefficient[j];
			}
			coefficients_to_use = convolution_coefficient_buff + half_poly - i;
			Normalize(convolution_coefficient + half_poly - i, coefficients_to_use, effective_window_size);
		}
		if (i >= count - half_poly - 1)
		{
			left = i - half_poly;
			right = count - 1;

			effective_window_size = half_poly + 1 + ((count - 1) - i);

			for (uint64_t j = 0; j < polynomial_order; ++j)
			{
				convolution_coefficient_buff[j] = convolution_coefficient[j];
			}
			coefficients_to_use = convolution_coefficient_buff;
			Normalize(convolution_coefficient, coefficients_to_use, effective_window_size);
		}

		double smoothed_value = 0;
		size_t coefficient_index = 0;

		for (size_t j = left; j <= right; ++j)
		{
			double coeff = coefficients_to_use[coefficient_index];
			smoothed_value += (coeff * input[j]);
			coefficient_index++;
		}

		output[i] = smoothed_value;
	}
}




double find_local_min_left(const double* input_data, uint64_t at_index, uint64_t count, int modifier)
{
	for (size_t i = at_index; i > 0; --i)
	{
		if (input_data[i] * modifier < input_data[i - 1] * modifier)
			return input_data[i] * modifier;
	}

	return input_data[0] * modifier;
}
	
double find_local_min_right(const double* input_data, uint64_t at_index, uint64_t count, int modifier)
{
	for (size_t i = at_index; i < count - 1; ++i)
	{
		if (input_data[i] * modifier < input_data[i + 1] * modifier)
		{
			return input_data[i] * modifier;
		}
	}

	return input_data[count - 1] * modifier;
}

int64_t find_crossing_left(const double* input_data, uint64_t at_index, uint64_t count, double height, int modifier)
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

int64_t find_crossing_right(const double* input_data, uint64_t at_index, uint64_t count, double height, int modifier)
{
	for (uint64_t i = at_index; i < count; ++i)
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

size_t find_next_peak(const double* input_data, const double* all_peaks, uint64_t start, uint64_t peaks_count, uint64_t min_distance, int modifier)
{
	if (start >= peaks_count)
	{
		return -1;
	}

	if (start == peaks_count - 1 ||
		all_peaks[start + 1] - all_peaks[start] > min_distance)
	{
		return start;
	}

	size_t result = start;
	for (size_t i = start; i < peaks_count - 1; ++i)
	{
		if (all_peaks[i] - all_peaks[start] <= min_distance)
		{
			if (input_data[(uint64_t)all_peaks[i]] * modifier > input_data[(uint64_t)all_peaks[result]] * modifier)
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

void TrendAnalysisDebug::FindPeaks(const double* input_data, uint64_t count, double* output_peaks_indices, uint64_t* peaks_count, double* prominences, uint64_t min_distance, uint64_t min_width, double relative_height, int input_modifier)
{
	std::vector<double> all_peaks;

	
	for (size_t i = 1; i < count - 1; ++i)
	{
		if (input_data[i] * input_modifier > input_data[i - 1] * input_modifier && 
			input_data[i] * input_modifier > input_data[i + 1] * input_modifier)
		{
			double peak_height = input_data[i] * input_modifier;

			// Step 1: Estimate prominence
			double prominence = 0;
			if (prominences)
			{
				prominence = prominences[i];
			}
			else
			{
				// Estimate prominence
				double left_min = find_local_min_left(input_data, i, count, input_modifier);
				double right_min = find_local_min_right(input_data, i, count, input_modifier);
				double base_height = std::max(left_min, right_min);
				prominence = peak_height - base_height;
			}

			// Step 2: Compute height at which to measure width
			double height_at_width = peak_height - (prominence * relative_height);

			// Step 3: Walk left and right from peak to find crossing points
			uint64_t left_index = find_crossing_left(input_data, i, count, height_at_width, input_modifier);
			uint64_t right_index = find_crossing_right(input_data, i, count, height_at_width, input_modifier);

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

	/*for (uint64_t i = 0; i < all_peaks.size(); ++i)
	{
		output_peaks_indices[i] = all_peaks[i];
	}
	*peaks_count = all_peaks.size();*/

	bool is_done = false;
	size_t start = 0;
	*peaks_count = 0;

	while (!is_done)
	{
		size_t index = find_next_peak(input_data, all_peaks.data(), start, all_peaks.size(), min_distance, input_modifier);
		if (index == -1)
		{
			is_done = true;
			break;
		}

		output_peaks_indices[*peaks_count] = all_peaks[index];
		(*peaks_count)++;

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

void TrendAnalysisDebug::Transpose(double* matrix, double* output, uint16_t row, uint16_t col)
{
	for (uint16_t y = 0; y < row; ++y)
	{
		for (uint16_t x = 0; x < col; ++x)
		{
			uint32_t index_1 = y * col + x;
			uint32_t index_2 = x * row + y;

			output[index_2] = matrix[index_1];
		}
	}
}

void TrendAnalysisDebug::Multiply(double* matrix_1, uint16_t row_1, uint16_t col_1, double* matrix_2, uint16_t row_2, uint16_t col_2, double* output)
{
	if (col_1 != row_2) return;
	
	memset(output, 0, sizeof(double) * row_1 * col_2);


	for (int i = 0; i < row_1; i++) {
		for (int j = 0; j < col_2; j++) {
			for (int k = 0; k < col_1; k++) {
				uint16_t result_index = i * col_2 + j;
				uint16_t mat_1_index = i * col_1 + k;
				uint16_t mat_2_index = k * col_2 + j;

				output[result_index] += matrix_1[mat_1_index] * matrix_2[mat_2_index];
			}
		}
	}
}



void TrendAnalysisDebug::Inverse(const double* matrix, double* tmp_buffer, double* output, uint64_t size)
{
	memcpy(tmp_buffer, matrix, sizeof(double) * size * size);
	
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
		int64_t pivot = tmp_buffer[i * size + i];
		for (uint64_t row = 0; row < size; ++row)
		{
			if (row != i)
			{
				double multiplier = -((double)tmp_buffer[row * size + i]/(double)pivot);
				for (uint64_t col = 0; col < size; ++col)
				{
					tmp_buffer[row * size + col] = tmp_buffer[row * size + col] + tmp_buffer[i * size + col] * multiplier;
					output[row * size + col] = output[row * size + col] + output[i * size + col] * multiplier;
				}
			}
		}
	}
	

	for (uint64_t row = 0; row < size; ++row)
	{
		int64_t pivot = tmp_buffer[row * size + row];
		for (uint64_t col = 0; col < size; ++col)
		{
			output[row * size + col] = output[row * size + col] / pivot;
		}
	}
}




void TrendAnalysisDebug::VandermondeMatrix(double* output, uint16_t polynomial_order, uint16_t window_size) const
{
	if (polynomial_order % 2 == 0) return;

	int16_t half_poly = polynomial_order / 2;

	uint16_t row = 0;
	for (int poly = -half_poly; poly <= half_poly; ++poly)
	{
		for (int16_t x = 0; x < window_size; x++)
		{
			uint32_t index = row * window_size + x;
			output[index] = pow(poly, x);
		}
		row++;
	}
}