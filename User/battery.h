#ifndef __BATTERY_H
#define __BATTERY_H

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if BATTERY_SCAN_ENABLE

// ======================================================
// 电池电量检测配置:
// 电池电量检测的更新时间(单位：ms，每隔 xx ms更新一次)
// 注意更新时间不能过长(不超过1min)，否则在求电池电压平均值前，会计数溢出
// #define BATTERY_SCAN_UPDATE_TIME_MS (1000)
#define BATTERY_SCAN_UPDATE_TIME_MS (2000) 
// #define BATTERY_SCAN_UPDATE_TIME_MS (3000)

// 电池满电时的电压:(例， 电池4.2V满电， MAX_VOLTAGE_OF_BATTERY == 42)
// 目前外部使用1/10分压（使用12V的电池，有可能到15V）
#define MAX_VOLTAGE_OF_BATTERY (150) 
// 电池放电的截止电压：（例，电池2.3V截止,MIN_VOLTAGE_OF_BATTERY == 23）
// #define MIN_VOLTAGE_OF_BATTERY (90)
// 电池满电时，在检测引脚检测到的ad值
// #define MAX_VOLTAGE_OF_BATTERY_AD_VAL (4034)
// 电池电量检测配置
//======================================================

extern volatile u16 battery_scan_time_cnt; // 电池扫描时间计时

void battery_scan(void);

#endif // BATTERY_SCAN_ENABLE
#endif