#ifndef __TEMP_OF_WATER_H
#define __TEMP_OF_WATER_H

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if TEMP_OF_WATER_SCAN_ENABLE
// 水温检测函数，如果水温过高，会发送水温报警，水温恢复正常时，才发送解除水温报警
void temp_of_water_scan(void);
#endif

#endif

