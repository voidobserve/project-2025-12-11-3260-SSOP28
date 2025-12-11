#ifndef __INSTRUCTION_H
#define __INSTRUCTION_H

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

// 定义同步请求的各个状态，用于状态机
enum
{
    SYN_REQUEST_STATUS_NONE = 0, // 未收到同步请求
    SYN_REQUEST_STATUS_HANDLING, // 正在处理同步请求，等待冷却时间
};

// 定义更新时间、日期、星期的各个状态，用在状态机中
enum
{
    UPDATE_STATUS_NONE = 0, // 无任何操作,已经准备就绪
    UPDATE_STATUS_HANDLING, // 正在更新时间，等待冷却
};

// 定义 MP5向单片机发送的的指令
enum
{
    INSTRUCTION_GET_ALL_STATUS = 0x80,   // 获取所有功能的状态
    INSTRUCTION_GET_GEAR = 0x21,         // 获取挡位的状态
    INSTRUCTION_GET_BATTERY = 0x22,      // 获取电池电量的状态
    INSTRUCTION_GET_BARKE = 0x23,        // 获取刹车的状态
    INSTRUCTION_GET_LEFT_TURN = 0x24,    // 获取左转向灯的状态
    INSTRUCTION_GET_RIGHT_TURN = 0x25,   // 获取右转向灯的状态
    INSTRUCTION_GET_HIGH_BEAM = 0x26,    // 获取远光灯的状态
    INSTRUCTION_GET_ENGINE_SPEED = 0x27, // 获取发动机转速
    INSTRUCTION_GET_SPEED = 0x28,        // 获取时速
    INSTRUCTION_GET_FUEL = 0x29,         // 获取油量
#if TEMP_OF_WATER_SCAN_ENABLE
    INSTRUCTION_GET_TEMP_OF_WATER = 0x2A, // 获取水温
#endif

    INSTRUCTION_GET_TOTAL_MILEAGE = 0x8B,      // 获取大计里程
    INSTRUCTION_GET_SUBTOTAL_MILEAGE = 0x8C,   // 获取小计里程
    INSTRUCTION_GET_SUBTOTAL_MILEAGE_2 = 0x94, // 获取小计里程2

    // INSTRUCTION_GET_TOUCH_KEY_STATUS = 0x2D,      // 获取触摸按键状态
    INSTRUCTION_ALTER_DATE = 0x2E,      // 修改日期
    INSTRUCTION_ALTER_TIME = 0x8E,      // 修改时间
    INSTRUCTION_GET_BATTERY_VAL = 0x30, // 获取电池电压
#if TEMP_OF_WATER_SCAN_ENABLE
    INSTRUCTION_SET_TEMP_OF_WATER_WARNING = 0x31, // 设置水温报警
#endif
    INSTRUCTION_CLEAR_TOTAL_MILEAGE = 0x98,      // 清除大计里程
    INSTRUCTION_CLEAR_SUBTOTAL_MILEAGE = 0x99,   // 清除小计里程
    INSTRUCTION_CLEAR_SUBTOTAL_MILEAGE_2 = 0x9A, // 清除小计里程2

};

// // 定义 水温报警的相关操作
// enum
// {
//     OPERATION_SET_TEMP_OF_WATER_WARNING_NONE = 0, // 无操作
//     OPERATION_SET_TEMP_OF_WATER_WARNING_ON,       // 打开水温报警
//     OPERATION_SET_TEMP_OF_WATER_WARNING_OFF,      // 关闭水温报警
// };

extern volatile bit flag_get_all_status;   // 获取所有功能的状态
extern volatile bit flag_get_gear;         // 获取挡位状态 / 得到了挡位的状态
extern volatile bit flag_get_battery;      // 获取电池电量的状态/ 得到了电池的状态（电池电量，单位：百分比）
// extern volatile bit flag_get_brake;        // 获取刹车状态 / 得到了刹车的状态
// extern volatile bit flag_get_left_turn;    // 获取左转向灯的状态 / 得到了左转向灯的状态
// extern volatile bit flag_get_right_turn;   // 获取右转向灯的状态 / 得到了右转向灯的状态
// extern volatile bit flag_get_high_beam;    // 获取远光灯的状态 / 得到了远光灯的状态
extern volatile bit flag_get_engine_speed; // 获取发动机的转速 / 得到了发动机的转速
extern volatile bit flag_get_speed;        // 获取时速 / 得到了时速
extern volatile bit flag_get_fuel;         // 获取油量 / 得到了油量（单位：百分比）
#if TEMP_OF_WATER_SCAN_ENABLE
// extern volatile bit flag_get_temp_of_water; // 获取水温 / 得到了水温
#endif

extern volatile bit flag_update_malfunction_status; // 标志位，更新故障的状态
extern volatile bit flag_update_abs_status;         // 标志位，更新abs的状态

extern volatile bit flag_get_total_mileage;       // 获取大计里程 / 得到了大计里程
extern volatile bit flag_get_sub_total_mileage;   // 获取小计里程 / 得到了小计里程
extern volatile bit flag_get_sub_total_mileage_2; // 获取小计里程2 / 得到了小计里程2(数据需要更新)

// extern volatile bit flag_get_touch_key_status;      // 获取触摸按键的状态
extern volatile bit flag_alter_date;                // 修改日期
extern volatile bit flag_alter_time;                // 修改时间
extern volatile bit flag_get_voltage_of_battery;    // 获取电池电压
#if TEMP_OF_WATER_SCAN_ENABLE
extern volatile bit flag_set_temp_of_water_warning; // 设置水温报警
#endif
extern volatile bit flag_clear_total_mileage;       // 清除大计里程
extern volatile bit flag_clear_sub_total_mileage;   // 清除小计里程
extern volatile bit flag_clear_sub_total_mileage_2; // 清除小计里程2

extern volatile u8 synchronous_request_status;    // 同步请求状态机
extern volatile u16 synchronous_request_time_cnt; // 同步请求时间计时

extern volatile u8 update_time_status;       // 更新时间的状态机
// extern volatile u8 update_date_status;       // 更新日期的状态机
extern volatile u16 update_time_cooling_cnt; // 更新时间的冷却计数
// extern volatile u16 update_date_cooling_cnt; // 更新日期的冷却计数

extern void instruction_scan(void);   // 扫描指令缓冲区中是否有对应指令
extern void instruction_handle(void); // 扫描是否有对应的指令,如果有，则执行对应操作

#endif