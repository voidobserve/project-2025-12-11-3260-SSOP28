#ifndef __ADC_H
#define __ADC_H

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if (BATTERY_SCAN_ENABLE || AD_KEY_ENABLE || FUEL_CAPACITY_SCAN_ENABLE || TEMP_OF_WATER_SCAN_ENABLE)

// 定义adc检测引脚
// ADC_PIN //
enum
{
    ADC_PIN_NONE = 0x00,

    ADC_PIN_BATTERY = 0x01, // 电池电量检测引脚
    // ADC_PIN_TOUCH, // 检测触摸IC发送过来的电压的引脚
    ADC_PIN_KEY,           // 检测ad按键的引脚
    ADC_PIN_FUEL,          // 检测油量的引脚
    ADC_PIN_TEMP_OF_WATER, // 检测水温的引脚
};

extern u16 adc_val;
// extern bit adc_done_flag;

void adc_config(void);
void adc_sel_pin(u8 pin_index);
u16 adc_single_convert(void);
u16 adc_getval(void); // adc采集+滤波

// void adc_showval(void);
// void adc_scan(void);

#endif

#endif
