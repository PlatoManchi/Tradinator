// Command to compile and create object and header files
// ispc utils.ispc -o utils_ispc.o -h utils_ispc.h --target=avx2,avx512skx-x64,sse2

#include "indicator_helper.ispc"
#include "matrix_helper.ispc"


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
                                            uniform double* uniform trend_output,
                                            uniform uint64* uniform peaks_count,
                                            uniform uint64* uniform troughs_count,
                                            uniform uint64 window_size,
                                            uniform uint64 polynomial_order,
                                            uniform uint64 distance_btw_peaks,
                                            uniform uint64 width_for_peaks,
                                            uniform double relative_height_for_peaks,
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
}