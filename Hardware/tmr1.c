#include "tmr1.h"

// #define TMR1_CNT_TIME 152 // 152 * 0.65625us 约等于100us

// 定时器定时周期 (单位:Hz)
// 周期值 = 系统时钟 / 定时器分频 / 频率 - 1
#define TMR1_PERIOD (SYSCLK / 128 / 1000 - 1) // 1000Hz,1ms

// volatile bit tmr1_flag = 0; // TMR1中断服务函数中会置位的标志位
// volatile u32 tmr1_cnt = 0; // 定时器TMR1的计数值（每次在中断服务函数中会加一）

/**
 * @brief 配置定时器TMR1，配置完成后，定时器默认关闭
 */
void tmr1_config(void)
{
    __SetIRQnIP(TMR1_IRQn, TMR1_IQn_CFG); // 设置中断优先级（TMR1）
    __DisableIRQ(TMR1_IRQn);              // 禁用中断
    IE_EA = 1;                            // 打开总中断

    TMR_ALLCON = TMR1_CNT_CLR(0x1); // 清除计数值

    TMR1_CONL &= ~TMR_PRESCALE_SEL(0x07); // 清除TMR1的预分频配置寄存器
    TMR1_CONL |= TMR_PRESCALE_SEL(0x07);  // 定时器预分频
    TMR1_CONL &= ~TMR_MODE_SEL(0x03);     // 清除TMR1的模式配置寄存器
    TMR1_CONL |= TMR_MODE_SEL(0x01);      // 配置TMR1的模式为计数器模式，最后对系统时钟的脉冲进行计数

    TMR1_CONH &= ~TMR_PRD_PND(0x01); // 清除TMR1的计数标志位，表示未完成计数
    TMR1_CONH |= TMR_PRD_IRQ_EN(1);  // 使能TMR1的计数中断

    // 配置TMR1的计数周期
    TMR1_PRH = TMR_PERIOD_VAL_H((TMR1_PERIOD >> 8) & 0xFF); // 周期值
    TMR1_PRL = TMR_PERIOD_VAL_L((TMR1_PERIOD >> 0) & 0xFF);

    // TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除TMR1的时钟源配置寄存器
    // TMR1_CONL |= TMR_SOURCE_SEL(0x05);    // 配置TMR1的时钟源，不用任何时钟

    // 重新给TMR1配置时钟
    TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR1_CONL |= TMR_SOURCE_SEL(0x06);    // 配置定时器的时钟源，使用系统时钟

    __EnableIRQ(TMR1_IRQn); // 使能中断
    // IE_EA = 1;              // 打开总中断
}

// TMR1中断服务函数
void TIMR1_IRQHandler(void) interrupt TMR1_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR1_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (TMR1_CONH & TMR_PRD_PND(0x1))
    {
        TMR1_CONH |= TMR_PRD_PND(0x1); // 清除pending

        // if (tmr1_cnt < 4294967295)
        // {
        //     tmr1_cnt++;
        // }

        // 串口接收超时计数
        if (flag_is_uart0_receive_timeout_enable)
        {
            uart0_receive_timeout_cnt++;
        }

#if AD_KEY_ENABLE
        // 在定时器注册按键扫描：
        // if (ad_key_para.cur_scan_times < 255)
        {
            ad_key_para.cur_scan_times++;
        }
#endif // AD_KEY_ENABLE

#if TOUCH_KEY_ENABLE
        // if (touch_key_para.cur_scan_times < 255)
        {
            touch_key_para.cur_scan_times++;
        }
#endif // TOUCH_KEY_ENABLE

#if PIN_LEVEL_SCAN_ENABLE
        // if (pin_level_scan_time_cnt < 65535) // 防止计数溢出
        {
            pin_level_scan_time_cnt++;
        }
#endif

        // if (mileage_save_time_cnt < 4294967295 - diff_ms_cnt) // 防止计数溢出
        if (mileage_save_time_cnt < 65535)
        {
            mileage_save_time_cnt++;
            // mileage_save_time_cnt += diff_ms_cnt;
        }

#if FUEL_CAPACITY_SCAN_ENABLE
        // if (fuel_capacity_scan_cnt < 4294967295 - diff_ms_cnt) // 防止计数溢出
        {
            fuel_capacity_scan_cnt++;
            // fuel_capacity_scan_cnt += diff_ms_cnt;
        }
#endif

        // if (synchronous_request_status == SYN_REQUEST_STATUS_HANDLING)
        {
            synchronous_request_time_cnt++; // 同步请求的冷却计时
            // synchronous_request_time_cnt += diff_ms_cnt; // 同步请求的冷却计时
            if (synchronous_request_time_cnt >= 2000)
            {
                // 如果接收同步请求已经过了 xx s，清除冷却状态
                synchronous_request_time_cnt = 0;
                synchronous_request_status = SYN_REQUEST_STATUS_NONE;
            }
        }

#if 0 // 日期和时间合到了一起，就不用这部分程序
    if (update_date_status == UPDATE_STATUS_HANDLING)
    {
        // 如果更新日期进入冷却状态，进行冷却计时
        // update_date_cooling_cnt++;
        update_date_cooling_cnt += diff_ms_cnt;
        if (update_date_cooling_cnt >= 100) // xx ms
        {
            // 过了冷却时间，退出冷却状态
            update_date_cooling_cnt = 0;
            update_date_status = UPDATE_STATUS_NONE;
        }
    }
#endif

        // if (update_time_status == UPDATE_STATUS_HANDLING)
        {
            // 如果更新时间进入冷却状态，进行冷却计时
            update_time_cooling_cnt++;
            // update_time_cooling_cnt += diff_ms_cnt;
            if (update_time_cooling_cnt >= 100) // xx ms
            {
                // 过了冷却时间，退出冷却状态
                update_time_cooling_cnt = 0;
                update_time_status = UPDATE_STATUS_NONE;
            }
        }

        // if (mileage_update_time_cnt < 65535)
        {
            mileage_update_time_cnt++;
        }

#if BATTERY_SCAN_ENABLE
        // if (battery_scan_time_cnt < 4294967295)
        {
            battery_scan_time_cnt++;
        }
#endif // BATTERY_SCAN_ENABLE

#if 0 // DEBUG 只在测试时使用

            {
                static u16 cnt;
                cnt++;
                if (cnt >= 1000)
                {
                    cnt = 0;
                    flag_is_debug_update = 1;
                }
            }

#endif // // DEBUG 只在测试时使用

        // {
        //     static u16 cnt;
        //     cnt++;
        //     if (cnt >= 4000)
        //     {
        //         cnt = 0;
        //         flag_debug_is_send_time = 1; // 发送时间
        //         // flag_debug_is_send_time_2 = 1;
        //     }
        // }
    }

    // P20 = 0;// 测试中断持续时间
    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR1_IRQn);
}