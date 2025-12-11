// 定时器TMR2的驱动源文件
#include "tmr2.h"

// 定时器定时周期 (单位:Hz)
// 周期值 = 系统时钟 / 定时器分频 / 频率 - 1
#define TMR2_PERIOD (SYSCLK / 1 / 20000 - 1) // 20khz，50us
// #define TMR2_PERIOD (SYSCLK / 1 / 10000 - 1) // 10khz，100us

// static volatile u8 tmr2_cnt = 0; // 定时器TMR2的计数值（每次在中断服务函数中会加一）

/**
 * @brief 配置定时器TMR2，配置完成后，定时器默认关闭
 */
void tmr2_config(void)
{
    __SetIRQnIP(TMR2_IRQn, TMR2_IQn_CFG); // 设置中断优先级
    __DisableIRQ(TMR2_IRQn);              // 禁用中断
    IE_EA = 1;                            // 打开总中断

    // 清除TMR2的计数值
    TMR_ALLCON = TMR2_CNT_CLR(0x1); // 清除计数值

    TMR2_CONL &= ~TMR_PRESCALE_SEL(0x07); // 清除TMR2的预分频配置寄存器
    TMR2_CONL |= TMR_PRESCALE_SEL(0x0);   // 定时器预分频
    TMR2_CONL &= ~TMR_MODE_SEL(0x03);     // 清除TMR2的模式配置寄存器
    TMR2_CONL |= TMR_MODE_SEL(0x01);      // 配置TMR2的模式为计数器模式，最后对系统时钟的脉冲进行计数

    // 配置TMR2的计数周期
    TMR2_PRH = TMR_PERIOD_VAL_H((TMR2_PERIOD >> 8) & 0xFF); // 周期值
    TMR2_PRL = TMR_PERIOD_VAL_L((TMR2_PERIOD >> 0) & 0xFF);

    // TMR2_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除TMR2的时钟源配置寄存器
    // TMR2_CONL |= TMR_SOURCE_SEL(0x05);    // 配置TMR2的时钟源，不用任何时钟
    TMR2_CONH &= ~TMR_PRD_PND(0x01);      // 清除TMR2的计数标志位，表示未完成计数
    TMR2_CONH |= TMR_PRD_IRQ_EN(1);       // 使能TMR2的计数中断

    TMR2_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR2_CONL |= TMR_SOURCE_SEL(0x06);    // 配置定时器的时钟源，使用系统时钟

    __EnableIRQ(TMR2_IRQn); // 使能中断 
}

// /**
//  * @brief 开启定时器TMR2，开始计时
//  */
// void tmr2_enable(void)
// {
//     // 重新给TMR2配置时钟
//     TMR2_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
//     TMR2_CONL |= TMR_SOURCE_SEL(0x06);    // 配置定时器的时钟源，使用系统时钟

//     __EnableIRQ(TMR2_IRQn); // 使能中断
//     IE_EA = 1;              // 打开总中断
// }

#if 0  // void tmr2_disable(void)
/**
 * @brief 关闭定时器，清空计数值
 */
void tmr2_disable(void)
{
    // 不给定时器提供时钟，让它停止计数
    TMR2_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR2_CONL |= TMR_SOURCE_SEL(0x05);    // 配置定时器的时钟源，不用任何时钟

    TMR_ALLCON = TMR2_CNT_CLR(0x1); // 清除计数值

    __DisableIRQ(TMR2_IRQn); // 关闭中断（不使能中断）
}
#endif // void tmr2_disable(void)

extern void update_engine_speed_scan_data(void); // 更新检测发动机转速的数据
#if SPEED_SCAN_ENABLE
extern void update_speed_scan_data(void);
#endif // #if SPEED_SCAN_ENABLE
// TMR2中断服务函数
void TIMR2_IRQHandler(void) interrupt TMR2_IRQn
{
// 上升沿检测
#if ENGINE_SPEED_SCAN_ENABLE

    static volatile bit last_engine_speed_scan_level = 0; // 记录上一次检测到的引脚电平（发送机转速检测脚）

#endif // #if ENGINE_SPEED_SCAN_ENABLE

#if SPEED_SCAN_ENABLE

    static volatile bit last_speed_scan_level = 0; // 记录上一次检测到的引脚电平（时速检测脚）

#endif // #if SPEED_SCAN_ENABLE

    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR2_IRQn);
    // P20 = 1; // 测试中断持续时间(约3us)

    // ---------------- 用户函数处理 -------------------
    // 周期中断
    if (TMR2_CONH & TMR_PRD_PND(0x1))
    {
        TMR2_CONH |= TMR_PRD_PND(0x1); // 清除pending

#if ENGINE_SPEED_SCAN_ENABLE

        { // 记录发动机转速扫描的时间
            static u8 cnt = 0;
            cnt++;
            if (cnt >= 20) // 每1ms进入一次
            {
                cnt = 0;
                engine_speed_scan_ms++;

                if (engine_speed_scan_ms >= ENGINE_SPEED_SCAN_OVER_TIME &&
                    flag_is_engine_speed_scan_over_time == 0)
                {
                    engine_speed_scan_ms = 0;
                    flag_is_engine_speed_scan_over_time = 1; // 说明超时，脉冲计数一直没有加一
                }
            }
        }

        if (ENGINE_SPEED_SCAN_PIN) // 检测发动机转速的引脚
        {
            if (0 == last_engine_speed_scan_level)
            {
                // 如果之前检测到低电平，现在检测到高电平，说明有上升沿，对脉冲计数加一
                // if (detect_engine_pulse_cnt[0] < 4294967295) // 防止计数溢出
                // {
                //     detect_engine_pulse_cnt[0]++;
                // }
                engine_speed_scan_cnt++;
                update_engine_speed_scan_data();
            }

            last_engine_speed_scan_level = 1;
        }
        else
        {
            // 如果现在检测到低电平
            last_engine_speed_scan_level = 0;
        }

#endif // #if ENGINE_SPEED_SCAN_ENABLE

#if SPEED_SCAN_ENABLE
        { // 记录时速扫描的时间
            static u16 cnt = 0;
            cnt++;
            if (cnt >= 20) // 每1ms进入一次
            {
                cnt = 0;
                speed_scan_time_ms++; // 每1ms加一

                // 600ms，为了滤掉1Hz的脉冲，认为 1Hz==0km/h
                if (speed_scan_time_ms >= SPEED_SCAN_OVER_TIME &&
                    flag_is_speed_scan_over_time == 0)
                {
                    speed_scan_time_ms = 0;
                    flag_is_speed_scan_over_time = 1; // 说明超时，脉冲计数一直没有加一
                }
            }
        } // 记录时速扫描的时间

        if (SPEED_SCAN_PIN) // 检测时速的引脚
        {
            if (0 == last_speed_scan_level)
            {
#if 0  // 定时扫描脉冲个数来计算时速的方式
       // 如果之前检测到低电平，现在检测到高电平，说明有上升沿，对脉冲计数加一
                if (detect_speed_pulse_cnt[0] < 4294967295) // 防止计数溢出
                {
                    detect_speed_pulse_cnt[0]++;
                }
#endif // 定时扫描脉冲个数来计算时速的方式

                speed_pulse_cnt++;
                update_speed_scan_data();
            }

            last_speed_scan_level = 1;
        }
        else
        {
            // 如果现在检测到低电平
            last_speed_scan_level = 0;
        }
#endif
    }

    // P20 = 0; // 测试中断持续时间
    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR2_IRQn);
}
