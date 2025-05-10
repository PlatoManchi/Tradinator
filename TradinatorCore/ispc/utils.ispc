// Command to compile and create object and header files
// ispc utils.ispc -o utils_ispc.o -h utils_ispc.h --target=avx2,avx512skx-x64,sse2

#include "indicator_helper.ispc"
//#include "matrix_helper.ispc"


/*****************************************************************************************************
                                         MATRIX OPERATIONS
******************************************************************************************************/

// width has to be odd
export void calculate_vandermonde_matrix(uniform double* uniform output, 
                                uniform uint64 polynomial_order,
                                uniform uint64 window_size)
{
    uniform int64 half_poly = polynomial_order / 2;

    // since all loops are independent, can just run tiled foreach    
    foreach_tiled(poly = -half_poly ... half_poly + 1, x = 0 ... window_size )
    {
        uint64 index = (poly + half_poly) * window_size + x;

        output[index] = pow((double)poly, (double)x);
    }
}

export void transpose_matrix(uniform double* uniform input, 
                                uniform double* uniform output,
                                uniform uint64 rows,
                                uniform uint64 cols)
{
    foreach_tiled(y = 0 ... rows, x = 0 ... cols)
    {
        uint32 index_1 = y * cols + x;
        uint32 index_2 = x * rows + y;

        output[index_2] = input[index_1];
    }
}


// cols_1 == rows_2
export void multiply_matrix(uniform double* uniform mat_1,
                            uniform uint64 rows_1,
                            uniform uint64 cols_1,
                            uniform double* uniform mat_2,
                            uniform uint64 rows_2,
                            uniform uint64 cols_2,
                            uniform double* uniform mat_output)
{
    uniform uint64 total = rows_1 * cols_2;
    // Zero out the memory
    foreach(i=0 ... total)
    {
        mat_output[i] = 0;
    }

    foreach_tiled (i = 0 ... rows_1, j = 0 ... cols_2) {
        for (uint64 k = 0; k < cols_1; ++k) {
            uint64 result_index = i * cols_2 + j;
            uint64 mat_1_index = i * cols_1 + k;
            uint64 mat_2_index = k * cols_2 + j;

            mat_output[result_index] += mat_1[mat_1_index] * mat_2[mat_2_index];
        }
    }
}


export void inverse_matrix(uniform double* uniform input_mat, 
                            uniform double* uniform output_mat,
                            uniform uint64 size)
{
    // identity
    foreach_tiled(i = 0 ... size, j = 0 ... size)
    {
        output_mat[i * size + j] = select(i == j, 1, 0);
    }

    
	// Gaussian–Jordan row elimination method for calculating inverse of matrix
	// https://youtu.be/vKBNzM3V-Rc?si=qcLywp_BXTuHxQp1
    for(uint64 i = 0; i < size ; ++i)
    {
        double pivot = input_mat[i * size + i];
        for(uint64 row = 0 ; row < size ; ++row)
        {
            if(row != i)
            {
                double multiplier = input_mat[row * size + i] / pivot;
                foreach(col = 0 ... size)
                {
                    input_mat[row * size + col] = input_mat[row * size + col] - input_mat[i * size + col] * multiplier;
                    output_mat[row * size + col] = output_mat[row * size + col] - output_mat[i * size + col] * multiplier;
                }
            } 
        }
    }

    for (uint64 row = 0; row < size; ++row)
    {
        double pivot = input_mat[row * size + row];
        foreach (col = 0 ... size)
        {
            output_mat[row * size + col] = output_mat[row * size + col] / pivot;
        }
    }
}

/*****************************************************************************************************
                                         END OF MATRIX OPERATIONS
******************************************************************************************************/





void normalize(uniform double* uniform input, 
                        uniform double* uniform output,
                        uint64 start,
                        uint64 size)
{
    double total = 0;
    for(uint64 i = start ; i < start + size ; ++i)
    {
        total += input[i];
    }

    total = abs(total);
    
    for(uint64 i = start ; i < start + size ; ++i)
    {
        output[i] = input[i] / total;
    }

}


task void calculate_savitzky_golay_filter_task(const uniform double* uniform input,
                                            uniform double* uniform convolution_coefficient,
                                            uniform double* uniform convolution_coefficient_buff,
                                            uniform double* uniform output,
                                            uniform uint32 half_poly,
                                            uniform uint64 start,
                                            uniform uint64 end)
{
    foreach(i = start ... end)
    {
        uint64 left = i - half_poly;
        uint64 right = i + half_poly;

        double smoothed_value = 0;
        uint64 coefficient_index = 0;
        for (uint64 j = left; j <= right; ++j)
        {
            double coeff = convolution_coefficient[coefficient_index];
            smoothed_value += (coeff * input[j]);
            coefficient_index++;
        }

        output[i] = smoothed_value;
    }
}

export void calculate_savitzky_golay_filter(const uniform double* uniform input,
                                            uniform double* uniform convolution_coefficient,
                                            uniform double* uniform convolution_coefficient_buff,
                                            uniform double* uniform output,
                                            uniform uint64 window_size,
                                            uniform uint64 polynomial_order,
                                            uniform uint64 count)
{
    foreach (i = 0 ... polynomial_order)
    {
        convolution_coefficient_buff[i] = convolution_coefficient[i];
    }

    uniform uint32 half_poly = polynomial_order / 2;

    // front boundary
    // cannoto parallelize boundry ones because the convolution_coefficient_buff
    // should contain the normalized coefficients for that particular loop
    // so only one instance can run for the boundries
    if (programIndex == 0) 
    {
        // front
        for (uint64 i = 0; i < half_poly; ++i)
        {
            uint64 left = 0;
            uint64 right = i + half_poly;

            uint64 effective_window_size = half_poly + i + 1;
            for (uint64 j = 0; j < polynomial_order; ++j)
            {
                convolution_coefficient_buff[j] = convolution_coefficient[j];
            }

            normalize(convolution_coefficient, convolution_coefficient_buff, half_poly - i, effective_window_size);

            double smoothed_value = 0;
            uint64 coefficient_index = half_poly - i;
            for (uint64 j = left; j <= right; ++j)
            {
                double coeff = convolution_coefficient_buff[coefficient_index];
                smoothed_value += (coeff * input[j]);
                coefficient_index++;
            }

            output[i] = smoothed_value;
        }

        // back
        for (uint64 i = count - half_poly - 1; i < count; ++i)
        {
            uint64 left = i - half_poly;
            uint64 right = count - 1;

            uint64 effective_window_size = half_poly + 1 + ((count - 1) - i);
            for (uint64 j = 0; j < polynomial_order; ++j)
            {
                convolution_coefficient_buff[j] = convolution_coefficient[j];
            }

            normalize(convolution_coefficient, convolution_coefficient_buff, 0, effective_window_size);

            double smoothed_value = 0;
            uint64 coefficient_index = 0;
            for (uint64 j = left; j <= right; ++j)
            {
                double coeff = convolution_coefficient_buff[coefficient_index];
                smoothed_value += (coeff * input[j]);
                coefficient_index++;
            }

            output[i] = smoothed_value;
        }
    }

    // calculate middle body without boundaries in parallel
    foreach(i = half_poly ... count - half_poly)
    {
        uint64 left = i - half_poly;
        uint64 right = i + half_poly;

        double smoothed_value = 0;
        uint64 coefficient_index = 0;
        for (uint64 j = left; j <= right; ++j)
        {
            double coeff = convolution_coefficient[coefficient_index];
            smoothed_value += (coeff * input[j]);
            coefficient_index++;
        }

        output[i] = smoothed_value;
    }

    /*
    // Launching tasks has more over head. Even when batch size is greater than count, so launching single task, it still takes more time,
    // Around double the time than normal foreach
    uniform uint64 batch_size = 10240;
    if(count > polynomial_order)
    {
        if (programIndex == 0) 
        {
            uniform uint64 batch_count = (uint64)((count - polynomial_order) / batch_size) + 1;
            for(uniform uint64 i = 0 ; i < batch_count ; ++i)
            {
                uniform uint64 start = select(i == 0, half_poly, i * batch_size);
                uniform uint64 end = select(i == batch_count - 1, count - half_poly, (i + 1) * batch_size);

                launch calculate_savitzky_golay_filter_task(input, convolution_coefficient, convolution_coefficient_buff, output, half_poly, start, end);
            }
        }
    }
    */
}


int64 find_crossing_left(const double* input_data, uint64 at_index, uint64 count, double height, int modifier)
{
	for (int64 i = at_index; i >= 0; --i)
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

int64 find_crossing_right(const double* input_data, uint64 at_index, uint64 count, double height, int modifier)
{
	for (int64 i = at_index; i < count; ++i)
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

export void find_peaks(const uniform double* uniform input_data, 
                        uniform double* uniform prominences, 
                        uniform double* uniform output_peaks_indices, 
                        uniform int8* uniform peaks_and_trough_tmp_buff,
                        uniform uint64* uniform peaks_count, 
                        uniform uint64 min_distance, 
                        uniform uint64 min_width, 
                        uniform double relative_height, 
                        uniform int input_modifier,
                        uniform uint64 count)
{
    peaks_and_trough_tmp_buff[0] = 0;
    peaks_and_trough_tmp_buff[count - 1] = 0;

    foreach( i = 1 ... count)
    {
        peaks_and_trough_tmp_buff[i] = 0;

        if (input_data[i] * input_modifier > input_data[i - 1] * input_modifier && 
            input_data[i] * input_modifier > input_data[i + 1] * input_modifier)
        {
            double peak_height = input_data[i] * input_modifier;

            double prominence = prominences[i];
            double height_at_width = peak_height - (prominence * relative_height);

            int64 left_index = find_crossing_left(input_data, i, count, height_at_width, input_modifier);
            int64 right_index = find_crossing_right(input_data, i, count, height_at_width, input_modifier);

            int64 width = right_index - left_index;

            if(width >= min_width && left_index != i && right_index != i)
            {
                peaks_and_trough_tmp_buff[i] = 1;
            }
        }
    }
    
    if(programIndex == 0)
    {
        *peaks_count = 0;
        for(uniform int64 i = 0 ; i < count ; ++i)
        {
            // find next peak
            uniform int64 peak = i;
            for(uniform int64 j = peak ; j < count ; ++j)
            {
                peak = j;
                if(peaks_and_trough_tmp_buff[j] > 0)
                {
                    break;
                }
            }

            // find the largest peak in the distance range
            i = peak;
            for(uniform int64 j = i ; j < i + min_distance && j < count ; ++j)
            {
                if(peaks_and_trough_tmp_buff[j] > 0 &&
                    input_data[j] * input_modifier > input_data[peak] * input_modifier)
                {
                    peak = j;
                }
            }

            if(peak != count - 1)
            {
                output_peaks_indices[*peaks_count] = peak;
                (*peaks_count)++;
            }
            

            i = peak + min_distance;
        }

        /*for(uniform int64 i = 0 ; i < count ; ++i)
        {
            if(peaks_and_trough_tmp_buff[i] > 0)
            {
                output_peaks_indices[*peaks_count] = i;
                (*peaks_count)++;
            }
        }*/
    }
}


export void calculate_trend_analysis_debug( const uniform double* uniform highs,
                                            const uniform double* uniform lows,
                                            const uniform double* uniform closes,
                                            uniform double* uniform a_buff,
                                            uniform double* uniform at_buff,
                                            uniform double* uniform ata_buff,
                                            uniform double* uniform ata_inv_buff,
                                            uniform double* uniform ata_inv_tmp_buff,
                                            uniform double* uniform ata_inv_at_buff,
                                            uniform double* uniform convolution_coefficient,
                                            uniform double* uniform convolution_coefficient_buff,
                                            uniform double* uniform savitzky_golay_output,
                                            uniform double* uniform atr_output,
                                            uniform double* uniform peaks_output,
                                            uniform double* uniform troughs_output,
                                            uniform int8* uniform peaks_and_trough_tmp_buff,
                                            uniform double* uniform trend_output,
                                            uniform uint64* uniform peaks_count,
                                            uniform uint64* uniform troughs_count,
                                            uniform uint64 window_size,
                                            uniform uint64 polynomial_order,
                                            uniform uint64 distance_btw_peaks,
                                            uniform uint64 width_for_peaks,
                                            uniform double relative_height_for_peaks,
                                            uniform uint64 history_length_for_trend_detection,
                                            uniform uint64 count)
{
    calculate_vandermonde_matrix(a_buff, polynomial_order, window_size);
    transpose_matrix(a_buff, at_buff, polynomial_order, window_size);
    multiply_matrix(at_buff, window_size, polynomial_order, a_buff, polynomial_order, window_size, ata_buff);
    inverse_matrix(ata_buff, ata_inv_buff, window_size);
    multiply_matrix(ata_inv_buff, window_size, window_size, at_buff, window_size, polynomial_order, ata_inv_at_buff);

    // first row will become the convolution coefficient
    foreach (i = 0 ... polynomial_order)
    {
        convolution_coefficient[i] = ata_inv_at_buff[i];
    }

    calculate_savitzky_golay_filter(closes, convolution_coefficient, convolution_coefficient_buff, savitzky_golay_output, window_size, polynomial_order, count);

    calculate_atr(highs, lows, closes, atr_output, 30, count);

    find_peaks(savitzky_golay_output, atr_output, peaks_output, peaks_and_trough_tmp_buff, peaks_count, distance_btw_peaks, width_for_peaks, relative_height_for_peaks, 1, count);
    find_peaks(savitzky_golay_output, atr_output, troughs_output, peaks_and_trough_tmp_buff, troughs_count, distance_btw_peaks, width_for_peaks, relative_height_for_peaks, -1, count);

    foreach (i = 0 ... count)
    {
        uint64 peak_index = 0;
        for (int64 j = *peaks_count - 1; j >= 0; --j)
        {
            if (peaks_output[j] < i)
            {
                peak_index = j;
                break;
            }
        }

        uint64 trough_index = 0;
        for (int64 j = *troughs_count - 1; j >= 0; --j)
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
            int64 peaks_up_count = 0;
            int64 troughs_up_count = 0;
            int64 peaks_down_count = 0;
            int64 troughs_down_count = 0;
            int64 perfect_peaks_up_count = 0;
            int64 perfect_troughs_up_count = 0;
            int64 perfect_peaks_down_count = 0;
            int64 perfect_troughs_down_count = 0;

            double threshold = 0.01;
            for (int64 j = peak_index; j > peak_index - history_length_for_trend_detection && j >= 0; --j)
            {
                int64 curr_peak_index = (int64)peaks_output[j];
                int64 prev_peak_index = (int64)peaks_output[j - 1];
                if (abs(savitzky_golay_output[curr_peak_index] - savitzky_golay_output[prev_peak_index]) < savitzky_golay_output[prev_peak_index] * threshold ||
                    savitzky_golay_output[curr_peak_index] > savitzky_golay_output[prev_peak_index])
                {
                    ++peaks_up_count;
                }
                if(savitzky_golay_output[curr_peak_index] > savitzky_golay_output[prev_peak_index])
                {
                    ++perfect_peaks_up_count;
                }
                if (abs(savitzky_golay_output[curr_peak_index] - savitzky_golay_output[prev_peak_index]) < savitzky_golay_output[prev_peak_index] * threshold ||
                    savitzky_golay_output[curr_peak_index] < savitzky_golay_output[prev_peak_index])
                {
                    ++peaks_down_count;
                }
                if(savitzky_golay_output[curr_peak_index] < savitzky_golay_output[prev_peak_index])
                {
                    ++perfect_peaks_down_count;
                }
            }
            for (int j = trough_index; j > trough_index - history_length_for_trend_detection && j >= 0; --j)
            {
                int64 curr_trough_index = (int64)troughs_output[j];
                int64 prev_trough_index = (int64)troughs_output[j - 1];
                if (abs(savitzky_golay_output[curr_trough_index] - savitzky_golay_output[prev_trough_index]) < savitzky_golay_output[prev_trough_index] * threshold ||
                    savitzky_golay_output[curr_trough_index] > savitzky_golay_output[prev_trough_index])
                {
                    ++troughs_up_count;
                }
                if(savitzky_golay_output[curr_trough_index] > savitzky_golay_output[prev_trough_index])
                {
                    ++perfect_troughs_up_count;
                }
                if (abs(savitzky_golay_output[curr_trough_index] - savitzky_golay_output[prev_trough_index]) < savitzky_golay_output[prev_trough_index] * threshold ||
                    savitzky_golay_output[curr_trough_index] < savitzky_golay_output[prev_trough_index])
                {
                    ++troughs_down_count;
                }
                if(savitzky_golay_output[curr_trough_index] < savitzky_golay_output[prev_trough_index])
                {
                    ++perfect_troughs_down_count;
                }
            }

            if(peaks_up_count == history_length_for_trend_detection && troughs_up_count == history_length_for_trend_detection &&
                peaks_up_count == history_length_for_trend_detection && troughs_up_count == history_length_for_trend_detection)
            {
                // Ambiguous results when using threshold.
                // Use perfect values
                if (perfect_peaks_up_count == history_length_for_trend_detection && perfect_troughs_up_count == history_length_for_trend_detection)
                {
                    trend = 1.0f;
                }
                else if (perfect_peaks_down_count == history_length_for_trend_detection && perfect_troughs_down_count == history_length_for_trend_detection)
                {
                    trend = -1.0f;
                }
            }
            else
            {
                if (peaks_up_count == history_length_for_trend_detection && troughs_up_count == history_length_for_trend_detection)
                {
                    trend = 1.0f;
                }
                else if (peaks_down_count == history_length_for_trend_detection && troughs_down_count == history_length_for_trend_detection)
                {
                    trend = -1.0f;
                }
            }
            
        }

        trend_output[i] = trend;
    }
}