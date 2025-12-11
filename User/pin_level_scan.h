#ifndef __PIN_LEVEL_SCAN_H
#define __PIN_LEVEL_SCAN_H

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if PIN_LEVEL_SCAN_ENABLE

#define PIN_DETECT_LEFT_TURN (P25)   // 检测左转向灯的引脚
#define PIN_DETECT_RIGHT_TURN (P22)  // 检测右转向灯的引脚
#define PIN_DETECT_HIGH_BEAM (P21)   // 检测远光灯的引脚
#define PIN_DETECT_BRAKE (P23)       // 检测刹车的引脚
#define PIN_DETECT_MALFUNCTION (P20) // 检测故障的引脚

#define PIN_DETECT_NEUTRAL_GEAR (P06) // 检测空挡的引脚
#define PIN_DETECT_FIRST_GEAR (P07)   // 检测一挡的引脚
#define PIN_DETECT_SECOND_GEAR (P10)  // 检测二挡的引脚
#define PIN_DETECT_THIRD_GEAR (P13)   // 检测三挡的引脚
#define PIN_DETECT_FOURTH_GEAR (P14)  // 检测四挡的引脚
#define PIN_DETECT_FIFTH_GEAR (P30)   // 检测五挡的引脚
#define PIN_DETECT_XIXTH_GEAR (P27)   // 检测六挡的引脚

// ======================================================
// 引脚电平扫描配置：
// 扫描时间，每隔 xx ms更新一次状态(不能大于变量类型对应的最大值)
#define PIN_LEVEL_SCAN_TIME_MS (200)
// 引脚电平扫描配置
// ======================================================

extern u16 pin_level_scan_time_cnt;

void pin_level_scan_config(void); // 扫描引脚的配置（初始化）
void pin_level_scan(void);        // 扫描引脚的电平
#endif                            // PIN_LEVEL_SCAN_ENABLE

#endif
