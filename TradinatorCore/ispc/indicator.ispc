// Command to compile and create object and header files
// ispc indicator.ispc -o indicator_ispc.o -h indicator_ispc.h --target=avx2,avx512skx-x64,sse2


// ---------------------------------- SMA -------------------------------------------
export void calculate_sma(uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int data_size, 
                            uniform unsigned int window_size) 
{
    
    foreach (i = 0 ... data_size) 
    {
        double sum = 0.0f;
        int count = 0;

        int essential_window_size = i + window_size < data_size ? window_size : data_size - i;

        for (int j = 0; j <= essential_window_size; ++j) 
        {
            int idx = i + j;
            
            sum += input[idx];
            count += 1;
        }

        output[i] = sum / count;
    }
}


// ---------------------------------- WMA -------------------------------------------
export void calculate_wma(uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int data_size, 
                            uniform unsigned int window_size) 
{
    
    foreach (i = 0 ... data_size) 
    {
        double sum = 0.0f;
        int count = 0;

        int32 essential_window_size = i + window_size < data_size ? window_size : data_size - i;

        for (int j = 0; j < essential_window_size; ++j) 
        {
            int idx = i + j;
            
            sum += (essential_window_size - j) * input[idx];
            count += (essential_window_size - j);
        }

        output[i] = sum / count;
    }
}


// ---------------------------------- EMA -------------------------------------------
// Because of the scalar way the loop runs, its faster and cheaper to run this in vanilla C++
// than wasting time to launch a SPMD
export void calculate_ema(uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int data_size, 
                            uniform unsigned int window_size) 
{
    output[0] = input[0];
    uniform double factor = 2.0 / ((double)window_size + 1.0);

    // since values are dependent on previous value cant do parallel execution
    // need to loop in scalar way
    for (uniform unsigned int i = 1 ; i < data_size ; ++i) 
    {
        output[i] = input[i] * factor + output[i - 1] * (1.0 - factor);
    }
}