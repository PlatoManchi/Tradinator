// Command to compile and create object and header files
// ispc indicator_helper.ispc -o indicator_helper_ispc.o -h indicator_helper_ispc.h --target=avx2,avx512skx-x64,sse2


// ---------------------------------- SMA -------------------------------------------
export void calculate_sma(const uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform uint64 window_size,
                            uniform uint64 data_size) 
{
    output[0] = input[0];

    uniform int64 zero = 0;
    foreach (i = 1 ... data_size) 
    {
        int64 start = select(window_size > data_size, zero, select(i + 1 < window_size, zero, (i + 1 - window_size)));

        double sum = 0.0f;
        for (int64 j = start; j <= i; ++j) 
        {
            sum += input[j];
        }

        output[i] = sum / (i + 1 - start);
    }
}


// ---------------------------------- WMA -------------------------------------------
export void calculate_wma(const uniform double* uniform input, 
                            uniform double* uniform output, 
                            uniform uint64 window_size,
                            uniform uint64 data_size) 
{
    output[0] = input[0];
    uniform int64 zero = 0;
    foreach (i = 1 ... data_size) 
    {
        int64 start = select(window_size > data_size, zero, select(i + 1 < window_size, zero, (i + 1 - window_size)));

        double sum = 0.0f;
        int64 count = 0;
        for (int64 j = start; j <= i; ++j) 
        {
            count = count + 1;
            sum += (count * input[j]);
        }

        int64 sum_count = (count * (count + 1)) / 2; // Sum of n numbers is N(N+1)/2
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
    if (programIndex == 0) 
    { 
        uniform double factor = 2.0 / ((double)window_size + 1.0);
        uniform double prev_ema = input[0];
        output[0] = prev_ema;
        
        for (uniform int64 i = 1 ; i < data_size ; ++i) 
        {
            prev_ema = input[i] * factor + (1.0 - factor) * prev_ema;
            output[i] = prev_ema;
        }
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

    uniform int64 zero = 0;
    foreach (i = 1 ... data_size) 
    {
        int64 start = select(window_size > data_size, zero, select(i + 1 < window_size, zero, (i + 1 - window_size)));

        double mean = sma_output[i];
        double cumulative_deviation_squared = 0;

        for (int64 j = start; j <= i; ++j)
        {
            double deviation = input[j] - mean;
            cumulative_deviation_squared += (deviation * deviation);
        }

        double variance = cumulative_deviation_squared / (i + 1 - start);
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
    uniform int64 zero = 0;
    foreach (i = 0 ... data_size) 
    {
        int64 compare_with_index = select(window_size > data_size, zero, select(i < window_size, zero, i - window_size));

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
                            uniform uint64 window_size, 
                            uniform uint64 data_size) 
{
    uniform double VERY_LARGE = 9999999999.0;
    uniform double VERY_SMALL = 0.0000000001;
    if(programIndex == 0)
    {
        output[0] = 0;

        uniform double prev_gain_avg = 0;
        uniform double prev_loss_avg = 0;
        for (uniform int64 i = 1; i < window_size + 1; ++i)
        {
            uniform double diff = input[i] - input[i - 1];
            prev_gain_avg += (diff > 0.0 ? diff : 0.0);
            prev_loss_avg -= (diff < 0.0 ? diff : 0.0);

            output[i] = 0;
        }

        prev_gain_avg = prev_gain_avg / window_size;
        prev_loss_avg = prev_loss_avg / window_size;

        for (uniform int64 i = window_size + 1; i < data_size; ++i)
        {
            uniform double diff = input[i] - input[i - 1];

            uniform double gain = (diff > 0.0 ? diff : 0.0);
            uniform double loss = -(diff < 0.0 ? diff : 0.0);

            prev_gain_avg = (prev_gain_avg * (window_size - 1) + gain) / window_size;
            prev_loss_avg = (prev_loss_avg * (window_size - 1) + loss) / window_size;

            uniform double relative_strength = select (prev_loss_avg < VERY_SMALL, VERY_LARGE, prev_gain_avg / prev_loss_avg);
            output[i] = 100.0 - 100.0 / (1 + relative_strength);
        }  
    }
}


// ---------------------------------- MACD -------------------------------------------
export void calculate_macd(const uniform double* uniform input, 
                            uniform double* uniform ema_period_1_buffer,
                            uniform double* uniform ema_period_2_buffer,
                            uniform double* uniform mcda_output, 
                            uniform double* uniform signal_output, 
                            uniform double* uniform histogram_output,  
                            uniform uint64 period_1,
                            uniform uint64 period_2,
                            uniform uint64 signal_period,
                            uniform uint64 data_size) 
{
    calculate_ema(input, ema_period_1_buffer, period_1, data_size);
    calculate_ema(input, ema_period_2_buffer, period_2, data_size);

    foreach(i = 0 ... data_size)
    {
        mcda_output[i] = ema_period_1_buffer[i] - ema_period_2_buffer[i];
    }

    calculate_ema(mcda_output, signal_output, signal_period, data_size);

    foreach(i = 0 ... data_size)
    {
        histogram_output[i] = mcda_output[i] - signal_output[i];
    }
}


// ---------------------------------- OBV -------------------------------------------
export void calculate_obv(const uniform double* uniform closes, 
                            const uniform uint64* uniform volumes, 
                            uniform double* uniform output, 
                            uniform uint64 data_size) 
{
    if (programIndex == 0) 
    {
        output[0] = volumes[0];

        for (uniform int64 i = 1 ; i < data_size ; ++i) 
        {
            uniform double factor = select(closes[i] > closes[i - 1], 1.0, -1.0);
            output[i] = output[i - 1] + factor * volumes[i];
        }
    }
}


// ---------------------------------- ATR -------------------------------------------
export void calculate_atr(const uniform double* uniform highs, 
                            const uniform double* uniform lows,
                            const uniform double* uniform closes,
                            uniform double* uniform output, 
                            uniform uint64 window_size, 
                            uniform uint64 data_size) 
{
    if (programIndex == 0) 
    {
        output[0] = highs[0] - lows[0];
        // for first window_size items there is no previous ATR,
        // so assign 0 to them and calculate average true range for that period
        uniform double true_range_sum = output[0];
        for (uniform int64 i = 1; i < window_size; ++i)
        {
            uniform double diff_1 = highs[i] - lows[i];
            uniform double diff_2 = highs[i] - closes[i - 1];
            uniform double diff_3 = lows[i] - closes[i - 1];

            true_range_sum += max(max(diff_1, diff_2), diff_3);

            output[i] = true_range_sum / i;
        }
        for (uniform int64 i = window_size; i < data_size; ++i)
        //foreach (i = window_size ... data_size)
        {
            uniform double diff_1 = highs[i] - lows[i];
            uniform double diff_2 = highs[i] - closes[i - 1];
            uniform double diff_3 = lows[i] - closes[i - 1];

            uniform double true_range = max(max(diff_1, diff_2), diff_3);

            output[i] = (output[i - 1] * (window_size - 1) + true_range) / window_size;
        }
    }
}