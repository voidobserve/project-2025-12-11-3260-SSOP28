#include "pin_level_scan.h"

#if PIN_LEVEL_SCAN_ENABLE

volatile u16 pin_level_scan_time_cnt;

void pin_level_scan_config(void)
{
#if 0
    // p21_input_config(); // 远光灯状态对应的引脚
    // 配置为输入模式
    P2_MD0 &= ~(GPIO_P21_MODE_SEL(0x3));
    // 配置为上拉
    P2_PU |= (GPIO_P21_PULL_UP(0x1));
#endif

    // 左转向灯对应的引脚
    P2_MD1 &= ~(GPIO_P25_MODE_SEL(0x3)); // 配置为输入模式

    // 右转向灯状态对应的引脚
    P2_MD0 &= ~(GPIO_P22_MODE_SEL(0x3)); // 配置为输入模式

    // 6档对应的引脚 （硬件原因，挡位检测脚不用开上拉）
    P2_MD1 &= ~(GPIO_P27_MODE_SEL(0x3)); // 配置为输入模式

    // 5档对应的引脚 （硬件原因，挡位检测脚不用开上拉）
    P3_MD0 &= ~(GPIO_P30_MODE_SEL(0x3)); // 配置为输入模式

    // 4档对应的引脚 （硬件原因，挡位检测脚不用开上拉）
    P1_MD1 &= ~(GPIO_P14_MODE_SEL(0x3)); // 配置为输入模式

    // 3档对应的引脚 （硬件原因，挡位检测脚不用开上拉）
    P1_MD0 &= ~(GPIO_P13_MODE_SEL(0x3)); // 配置为输入模式

    // 2档对应的引脚 （硬件原因，挡位检测脚不用开上拉）
    P1_MD0 &= ~(GPIO_P10_MODE_SEL(0x3)); // 配置为输入模式

    // 1档对应的引脚 （硬件原因，挡位检测脚不用开上拉）
    P0_MD1 &= ~(GPIO_P07_MODE_SEL(0x3)); // 配置为输入模式

    // 空挡对应的引脚 （硬件原因，挡位检测脚不用开上拉）
    P0_MD1 &= ~(GPIO_P06_MODE_SEL(0x3)); // 配置为输入模式

    // 检测 水温报警 的引脚:
    P2_MD0 &= ~(GPIO_P20_MODE_SEL(0x03)); // 输入模式
    P2_PU |= GPIO_P20_PULL_UP(0x01);      // 上拉
 
}

// 引脚电平扫描，都是低电平有效
void pin_level_scan(void)
{
    // 如果到了扫描时间，再更新挡位、转向灯和刹车的状态
    if (pin_level_scan_time_cnt >= PIN_LEVEL_SCAN_TIME_MS)
    {
        pin_level_scan_time_cnt = 0;

#if 0  // 刹车检测
       // if (PIN_DETECT_BRAKE)
       // {
       //     // 如果没有刹车
       //     fun_info.brake = OFF;
       // }
       // else
       // {
       //     // 如果有刹车
       //     fun_info.brake = ON;
       // }

        // flag_get_brake = 1;
#endif // 刹车检测

#if 1 // 左转向灯、右转向灯、远光灯
        if (PIN_DETECT_LEFT_TURN)
        {
            // 如果左转向灯未开启
            fun_info.left_turn = OFF;
        }
        else
        {
            // 如果左转向灯开启
            fun_info.left_turn = ON;
        }
        flag_get_left_turn = 1;

        if (PIN_DETECT_RIGHT_TURN)
        {
            // 如果右转向灯未开启
            fun_info.right_turn = OFF;
        }
        else
        {
            // 如果右转向灯开启
            fun_info.right_turn = ON;
        }
        flag_get_right_turn = 1;

        // if (PIN_DETECT_HIGH_BEAM)
        // {
        //     // 如果远光灯未开启
        //     fun_info.high_beam = OFF;
        // }
        // else
        // {
        //     // 如果远光灯开启
        //     fun_info.high_beam = ON;
        // }
        // flag_get_high_beam = 1;
#endif

        // 以最低挡位优先，当最低档有信号时，不管其他挡位的信号，直接以最低档的为主
        if (0 == PIN_DETECT_NEUTRAL_GEAR)
        {
            // 空挡
            fun_info.gear = GEAR_NEUTRAL;
        }
        else if (0 == PIN_DETECT_FIRST_GEAR)
        {
            // 一档
            fun_info.gear = GEAR_FIRST;
        }
        else if (0 == PIN_DETECT_SECOND_GEAR)
        {
            // 二档
            fun_info.gear = GEAR_SECOND;
        }
        else if (0 == PIN_DETECT_THIRD_GEAR)
        {
            // 三档
            fun_info.gear = GEAR_THIRD;
        }
        else if (0 == PIN_DETECT_FOURTH_GEAR)
        {
            // 四档
            fun_info.gear = GEAR_FOURTH;
        }
        else if (0 == PIN_DETECT_FIFTH_GEAR)
        {
            // 五档
            fun_info.gear = GEAR_FIFTH;
        }
        else if (0 == PIN_DETECT_XIXTH_GEAR)
        {
            // 六档
            fun_info.gear = GEAR_SIXTH;
        }
        else
        {
            /*
                空挡、一档 ~ 六档都没有检测到，则赋值为 GEAR_UNKNOWN
                让显示屏中档位对应的图标空着
            */
            fun_info.gear = GEAR_UNKNOWN;
        }

        // printf("cur gear %bu\n", fun_info.gear);
        flag_get_gear = 1;

        if (0 == PIN_DETECT_TEMP_OF_WATER_ALERT)
        {

            // 如果检测到水温报警
            fun_info.flag_is_in_water_temp_warning = 1;
        }
        else
        {
            // 如果未检测到水温报警
            fun_info.flag_is_in_water_temp_warning = 0;
        }
        flag_set_temp_of_water_warning = 1;

        if (0 == PIN_DETECT_MALFUNCTION)
        {
            // 如果检测到了故障
            fun_info.flag_is_detect_malfunction = 1;

            // 没有引脚检测abs的状态，这里检测到故障后，也顺便设置abs的状态
            // fun_info.flag_is_detect_abs = 1;
        }
        else
        {
            // 如果未检测到故障
            fun_info.flag_is_detect_malfunction = 0;

            // 没有引脚检测abs的状态，这里更新故障的状态后，也顺便设置abs的状态
            // fun_info.flag_is_detect_abs = 0;
        }

        //   printf("cur malfunction %bu\n", fun_info.flag_is_detect_malfunction);

        flag_update_malfunction_status = 1;
    }

    // if () // ACC引脚检测，检测到高电平，P03也输出高电平，检测到低电平，让P03输出低电平
    // if (P04)
    // {
    //     // 检测到ACC为高电平,
    //     P03 = 1;
    // }
    // else
    // {
    //     P03 = 0;
    // }
}

#endif
