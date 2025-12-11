#ifndef __TYPEDEF_STRUCT_H
#define __TYPEDEF_STRUCT_H

#include "typedef.h"

typedef struct 
{
    // 日期 4byte 年 月 日
    u16 year; // 年份
    u8 month; // 月份
    u8 day;   // 日期

    // u8 weekday; // 存放星期几?

    // 时间 3byte 时 分 秒，24H制存放
    u8 time_hour;
    u8 time_min;
    u8 time_sec;
} aip1302_saveinfo_t;

#endif 