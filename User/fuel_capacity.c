#include "fuel_capacity.h"

#if FUEL_CAPACITY_SCAN_ENABLE


volatile u32 fuel_capacity_scan_cnt = 0; // 扫描时间计数，在1ms定时器中断中累加
// volatile u32 fuel_adc_val = 0;
// volatile u32 fuel_adc_scan_cnt = 0; // 在更新时间到来前，记录adc扫描的次数

// volatile u8 fuel_percent = 0xFF;

// static volatile u8 last_fuel_percent = 0xFF; // 记录上一次检测到的油量百分比
// 旧版到的油量检测程序：
#if 0
void fuel_capacity_scan(void)
{
    adc_sel_pin(ADC_PIN_FUEL); // 内部至少占用1ms
    adc_val = adc_getval();
    // printf("fuel adc %u \n", adc_val);

    fuel_adc_val += adc_val;
    fuel_adc_scan_cnt++;

    // fuel_capacity_scan_cnt += ONE_CYCLE_TIME_MS;
    if (fuel_capacity_scan_cnt >= FUEL_CAPACITY_SCAN_TIME_MS)
    {
        // 如果到了扫描更新时间，
        // bit flag_is_update_percent = 1; // 是否更新百分比,0--不更新,1--更新
        fuel_capacity_scan_cnt = 0;
        fuel_adc_val /= fuel_adc_scan_cnt; // 求出扫描时间内得到的ad平均值
        fuel_adc_scan_cnt = 0;
        // printf("fuel adc val %lu \n", fuel_adc_val);

#ifdef USE_MY_DEBUG
#if USE_MY_DEBUG
        // printf("fuel adc val %lu \n", fuel_adc_val);
#endif // #if USE_MY_DEBUG
#endif // #ifdef USE_MY_DEBUG

        // 先确定油量百分比的大致范围：
        if (fuel_adc_val < FUEL_MAX_ADC_VAL + (FUEL_90_PERCENT_ADC_VAL - FUEL_MAX_ADC_VAL) / 3)
        {
            fuel_percent = 100;
        }
        else if (fuel_adc_val < (FUEL_90_PERCENT_ADC_VAL - (FUEL_90_PERCENT_ADC_VAL - FUEL_MAX_ADC_VAL) / 3))
        {
            fuel_percent = 90;
        }
        else if (fuel_adc_val < (FUEL_80_PERCENT_ADC_VAL - (FUEL_80_PERCENT_ADC_VAL - FUEL_90_PERCENT_ADC_VAL) / 3))
        {
            fuel_percent = 80;
        }
        else if (fuel_adc_val < (FUEL_70_PERCENT_ADC_VAL - (FUEL_70_PERCENT_ADC_VAL - FUEL_80_PERCENT_ADC_VAL) / 3))
        {
            fuel_percent = 70;
        }
        else if (fuel_adc_val < (FUEL_60_PERCENT_ADC_VAL - (FUEL_60_PERCENT_ADC_VAL - FUEL_70_PERCENT_ADC_VAL) / 3))
        {
            fuel_percent = 60;
        }
        else if (fuel_adc_val < (FUEL_50_PERCENT_ADC_VAL - (FUEL_50_PERCENT_ADC_VAL - FUEL_60_PERCENT_ADC_VAL) / 3))
        {
            fuel_percent = 50;
        }
        else if (fuel_adc_val < (FUEL_40_PERCENT_ADC_VAL - (FUEL_40_PERCENT_ADC_VAL - FUEL_50_PERCENT_ADC_VAL) / 3))
        {
            fuel_percent = 40;
        }
        else if (fuel_adc_val < (FUEL_30_PERCENT_ADC_VAL - (FUEL_30_PERCENT_ADC_VAL - FUEL_40_PERCENT_ADC_VAL) / 3))
        {
            fuel_percent = 30;
        }
        else if (fuel_adc_val < (FUEL_20_PERCENT_ADC_VAL - (FUEL_20_PERCENT_ADC_VAL - FUEL_30_PERCENT_ADC_VAL) / 3))
        {
            fuel_percent = 20;
        }
        else if (fuel_adc_val < (FUEL_10_PERCENT_ADC_VAL - (FUEL_10_PERCENT_ADC_VAL - FUEL_20_PERCENT_ADC_VAL) / 3))
        {
            fuel_percent = 10;
        }
        else
        {
            fuel_percent = 0;
        }

#ifdef USE_MY_DEBUG
#if USE_MY_DEBUG
        // printf("fuel percent nearly %bu\n", fuel_percent);
#endif // #if USE_MY_DEBUG
#endif // #ifdef USE_MY_DEBUG

        // 再根据死区限制油量百分比
        if (fuel_adc_val > FUEL_MIN_ADC_VAL - ((FUEL_MIN_ADC_VAL - FUEL_10_PERCENT_ADC_VAL) / 3))
        {
            // 0%油量
            fuel_percent = 0;
        }
        else if (fuel_adc_val < (FUEL_10_PERCENT_ADC_VAL + (FUEL_MIN_ADC_VAL - FUEL_10_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_10_PERCENT_ADC_VAL - (FUEL_10_PERCENT_ADC_VAL - FUEL_20_PERCENT_ADC_VAL) / 3)
        {
            // 10%油量
            fuel_percent = 10;
        }
        else if (fuel_adc_val < (FUEL_20_PERCENT_ADC_VAL + (FUEL_10_PERCENT_ADC_VAL - FUEL_20_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_20_PERCENT_ADC_VAL - (FUEL_20_PERCENT_ADC_VAL - FUEL_30_PERCENT_ADC_VAL) / 3)
        {
            // 20%油量
            fuel_percent = 20;
        }
        else if (fuel_adc_val < (FUEL_30_PERCENT_ADC_VAL + (FUEL_20_PERCENT_ADC_VAL - FUEL_30_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_30_PERCENT_ADC_VAL - (FUEL_30_PERCENT_ADC_VAL - FUEL_40_PERCENT_ADC_VAL) / 3)
        {
            // 30%油量
            fuel_percent = 30;
        }
        else if (fuel_adc_val < (FUEL_40_PERCENT_ADC_VAL + (FUEL_30_PERCENT_ADC_VAL - FUEL_40_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_40_PERCENT_ADC_VAL - (FUEL_40_PERCENT_ADC_VAL - FUEL_50_PERCENT_ADC_VAL) / 3)
        {
            // 40%油量
            fuel_percent = 40;
        }
        else if (fuel_adc_val < (FUEL_50_PERCENT_ADC_VAL + (FUEL_40_PERCENT_ADC_VAL - FUEL_50_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_50_PERCENT_ADC_VAL - (FUEL_50_PERCENT_ADC_VAL - FUEL_60_PERCENT_ADC_VAL) / 3)
        {
            // 50%油量
            fuel_percent = 50;
        }

        else if (fuel_adc_val < (FUEL_60_PERCENT_ADC_VAL + (FUEL_50_PERCENT_ADC_VAL - FUEL_60_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_60_PERCENT_ADC_VAL - (FUEL_60_PERCENT_ADC_VAL - FUEL_70_PERCENT_ADC_VAL) / 3)
        {
            // 60%油量
            fuel_percent = 60;
        }
        else if (fuel_adc_val < (FUEL_70_PERCENT_ADC_VAL + (FUEL_60_PERCENT_ADC_VAL - FUEL_70_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_70_PERCENT_ADC_VAL - (FUEL_70_PERCENT_ADC_VAL - FUEL_80_PERCENT_ADC_VAL) / 3)
        {
            // 70%油量
            fuel_percent = 70;
        }
        else if (fuel_adc_val < (FUEL_80_PERCENT_ADC_VAL + (FUEL_70_PERCENT_ADC_VAL - FUEL_80_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_80_PERCENT_ADC_VAL - (FUEL_80_PERCENT_ADC_VAL - FUEL_90_PERCENT_ADC_VAL) / 3)
        {
            // 80%油量
            fuel_percent = 80;
        }
        else if (fuel_adc_val < (FUEL_90_PERCENT_ADC_VAL + (FUEL_80_PERCENT_ADC_VAL - FUEL_90_PERCENT_ADC_VAL) / 3) &&
                 fuel_adc_val > FUEL_90_PERCENT_ADC_VAL - (FUEL_90_PERCENT_ADC_VAL - FUEL_MAX_ADC_VAL) / 3)
        {
            // 90%油量
            fuel_percent = 90;
        }
        else if (fuel_adc_val < (FUEL_MAX_ADC_VAL + ((FUEL_90_PERCENT_ADC_VAL - FUEL_MAX_ADC_VAL) / 3)))
        {
            // 100%油量
            fuel_percent = 100;
        }
        else
        {
            // 如果检测到的ad值不在死区范围内,不更新油量
            // flag_is_update_percent = 0;
        }


        // printf("fuel percent %bu\n", fuel_percent);
#ifdef USE_MY_DEBUG
#if USE_MY_DEBUG
        // printf("fuel percent %bu\n", fuel_percent);
#endif // #if USE_MY_DEBUG
#endif // #ifdef USE_MY_DEBUG

        fun_info.fuel = fuel_percent;
        fuel_adc_val = 0xFF;
        flag_get_fuel = 1;
    } // if (fuel_capacity_scan_cnt >= FUEL_CAPACITY_SCAN_TIME_MS)
}
#endif

// 滑动平均：
#define SAMPLE_COUNT 20 // 样本计数
static volatile u16 samples[SAMPLE_COUNT] = {0};
static volatile u8 sample_index = 0;
u16 get_filtered_adc(u16 adc_val)
{
    u8 i = 0;
    u32 sum = 0;
    samples[sample_index++] = adc_val;
    if (sample_index >= SAMPLE_COUNT)
        sample_index = 0;

    for (i = 0; i < SAMPLE_COUNT; i++)
        sum += samples[i];

    return sum / SAMPLE_COUNT;
}

// 给滑动平均使用到的数组进行初始化
void samples_init(u16 adc_val)
{
    u8 i = 0;
    for (; i < SAMPLE_COUNT; i++)
    {
        samples[i] = adc_val;
    }
}

// 将油量检测对应的ad值转换成百分比值
u8 convert_fuel_adc_to_percent(u16 fuel_adc_val)
{
#if 0
    u8 ret = 0;

    // 如果超出了 最大油量的ad值和最小油量的ad值之间的范围 ，说明没有接油量检测
    if (fuel_adc_val >= (4095 - 500) ||
        fuel_adc_val <= (0 + 500))
    {
        ret = 0xFF; // 根据串口收发协议，0xFF对应没有接油量检测
    }
    else
    {
        if (fuel_adc_val > FUEL_MIN_ADC_VAL) // 如果检测到的ad值比最小油量对应的ad值还要小
        {
            ret = 0; // 0% 油量
        }
        else if (fuel_adc_val < FUEL_MAX_ADC_VAL) // 如果检测到的油量比100%还要大一些
        {
            ret = 100;
        }
        else
        {
            // u16 tmp_val = (FUEL_MAX_ADC_VAL - FUEL_MIN_ADC_VAL) / 100; /* 将油量最大的ad值和油量最小对应的ad值 划成100份 */
            // ret = (fuel_adc_val - FUEL_MIN_ADC_VAL) / tmp_val;

            u16 tmp_val = (FUEL_MIN_ADC_VAL - FUEL_MAX_ADC_VAL) / 100; /* 将油量最大的ad值和油量最小对应的ad值 划成100份 */
            ret = (fuel_adc_val - FUEL_MAX_ADC_VAL) / tmp_val;
        }
    }

    return ret;
#endif

#if 1
    u8 ret = 0;

    // // 如果超出了 最大油量的ad值和最小油量的ad值之间的范围 ，说明没有接油量检测
    // if (fuel_adc_val >= (4095 - 500) ||
    //     fuel_adc_val <= (0 + 500))
    // {
    //     ret = 0xFF; // 根据串口收发协议，0xFF对应没有接油量检测
    // }

    // 使用3V参考电压：
    if (fuel_adc_val >= (u16)((u32)2207 * 4096 / 3 / 1000) ||
        fuel_adc_val <= (u16)((u32)250 * 4096 / 3 / 1000))
    {
        ret = 0xFF; // 根据客户的修改需求，单片机检测到电压大于 2.207 V或者小于 0.25V ,认为是没有接油量，小于 2.207 V，认为接油量检测正常
    }
    else
    {
        if (fuel_adc_val > FUEL_LEVEL_0_ADC_VAL) // 如果检测到的ad值比最小油量对应的ad值还要大
        {
            ret = 0; // 0% 油量
        }
        else if (fuel_adc_val <= FUEL_LEVEL_6_ADC_VAL) // 如果检测到的油量比100%还要大一些
        {
            ret = 100;
        }
        else if (fuel_adc_val <= FUEL_LEVEL_0_ADC_VAL && fuel_adc_val > FUEL_LEVEL_1_ADC_VAL) /* 0格油量到1格油量，0% ~ 18% */
        {
            /*
                用采集到的ad值减去当前挡位最小的ad值，再除以（当前挡位最大的ad值-当前挡位最小的ad值），得到
                采集到的ad值对应当前挡位的占比，再乘以当前挡位对应的百分比
            */
            ret = ((u32)fuel_adc_val - FUEL_LEVEL_1_ADC_VAL) * 18 / ((u32)FUEL_LEVEL_0_ADC_VAL - FUEL_LEVEL_1_ADC_VAL);
            ret = 18 - ret;
        }
        else if (fuel_adc_val <= FUEL_LEVEL_1_ADC_VAL && fuel_adc_val > FUEL_LEVEL_2_ADC_VAL) /* 1格油量到2格油量，18% ~ 34% */
        {
            ret = ((u32)fuel_adc_val - FUEL_LEVEL_2_ADC_VAL) * 34 / ((u32)FUEL_LEVEL_1_ADC_VAL - FUEL_LEVEL_2_ADC_VAL);
            ret = 34 - ret;
            if (ret < 18)
            {
                ret = 18;
            }
        }
        else if (fuel_adc_val <= FUEL_LEVEL_2_ADC_VAL && fuel_adc_val > FUEL_LEVEL_3_ADC_VAL) /* 2格油量到3格油量，34% ~ 51% */
        {
            ret = ((u32)fuel_adc_val - FUEL_LEVEL_3_ADC_VAL) * 51 / ((u32)FUEL_LEVEL_2_ADC_VAL - FUEL_LEVEL_3_ADC_VAL);
            ret = 51 - ret;
            if (ret < 34)
            {
                ret = 34;
            }
        }
        else if (fuel_adc_val <= FUEL_LEVEL_3_ADC_VAL && fuel_adc_val > FUEL_LEVEL_4_ADC_VAL) /* 3格油量到4格油量，51% ~ 68% */
        {
            ret = ((u32)fuel_adc_val - FUEL_LEVEL_4_ADC_VAL) * 68 / ((u32)FUEL_LEVEL_3_ADC_VAL - FUEL_LEVEL_4_ADC_VAL);
            ret = 68 - ret;
            if (ret < 51)
            {
                ret = 51;
            }
        }
        else if (fuel_adc_val <= FUEL_LEVEL_4_ADC_VAL && fuel_adc_val > FUEL_LEVEL_5_ADC_VAL) /* 4格油量到5格油量，68% ~ 84% */
        {
            ret = ((u32)fuel_adc_val - FUEL_LEVEL_5_ADC_VAL) * 84 / ((u32)FUEL_LEVEL_4_ADC_VAL - FUEL_LEVEL_5_ADC_VAL);
            ret = 84 - ret;
            if (ret < 68)
            {
                ret = 68;
            }
        }
        else if (fuel_adc_val <= FUEL_LEVEL_5_ADC_VAL && fuel_adc_val > FUEL_LEVEL_6_ADC_VAL) /* 5格油量到6格油量，84% ~ 100.0% */
        {
            ret = ((u32)fuel_adc_val - FUEL_LEVEL_6_ADC_VAL) * 100 / ((u32)FUEL_LEVEL_5_ADC_VAL - FUEL_LEVEL_6_ADC_VAL);
            ret = 100 - ret;
            if (ret < 84)
            {
                ret = 84;
            }
        }
    }

    // 0，显示0格，游标闪烁
    // 18以下，不含18，显示1格
    // ret = 18; // 18及以上，显示2格
    // ret = 34; // 34及以上，显示3格
    // ret = 51; // 51及以上，显示4格
    // ret = 68; // 68及以上，显示5格
    // ret = 84; // 84及以上，显示6格
    return ret;
#endif
}

enum
{
    STATUS_JUST_POWER_ON = 0, // 刚上电
    STATUS_IN_SERVICE,        // 运行中
};

void fuel_capacity_scan(void)
{
    u8 fuel_percent = 0;
    u16 fuel_adc_val = 0;

    adc_sel_pin(ADC_PIN_FUEL); // 内部至少占用1ms
    adc_val = adc_getval();    //
    fuel_adc_val = get_filtered_adc(adc_val);
    // printf("fuel_adc_val: %u\n", fuel_adc_val);

    // if (fuel_adc_val <= 4294967295 - 4095) // 防止计数溢出
    // {
    //     fuel_adc_val += adc_val;
    //     fuel_adc_scan_cnt++;
    // }

    /*
        刚上电直接获取一次，作为油量的状态
    */
    {
        static u8 status = STATUS_JUST_POWER_ON;
        if (STATUS_JUST_POWER_ON == status) // 如果是第一次上电
        {
            if (fuel_capacity_scan_cnt >= FUEL_UPDATE_TIME_WHEN_POWER_ON)
            {
                fuel_capacity_scan_cnt = 0;
                // fuel_adc_val /= fuel_adc_scan_cnt; // 求出扫描时间内得到的ad平均值
                adc_val = adc_getval(); //
                samples_init(adc_val);
                fuel_adc_val = adc_val;

                fuel_percent = convert_fuel_adc_to_percent(fuel_adc_val);

                // printf("power on, fuel_percent:%bu\n", fuel_percent);
                fun_info.fuel = fuel_percent;
                // fuel_adc_scan_cnt = 0;
                fuel_adc_val = 0;
                flag_get_fuel = 1;

                status = STATUS_IN_SERVICE;
            }
        }
    }

    if (fuel_capacity_scan_cnt >= FUEL_UPDATE_TIME)
    {
        // 如果到了扫描更新时间
        fuel_capacity_scan_cnt = 0;
        // fuel_adc_val /= fuel_adc_scan_cnt; // 求出扫描时间内得到的ad平均值
        // fuel_percent = convert_fuel_adc_to_percent(fuel_adc_val);

        fuel_percent = convert_fuel_adc_to_percent(fuel_adc_val);

        // printf("fuel_percent:%bu\n", fuel_percent);
        fun_info.fuel = fuel_percent;
        // fuel_adc_scan_cnt = 0;
        fuel_adc_val = 0;
        flag_get_fuel = 1;
    } //  if (fuel_capacity_scan_cnt >= FUEL_UPDATE_TIME)
}

#endif

