// encoding UTF-8
// 水温检测源程序
#include "temp_of_water.h"

// 实物是检测NTC热敏电阻的分压

#if TEMP_OF_WATER_SCAN_ENABLE
// 水温检测函数，如果水温过高，会发送水温报警，水温恢复正常时，才发送解除水温报警
void temp_of_water_scan(void)
{
    static u32 over_heat_accumulate_cnt = 0;    // 过热累计计数
    static u32 cooling_down_accumulate_cnt = 0; // 冷却累计计数
    static u32 temp_of_water_update_time_cnt = 0;

    adc_sel_pin(ADC_PIN_TEMP_OF_WATER);
    adc_val = adc_getval(); //
    temp_of_water_update_time_cnt += ONE_CYCLE_TIME_MS;

    // 如果处于水温报警
    if (adc_val <= TEMP_OF_WATER_CANCEL_WARNING_AD_VAL)
    {
        // 如果检测到水温 低于 解除水温报警的阈值
        cooling_down_accumulate_cnt++;
        // 清除水温过热计数
        over_heat_accumulate_cnt = 0;
    }
    else
    {
        cooling_down_accumulate_cnt = 0;
    }

    if (cooling_down_accumulate_cnt >= (TEMP_OF_WATER_ACCUMULATE_TIEM_MS / ONE_CYCLE_TIME_MS))
    {
        // 如果解除水温报警的计数大于 (水温检测的累计时间 / 一轮主循环的时间)，
        // 即，检测到可以解除水温报警的时间 大于 水温检测的累计时间
        cooling_down_accumulate_cnt = 0; // 清除计数
        fun_info.flag_is_in_water_temp_warning = 0;
        flag_set_temp_of_water_warning = 1; // 发送解除水温报警的信息
    }

    // 如果不处于水温报警
    if (adc_val >= TEMP_OF_WATER_WARNING_AD_VAL)
    {
        over_heat_accumulate_cnt++;
        // 清除解除水温报警的计数
        cooling_down_accumulate_cnt = 0;
    }
    else
    {
        // 有一次没有检测到水温过热，清除水温过热计数
        over_heat_accumulate_cnt = 0;
    }

    if (over_heat_accumulate_cnt >= (TEMP_OF_WATER_ACCUMULATE_TIEM_MS / ONE_CYCLE_TIME_MS))
    {
        // 如果水温报警的计数大于 (水温检测的累计时间 / 一轮主循环的时间)，
        // 即，检测到水温报警的时间 大于 水温检测的累计时间
        over_heat_accumulate_cnt = 0; // 清除计数
        fun_info.flag_is_in_water_temp_warning = 1;
        flag_set_temp_of_water_warning = 1; // 发送水温报警的信息
    }

    if (temp_of_water_update_time_cnt >= TEMP_OF_WATER_UPDATE_TIME_MS)
    {
        // 如果到了发送水温报警/发送解除水温报警的时间，发送一次当前的状态
        temp_of_water_update_time_cnt = 0;
        flag_set_temp_of_water_warning = 1; //

        // P20 = ~P20; // 测试发现,TEMP_OF_WATER_UPDATE_TIME_MS == 1000时，每2.28s发送一次状态
    }
}

#endif
