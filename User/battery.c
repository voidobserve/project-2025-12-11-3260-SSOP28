#include "battery.h"

#if BATTERY_SCAN_ENABLE

volatile u16 battery_scan_time_cnt; // 电池扫描时间计时（在定时器中累加）

/**
 * @brief 将ad值转换为对应的电压值
 *
 * @param arg_adc_val 采集到的ad值
 * @return u8 计算好的电压值，单位0.1V
 */
u16 conver_adc_val_to_voltage(u16 arg_adc_val)
{
    /*
        采集到的ad值范围：0~4095
        ad值对应的电压： 0 ~ MAX_VOLTAGE_OF_BATTERY
        那么 每单位ad值对应 MAX_VOLTAGE_OF_BATTERY / 4096
    */
    // return (u32)arg_adc_val * MAX_VOLTAGE_OF_BATTERY / 4096;

    // 使用2.4V参考电压，12位精度
    // 电池电压是 1/11 分压：
    return (u32)arg_adc_val * 240 * 11 / 10 / 4096;
}

// 将电池电压转换为对应的百分比
// voltage： 0~255 ， 对应0~25.5V
// 例如 voltage == 140，对应14.0V
u8 conver_voltage_of_battery_to_percentage(u8 voltage)
{
    u8 tmp;

    // 客户要求，如果检测到大于15V，就认为满电
    if (voltage >= 150)
    {
        tmp = 100;
    }
    else
    {
        // 如果电压小于15V，根据公式来得到百分比
        // 用电池电压voltage除以MAX_VOLTAGE_OF_BATTERY，得到占比，再乘以100，得到百分比
        tmp = (u16)voltage * 100 / MAX_VOLTAGE_OF_BATTERY;
    }

    return tmp;
}

void battery_scan(void)
{
    u16 voltage_of_battery = 0;       // 存放电池电压
    u8 cur_percentage_of_battery = 0; // 存放当前电池电量百分比

    static volatile u32 battery_scan_cnt; // 记录电池电压扫描次数
    static volatile u32 battery_val;      // 累加每次采集到的ad值，到了电池扫描时间时，直接求平均值

    static bit flag_is_power_on_first = 1; // 是否第一次上电

#if 1
    adc_sel_pin(ADC_PIN_BATTERY);
    battery_val += adc_getval(); // 可能要防止计数溢出
    battery_scan_cnt++;          // 上面采集到一次ad值之后，这里加一表示采集了一次

    if (flag_is_power_on_first)
    {
        // 第一次上电
        battery_val /= battery_scan_cnt; // 取平均数
        voltage_of_battery = conver_adc_val_to_voltage(battery_val);
        cur_percentage_of_battery = conver_voltage_of_battery_to_percentage(voltage_of_battery);
        battery_val = 0;           // 清空数值
        battery_scan_cnt = 0;      // 清空计数值
        battery_scan_time_cnt = 0; // 清空时间计数值

        fun_info.battery = cur_percentage_of_battery;
        fun_info.voltage_of_battery = voltage_of_battery;

        // printf("cur voltage of battery : %bu\n", voltage_of_battery);
        // printf("cur percent of battery : %bu\n", cur_percentage_of_battery);

        flag_get_voltage_of_battery = 1;
        flag_get_battery = 1;

        flag_is_power_on_first = 0;
    }
    else
    {
        // 不是第一次上电

        if (battery_scan_time_cnt >= BATTERY_SCAN_UPDATE_TIME_MS) // 如果到了电池数据的更新时间（更新/发送电池数据的时间）
        {
            battery_val /= battery_scan_cnt; // 取平均数
            voltage_of_battery = conver_adc_val_to_voltage(battery_val);
            cur_percentage_of_battery = conver_voltage_of_battery_to_percentage(voltage_of_battery);
            battery_val = 0;           // 清空数值
            battery_scan_cnt = 0;      // 清空计数值
            battery_scan_time_cnt = 0; // 清空时间计数值

            fun_info.battery = cur_percentage_of_battery;
            fun_info.voltage_of_battery = voltage_of_battery;

            // printf("cur voltage of battery : %bu\n", voltage_of_battery);
            // printf("cur percent of battery : %bu\n", cur_percentage_of_battery);

            flag_get_voltage_of_battery = 1;
            flag_get_battery = 1;
        }
    }

#endif

#if 0
    adc_sel_pin(ADC_PIN_BATTERY);
    // battery_val += adc_getval(); // 可能要防止计数溢出
    // battery_scan_cnt++; // 上面采集到一次ad值之后，这里加一表示采集了一次
    if (battery_scan_time_cnt >= BATTERY_SCAN_UPDATE_TIME_MS) // 如果到了电池数据的更新时间（更新/发送电池数据的时间）
    {
        // battery_val /= battery_scan_cnt; // 取平均数
        // battery_val = adc_getval();
        battery_val = adc_single_convert();
        voltage_of_battery = conver_adc_val_to_voltage(battery_val);
        cur_percentage_of_battery = conver_voltage_of_battery_to_percentage(voltage_of_battery);
        battery_val = 0;           // 清空数值
        battery_scan_cnt = 0;      // 清空计数值
        battery_scan_time_cnt = 0; // 清空时间计数值

        fun_info.battery = cur_percentage_of_battery;
        fun_info.voltage_of_battery = voltage_of_battery;

        // printf("cur voltage of battery : %bu\n", voltage_of_battery);
        // printf("cur percent of battery : %bu\n", cur_percentage_of_battery);

        flag_get_voltage_of_battery = 1;
        flag_get_battery = 1;
    }
#endif
}

#endif // BATTERY_SCAN_ENABLE