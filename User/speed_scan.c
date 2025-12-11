#include "speed_scan.h"

#if SPEED_SCAN_ENABLE

// 时速扫描的配置
void speed_scan_config(void)
{
#if 1 // 使用定时器扫描IO电平变化来计算脉冲

    P1_MD1 &= ~GPIO_P15_MODE_SEL(0x3); // 输入模式
    P1_PU |= GPIO_P15_PULL_UP(0x1);    // 配置为上拉

#endif // 使用定时器扫描IO电平变化来计算脉冲
}
 

volatile bit flag_is_speed_scan_over_time = 0; // 速度检测是否一直没有脉冲到来，导致超时
volatile u32 speed_pulse_cnt = 0;              // 记录脉冲个数，在定时器中断累加
volatile u32 speed_scan_time_ms = 0;           // 记录扫描时间
static volatile u32 cur_speed_scan_time = 0;
static volatile u32 cur_speed_scan_pulse = 0;

void update_speed_scan_data(void) // 更新检测时速的数据
{
    cur_speed_scan_time += speed_scan_time_ms;
    speed_scan_time_ms = 0;
    cur_speed_scan_pulse += speed_pulse_cnt;
    speed_pulse_cnt = 0;
}

void speed_scan(void)
{
    volatile u32 cur_speed = 0;
    u32 tmp = 0;

    if (cur_speed_scan_time >= SPEED_SCAN_UPDATE_TIME || flag_is_speed_scan_over_time)
    {
        /*
            采集到的脉冲个数 / 一圈对应的脉冲个数 * 车轮一圈对应走过的距离（单位：mm），
            计算得到 采集的脉冲个数对应走过的距离（单位：mm）
        */
        // u32 tmp = (cur_speed_scan_pulse * SPEED_SCAN_MM_PER_TURN / SPEED_SCAN_PULSE_PER_TURN);
        tmp = ((cur_speed_scan_pulse * SPEED_SCAN_MM_PER_TURN) / SPEED_SCAN_PULSE_PER_TURN);
        // printf("cur_speed_scan_pulse %lu\n", cur_speed_scan_pulse);

        if (flag_is_speed_scan_over_time) // 超时，采集到的脉冲个数对应一直是0km/h，认为时速是0
        {
            cur_speed = 0;
        }
        else // 未超时，计算采集到的脉冲个数对应走过的距离，再转换成以km/h的单位
        {
            /*
                采集的脉冲个数对应走过的距离（单位：mm）/ 采集所用的时间（单位：ms） == 速度（单位：mm/ms）
                1mm/ms == 1m/s
                因为 1mm/ms * 1000 == 1m/ms
                     1m/ms  / 1000 == 1m/s
                     先乘以1000再除以1000，那么这个操作就可以去掉，直接化简为 1mm/ms == 1m/s

                1m/s == 3.6km/h，那么根据得到的 以 m/s 为单位的速度，先乘以36再除以10，得到以km/h为单位的速度值

                cur_speed == 采集的脉冲个数对应走过的距离（单位：mm）/ 采集所用的时间（单位：ms）* 3.6
                换成单片机可以计算的格式：
                cur_speed == 采集的脉冲个数对应走过的距离（单位：mm） * 36 / 10 / 采集所用的时间（单位：ms）
            */
            cur_speed = tmp * 36 / 10 / cur_speed_scan_time;
        }

        // 防止时速为0时（有可能是推车，记录不到速度），记录不到里程
        distance += tmp;

        // printf("cur distace 2 %lu\n", distance);
        // printf("cur distace %lu\n", distance);
        // printf("cur speed %lu km/h\n", cur_speed);

        cur_speed_scan_pulse = 0;
        cur_speed_scan_time = 0;
        flag_is_speed_scan_over_time = 0;

        fun_info.speed = cur_speed;
        // 限制要发送的时速:
        // if (fun_info.speed > 999)
        // {
        //     fun_info.speed = 999;
        // }

        flag_get_speed = 1; // 表示速度有数据更新
    } // if (cur_speed_scan_time >= 500 || flag_is_speed_scan_over_time)
}

#endif
