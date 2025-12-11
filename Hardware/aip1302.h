#ifndef __AIP1302_H
#define __AIP1302_H

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件
#include "typedef_struct.h"

#if IC_1302_ENABLE

#define AIP1302_CE_PIN P17
#define AIP1302_SCLK_PIN P16
#define AIP1302_DATA_PIN P24

// 写地址(读地址是在写地址的基础上加一)
#define AIP1302_SEC_REG_ADDR ((u8)0x80)           // 秒寄存器地址
#define AIP1302_MIN_REG_ADDR ((u8)0x82)           // 分寄存器地址
#define AIP1302_HOUR_REG_ADDR ((u8)0x84)          // 小时寄存器地址
#define AIP1302_DATE_REG_ADDR ((u8)0x86)          // 日
#define AIP1302_MONTH_REG_ADDR ((u8)0x88)         // 月
#define AIP1302_WEEKDAY_REG_ADDR ((u8)0x8A)       // 星期
#define AIP1302_YEAR_REG_ADDR ((u8)0x8C)          // 年
#define AIP1302_WRITE_PROTECT_REG_ADDR ((u8)0x8E) // 写保护位读地址

#define AIP1302_RAM_START_ADDR ((u8)0xC0) // RAM起始地址

#define AIP1302_DELAY() delay(4)

u8 __aip1302_read_byte(const u8 cmd);
u8 aip1302_is_running(void);

void aip1302_config(void);
u8 aip1302_read_byte(const u8 cmd);
void aip1302_write_byte(const u8 cmd, u8 byte);
void aip1302_update_time(aip1302_saveinfo_t aip1302_saveinfo); // 向aip1302更新时间，时:分:秒
#if 0
void aip1302_update_date(aip1302_saveinfo_t aip1302_saveinfo);     // 向aip1302更新日期,年月日
#endif
// void aip1302_update_weekday(aip1302_saveinfo_t aip1302_saveinfo);  // 向aip1302更新星期

void aip1302_read_all(void); // 从aip1302中读出所有有关时间的数据

#if 1
void aip1302_test(void);
void aip1302_update_all_data(aip1302_saveinfo_t aip1302_saveinfo); // 从aip1302中读出所有有关时间的数据,存放到全局变量中
#endif

#endif

#endif
