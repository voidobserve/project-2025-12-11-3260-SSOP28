#include "engine_speed_scan.h"

#if ENGINE_SPEED_SCAN_ENABLE

// 发动机转速的相关配置
void engine_speed_scan_config(void)
{
    // 使用定时器扫描IO电平的方式

    P0_MD0 &= ~GPIO_P02_MODE_SEL(0x3); // 输入模式
    P0_PU |= GPIO_P02_PULL_UP(0x01);   // 上拉
}

volatile u32 engine_speed_scan_cnt; // 检测到的脉冲个数，在定时器中断累加
volatile u16 engine_speed_scan_ms;  // 在定时器中断累加

static volatile u32 cur_engine_speed_scan_cnt;
static volatile u32 cur_engine_speed_scan_ms;

volatile bit flag_is_engine_speed_scan_over_time; // 标志位，检测是否超时

void update_engine_speed_scan_data(void) // 更新检测发动机转速的数据
{
    cur_engine_speed_scan_cnt += engine_speed_scan_cnt;
    engine_speed_scan_cnt = 0;
    cur_engine_speed_scan_ms += engine_speed_scan_ms;
    engine_speed_scan_ms = 0;
}

// 发动机转速扫描
void engine_speed_scan(void)
{
#define CONVER_ONE_MINUTE_TO_MS (60000UL) // 将1min转换成以ms为单位的数据
    volatile u32 rpm;                     // 由下面的语句赋值，这里为了节省程序空间，没有赋初始值

    if (cur_engine_speed_scan_ms >= ENGINE_SPEED_SCAN_UPDATE_TIME || flag_is_engine_speed_scan_over_time)
    // if (cur_engine_speed_scan_ms >= ENGINE_SPEED_SCAN_UPDATE_TIME )
    {
        // printf("cur_engine_speed_scan_ms:%lu\n", cur_engine_speed_scan_ms);
        if (flag_is_engine_speed_scan_over_time)
        {
            rpm = 0;
        }
        else
        {
            /*
                (1min / 1min转过的圈数) == (扫描时间 / 扫描时间内的转过的圈数)
                1min转过的圈数 == 1min * 扫描时间内转过的圈数 / 扫描时间
                1min转过的圈数 == 1min * (扫描时间内采集到的脉冲个数 / 发动机转过一圈对应的脉冲个数) / 扫描时间
                转换成单片机能计算的形式：
                1min转过的圈数 == 扫描时间内采集到的脉冲个数 * 1min / 发动机转过一圈对应的脉冲个数 / 扫描时间
                1min转过的圈数 == 扫描时间内采集到的脉冲个数 * 1min / 扫描时间 / 发动机转过一圈对应的脉冲个数
            */
            // rpm = (u32)cur_engine_speed_scan_cnt * ((u32)CONVER_ONE_MINUTE_TO_MS / ENGINE_SPEED_SCAN_PULSE_PER_TURN) / cur_engine_speed_scan_ms;

            /*
                扫描时间内转过的圈数 == 一个脉冲对应转过的圈数 *　扫描时间内采集到的脉冲个数
                1min转过的圈数　== 扫描时间内转过的圈数 / 扫描时间 * 1min
            */
            rpm = (u32)cur_engine_speed_scan_cnt * ENGINE_SPEED_SCAN_A_PULSE_PER_TURNS * CONVER_ONE_MINUTE_TO_MS / cur_engine_speed_scan_ms;
        }

        // printf("cur engine speed pulse cnt:%lu\n",cur_engine_speed_scan_cnt);

        cur_engine_speed_scan_cnt = 0;
        cur_engine_speed_scan_ms = 0;
        flag_is_engine_speed_scan_over_time = 0;

        // 限制待发送的发动机转速
        if (rpm >= 65535)
        {
            rpm = 65535;
        }

#if 1
        /*
            扫描完就发送的程序，在显示部分会有卡顿，
            显示做不了动画，只能单片机来调节
        */

        // printf("cur rpm %lu\n", rpm);

        fun_info.engine_speeed = rpm; // 向全局变量存放发动机转速
        flag_get_engine_speed = 1;    // 发送发动机转速
#endif

        // engine_speed_buff_update(rpm);
    }
}

#if 0
void engine_speed_buff_update(u32 engine_speed)
{
    static u32 last_engine_speed;       // 存放上一次采集到的发动机转速
    u32 engine_speed_difference = 0;    // 存放 发动机转速 的差值
    bit dir_of_engine_speed_change = 0; // 发动机转速变化的方向，0--变小，1--变大
    u8 i;                               // 循环计数值（由下面的语句赋值，这里为了节省程序空间，没有给初始值）

    if (engine_speed > last_engine_speed)
    {
        // 如果当前的发动机转速 大于 上一次采集到的发动机转速
        engine_speed_difference = engine_speed - last_engine_speed;
        dir_of_engine_speed_change = 1; // 表示发动机转速变大
    }
    else if (engine_speed < last_engine_speed)
    {
        // 如果当前的发动机转速 小于 上一次采集到的发动机转速
        engine_speed_difference = last_engine_speed - engine_speed;
        dir_of_engine_speed_change = 0; // 表示发动机转速变小
    }
    else
    {
        for (i = 0; i < ENGINE_SPEED_SCAN_BUFF_SIZE; i++)
        {
            engine_speed_buff[i] = engine_speed;
        }

        // // 没有差值，直接更新（修复没有差值且数值为0时，没有发送数据的问题）
        // fun_info.engine_speeed = engine_speed;
        // flag_get_engine_speed = 1;

        cur_send_engine_speed_buff_index = 0; // 游标复位
        return;
    }

    if (dir_of_engine_speed_change)
    {
        // 如果发动机转速在变大，数组从 [0] ~ [ENGINE_SPEED_SCAN_BUFF_SIZE - 1] 数值越来越大
        for (i = 0; i < ENGINE_SPEED_SCAN_BUFF_SIZE; i++)
        {
            engine_speed_buff[i] = engine_speed_difference * (i + 1) / ENGINE_SPEED_SCAN_BUFF_SIZE + last_engine_speed;
        }
    }
    else
    {
        // 如果发动机转速在变小，数组从 [0] ~ [ENGINE_SPEED_SCAN_BUFF_SIZE - 1] 数值越来越小
        for (i = 0; i < ENGINE_SPEED_SCAN_BUFF_SIZE; i++)
        {
            // 这一句会导致最后不能显示 数值为0 ：
            // engine_speed_buff[ENGINE_SPEED_SCAN_BUFF_SIZE - 1 - i] = last_engine_speed - (u32)engine_speed_difference * (ENGINE_SPEED_SCAN_BUFF_SIZE - i - 1) / ENGINE_SPEED_SCAN_BUFF_SIZE;

            engine_speed_buff[ENGINE_SPEED_SCAN_BUFF_SIZE - 1 - i] = last_engine_speed - (u32)engine_speed_difference * (ENGINE_SPEED_SCAN_BUFF_SIZE - i - 1) / ENGINE_SPEED_SCAN_BUFF_SIZE;
        }
    }

    last_engine_speed = engine_speed;
    cur_send_engine_speed_buff_index = 0; // 游标复位
}

void engine_speed_send_data(void)
{
    if (flag_is_send_engine_speed_time_come) // 如果发送 发动机转速的时间到来
    {
        flag_is_send_engine_speed_time_come = 0;

        if (cur_send_engine_speed_buff_index >= ENGINE_SPEED_SCAN_BUFF_SIZE)
        {
            // 防止越界
            return;
        }

        fun_info.engine_speeed = engine_speed_buff[cur_send_engine_speed_buff_index];
        cur_send_engine_speed_buff_index++;

        // printf("fun_info.engine_speed = %lu\n", fun_info.engine_speeed);
        flag_get_engine_speed = 1;
    }
}
#endif

#endif // #if ENGINE_SPEED_SCAN_ENABLE
