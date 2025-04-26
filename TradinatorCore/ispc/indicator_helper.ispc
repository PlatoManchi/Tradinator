// Command to compile and create object and header files
// ispc indicator_helper.ispc -o indicator_helper_ispc.o -h indicator_helper_ispc.h --target=avx2,avx512skx-x64,sse2


// ---------------------------------- SMA -------------------------------------------
export void calculate_sma(uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int64 data_size, 
                            uniform unsigned int64 window_size) 
{
    
    foreach (i = 0 ... data_size) 
    {
        double sum = 0.0f;
        
        int64 essential_window_size = select(i + window_size < data_size, window_size, data_size - i);

        for (int j = 0; j < essential_window_size; ++j) 
        {
            int idx = i + j;
            sum += input[idx];
        }

        output[i] = sum / essential_window_size;
    }
}


// ---------------------------------- WMA -------------------------------------------
export void calculate_wma(uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int64 data_size, 
                            uniform unsigned int64 window_size) 
{
    
    foreach (i = 0 ... data_size) 
    {
        double sum = 0.0f;
        int count = 0;

        int64 essential_window_size = select(i + window_size < data_size, window_size, data_size - i);

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
                            uniform unsigned int64 data_size, 
                            uniform unsigned int64 window_size) 
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



// ---------------------------------- Bollinger Band -------------------------------------------
export void calculate_bollinger_band(uniform double* uniform input,
                                        uniform double* uniform top_output,
                                        uniform double* uniform sma_output,
                                        uniform double* uniform bottom_output,
                                        uniform unsigned int64 data_size,
                                        uniform unsigned int64 window_size,
                                        uniform double standard_deviation_multiplier)
{
    calculate_sma(input, sma_output, data_size, window_size);

    foreach (i = 0 ... data_size) 
    {
        int64 essential_window_size = select(i + window_size < data_size, window_size, data_size - i);

        double mean = sma_output[i];
        double cumulative_deviation_squared = 0;

        for (int j = 0; j < essential_window_size; ++j) 
        {
            int idx = i + j;
            double deviation = input[idx] - mean;
            cumulative_deviation_squared += (deviation * deviation);
        }

        double variance = cumulative_deviation_squared / essential_window_size;
        double standard_deviation = sqrt(variance);

        top_output[i] = mean + standard_deviation_multiplier * standard_deviation;
        bottom_output[i] = mean - standard_deviation_multiplier * standard_deviation;
    }
}


// ---------------------------------- ROC -------------------------------------------
export void calculate_roc(uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int64 data_size, 
                            uniform unsigned int64 window_size) 
{
    foreach (i = 0 ... data_size) 
    {
        int64 essential_window_size = select(i + window_size < data_size, window_size, data_size - i);

        double current = input[i];
        double compare_with = select(i + essential_window_size < data_size, input[i + essential_window_size], input[data_size - 1]);

        output[i] = ((current - compare_with) / compare_with) * 100.0;
    }
}


// ---------------------------------- RSI -------------------------------------------
export void calculate_rsi(uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int64 data_size, 
                            uniform unsigned int64 window_size) 
{
    uniform double zero = 0.0;
    foreach (i = 0 ... data_size - 1)
    {
        int64 essential_window_size = select(i + window_size < data_size - 1, window_size, data_size - i - 1);

        double cumulative_gain = 0;
        double cumulative_loss = 0;

        for (int j = 0; j < essential_window_size; ++j) 
        {
            int idx = i + j;
            
            double diff = input[idx] - input[idx + 1];

            cumulative_gain += select(diff > 0.0, diff, zero);
            cumulative_loss += select(diff < 0.0, diff, zero);
        }

        double relative_strength = cumulative_gain / abs(cumulative_loss);
        output[i] = 100.0 - 100.0 / (1 + relative_strength);
    }

    output[data_size - 1] = 0;
}