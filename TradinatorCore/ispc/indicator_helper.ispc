// Command to compile and create object and header files
// ispc indicator_helper.ispc -o indicator_helper_ispc.o -h indicator_helper_ispc.h --target=avx2,avx512skx-x64,sse2


// ---------------------------------- SMA -------------------------------------------
export void calculate_sma(const uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform uint64 window_size,
                            uniform uint64 data_size) 
{
    output[0] = input[0];

    uniform uint64 zero = 0;
    foreach (i = 1 ... data_size) 
    {
        int32 start = select(window_size > data_size, zero, select(i < window_size, zero, (i - window_size)));

        double sum = 0.0f;
        for (uint64 j = start; j <= i; ++j) 
        {
            sum += input[j];
        }

        output[i] = sum / (i - start);
    }
}


// ---------------------------------- WMA -------------------------------------------
export void calculate_wma(const uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform uint64 window_size,
                            uniform uint64 data_size) 
{
    output[0] = input[0];
    uniform uint64 zero = 0;
    foreach (i = 1 ... data_size) 
    {
        int32 start = select(window_size > data_size, zero, select(i < window_size, zero, i - window_size));

        double sum = 0.0f;
        int32 count = 0;
        for (uint64 j = start; j <= i; ++j) 
        {
            count = count + 1;
            sum += (count * input[j]);
        }

        int32 sum_count = (count * (count + 1)) / 2; // Sum of n numbers is N(N+1)/2
        output[i] = sum / sum_count;
    }
}


// ---------------------------------- EMA -------------------------------------------
export void calculate_ema(const uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform uint64 window_size, 
                            uniform uint64 data_size) 
{
    // since values are dependent on previous value cant do parallel execution
    // need to loop in scalar way    
    uniform double factor = 2.0 / ((double)window_size + 1.0);
    uniform double prev_ema = input[0];
    output[0] = prev_ema;
    
    for (uniform uint64 i = 1 ; i < data_size ; ++i) 
    {
        prev_ema = input[i] * factor + (1.0 - factor) * prev_ema;
        output[i] = prev_ema;
    }
}



// ---------------------------------- Bollinger Band -------------------------------------------
export void calculate_bollinger_band(const uniform double* uniform input,
                                        uniform double* uniform top_output,
                                        uniform double* uniform sma_output,
                                        uniform double* uniform bottom_output,
                                        uniform uint64 window_size,
                                        uniform double standard_deviation_multiplier,
                                        uniform uint64 data_size)
{
    calculate_sma(input, sma_output, window_size, data_size);

    top_output[0] = input[0];
    sma_output[0] = input[0];
    bottom_output[0] = input[0];

    uniform uint64 zero = 0;
    foreach (i = 1 ... data_size) 
    {
        int32 start = select(window_size > data_size, zero, select(i < window_size, zero, i - window_size));

        double mean = sma_output[i];
        double cumulative_deviation_squared = 0;

        for (uint64 j = start; j <= i; ++j)
        {
            double deviation = input[j] - mean;
            cumulative_deviation_squared += (deviation * deviation);
        }

        double variance = cumulative_deviation_squared / (i - start);
        double standard_deviation = sqrt(variance);

        top_output[i] = mean + standard_deviation_multiplier * standard_deviation;
        bottom_output[i] = mean - standard_deviation_multiplier * standard_deviation;
    }
}


// ---------------------------------- ROC -------------------------------------------
export void calculate_roc(const uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int64 window_size, 
                            uniform unsigned int64 data_size) 
{
    uniform uint64 zero = 0;
    foreach (i = 0 ... data_size) 
    {
        uint64 compare_with_index = select(window_size > data_size, zero, select(i < window_size, zero, i - window_size));

        double current = input[i];
        double compare_with = input[compare_with_index];

        if(compare_with != 0.0)
        {
            output[i] = ((current - compare_with) / compare_with) * 100.0;
        }
        else
        {
            output[i] = 0.0;
        }
    }
}


// ---------------------------------- RSI -------------------------------------------
export void calculate_rsi(const uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform unsigned int64 data_size, 
                            uniform unsigned int64 window_size) 
{
    output[0] = 0;

    uniform double dbl_zero = 0.0;
    uniform uint64 int_zero = 0;
    foreach (i = 1 ... data_size)
    {
        uint64 start = select(window_size > data_size, int_zero, select(i < window_size, int_zero, i - window_size));

        double cumulative_gain = 0;
        double cumulative_loss = 0;

        for (uint64 j = start; j <= i; ++j) 
        {
            double diff = input[j] - input[j - 1];

            cumulative_gain += select(diff > 0.0, diff, dbl_zero);
            cumulative_loss += select(diff < 0.0, diff, dbl_zero);
        }

        double relative_strength = cumulative_gain / abs(cumulative_loss);
        output[i] = 100.0 - 100.0 / (1 + relative_strength);
    }    
}


// ---------------------------------- MACD -------------------------------------------
export void calculate_mcda(const uniform double* uniform input, 
                            uniform double* uniform ema_period_1_buffer,
                            uniform double* uniform ema_period_2_buffer,
                            uniform double* uniform mcda_output, 
                            uniform double* uniform signal_output, 
                            uniform double* uniform histogram_output,  
                            uniform unsigned int64 period_1,
                            uniform unsigned int64 period_2,
                            uniform unsigned int64 signal_period,
                            uniform unsigned int64 data_size) 
{
    calculate_ema(input, ema_period_1_buffer, data_size, period_1);
    calculate_ema(input, ema_period_2_buffer, data_size, period_2);

    foreach(i = 0 ... data_size)
    {
        mcda_output[i] = ema_period_1_buffer[i] - ema_period_2_buffer[i];
    }

    calculate_ema(mcda_output, signal_output, data_size, signal_period);

    foreach(i = 0 ... data_size)
    {
        histogram_output[i] = mcda_output[i] - signal_output[i];
    }
}


// ---------------------------------- ATR -------------------------------------------
export void calculate_atr(const uniform double* uniform input_high, 
                            const uniform double* uniform input_low,
                            const uniform double* uniform input_close,  
                            uniform double* uniform true_range_buff, 
                            uniform double* uniform output, 
                            uniform unsigned int64 data_size, 
                            uniform unsigned int64 window_size) 
{
    
    foreach(i = 0 ... data_size)
    {
        double diff_1 = input_high[i] - input_low[i];
        double diff_2 = 0;
        double diff_3 = 0;

        if(i < data_size - 1)
        {
            diff_2 = input_high[i] - input_close[i+1];
            diff_3 = input_low[i] - input_close[i+1];
        }
        true_range_buff[i] = max(max(diff_1, diff_2), diff_3);
    }

    calculate_sma(true_range_buff, output, data_size, window_size);
}