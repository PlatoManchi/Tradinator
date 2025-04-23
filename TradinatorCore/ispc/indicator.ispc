// ispc indicator.ispc -o indicator_ispc.o -h indicator_ispc.h --target=avx2,avx512skx-x64,sse2

export void calculate_sma(uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int data_size, 
                            uniform unsigned int window_size) 
{
    
    foreach (i = 0 ... data_size) 
    {
        double sum = 0.0f;
        int count = 0;

        for (int j = 0; j <= window_size; ++j) 
        {
            int idx = i + j;
            if (idx >= 0 && idx < data_size) 
            {
                sum += input[idx];
                count += 1;
            }
        }

        output[i] = sum / count;
    }
}