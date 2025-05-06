// Command to compile and create object and header files
// ispc matrix_helper.ispc -o matrix_helper_ispc.o -h matrix_helper_ispc.h --target=avx2,avx512skx-x64,sse2



// width has to be odd
export void calculate_vandermonde_matrix(uniform double* uniform output, 
                                uniform uint64 width,
                                uniform uint64 window_size)
{
    uniform int64 half_poly = width/2;

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




void normalize(uniform double* uniform input, 
                        uniform double* uniform output,
                        uint64 start,
                        uint64 size)
{
    /*double total = 0;
    foreach (i = start ... size) 
    {
        total += input[i];
    }

    total = reduce_add(total);

    total = abs(total);

    foreach (i = start ... size)
    {
        output[i] = input[i] / total;
    }*/
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

export void calculate_trend_analysis_debug(uniform double* uniform input,
                                            uniform uint64 count,
                                            uniform double* uniform a_buff,
                                            uniform double* uniform at_buff,
                                            uniform double* uniform ata_buff,
                                            uniform double* uniform ata_inv_buff,
                                            uniform double* uniform ata_inv_at_buff,
                                            uniform double* uniform convolution_coefficient,
                                            uniform double* uniform convolution_coefficient_buff,
                                            uniform uint64 width,
                                            uniform uint64 window_size,
                                            uniform double* uniform output)
{
    calculate_vandermonde_matrix(a_buff, width, window_size);
    transpose_matrix(a_buff, at_buff, width, window_size);
    multiply_matrix(at_buff, window_size, width, a_buff, width, window_size, ata_buff);
    inverse_matrix(ata_buff, ata_inv_buff, window_size);
    multiply_matrix(ata_inv_buff, window_size, window_size, at_buff, window_size, width, ata_inv_at_buff);

    foreach (i = 0 ... width)
    {
        convolution_coefficient[i] = ata_inv_at_buff[i];
        convolution_coefficient_buff[i] = ata_inv_at_buff[i];
    }

    uniform uint32 half_poly = width / 2;

    // front boundary
    // cannoto parallelize boundry ones because the convolution_coefficient_buff
    // should contain the normalized coefficients for that particular loop
    // so only one instance can run for the boundries
    if (programIndex == 0) {
        // front
        for (uint64 i = 0; i < half_poly; ++i)
        {
            uint64 left = 0;
            uint64 right = i + half_poly;

            uint64 effective_window_size = half_poly + i + 1;
            for (uint64 j = 0; j < width; ++j)
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
            for (uint64 j = 0; j < width; ++j)
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

    // calculate middle body without boundaries
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