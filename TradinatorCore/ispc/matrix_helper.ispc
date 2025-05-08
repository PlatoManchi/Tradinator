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


