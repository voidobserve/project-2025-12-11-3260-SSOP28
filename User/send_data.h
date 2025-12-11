#ifndef __SEND_DATA_H__
#define __SEND_DATA_H__

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#ifndef FORMAT_HEAD // 定义格式头
#define FORMAT_HEAD ((u8)0xA5)
#endif // FORMAT_HEAD

// 定义单片机发送的带有数据的指令
enum
{
    SEND_GEAR = 0x01,         // 发送 挡位的状态
    SEND_BATTERY = 0x02,      // 发送 电池电量的状态
    SEND_BARKE = 0x03,        // 发送 刹车的状态
    SEND_LEFT_TURN = 0x04,    // 发送 左转向灯的状态
    SEND_RIGHT_TURN = 0x05,   // 发送 右转向灯的状态
    SEND_HIGH_BEAM = 0x06,    // 发送 远光灯的状态
    SEND_ENGINE_SPEED = 0x07, // 发送 发动机的转速
    SEND_SPEED = 0x08,        // 发送 时速
    SEND_FUEL = 0x09,         // 发送 油量
    SEND_WATER_TEMP = 0x0A,   // 发送 水温

    /*
        发送大计里程， 数据 3 byte ， 数据直接用作屏幕显示，
        例如 0d 04 17 65 ，显示 04176.5 km
    */ 
    SEND_TOTAL_MILEAGE_TENTH_OF_KM = 0x0B,
    /*
        发送大计里程， 数据 3 byte ， 数据直接用作屏幕显示，
        例如 0d 04 17 65 ，显示 04176.5 mile
    */ 
    SEND_TOTAL_MILEAGE_TENTH_OF_MILE = 0x0C,

    SEND_TOUCH_KEY_STATUS = 0x0D, // 发送触摸按键的状态
    SEND_TIME = 0x0E,             // 发送时间（年月日，时分秒）

    SEND_VOLTAGE_OF_BATTERY = 0x10,  // 发送电池电压
    SEND_TEMP_OF_WATER_ALERT = 0x11, // 发送水温报警

    SEND_MALFUNCTION_STATUS = 0x12, // 发送故障的状态
    SEND_ABS_STATUS = 0x13,         // 发送ABS的状态

    /*
        发送小计里程， 数据 3 byte ， 数据直接用作屏幕显示，
        例如 0d 04 17 65 ，显示 04176.5 km
    */ 
    SEND_SUBTOTAL_MILEAGE_TENTH_OF_KM = 0x14,
    /*
        发送小计里程， 数据 3 byte ， 数据直接用作屏幕显示，
        例如 0d 04 17 65 ，显示 04176.5 mile
    */ 
    SEND_SUBTOTAL_MILEAGE_TENTH_OF_MILE = 0x15,

    /*
        发送 小计里程， 数据 2 byte ， 直接用作屏幕显示，
        例如 0d 1765 ，显示 1765 km
    */
    SEND_SUBTOTAL_MILEAGE_KM = 0x18,

    /*
        发送 小计里程， 数据 2 byte ， 直接用作屏幕显示，
        例如 0d 1765 ，显示 1765 mile
    */
    SEND_SUBTOTAL_MILEAGE_MILE = 0x19,

    SEND_SPEED_WITH_MILE = 0x1A, // 发送 时速（单位：英里每小时 mile/h）
};
typedef u8 SEND_DATA_CMD_T; // 定义 发送数据的指令 枚举类型

#define SEND_DATA_BUFF_MAX_LEN ((u8)20) // 待发送数据缓存最大长度
// void send_data(SEND_DATA_CMD_T instruct, u32 send_data);

void send_data_packet(SEND_DATA_CMD_T cmd);

#endif
