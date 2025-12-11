#include "send_data.h"

#if 0
/**
 * @brief 通过串口发送数据
 *
 * @param instruct 指令
 * @param send_data 待发送的数据，如果要发送时间，该参数无效，因为时间信息一共7个字节
 */
void send_data(SEND_DATA_CMD_T instruct, u32 send_data)
{
    u32 check_num = 0; // 存放校验和

    uart0_sendbyte(FORMAT_HEAD); // 先发送格式头

    check_num += FORMAT_HEAD; // 使用if()语句时，才使用这一条程序

    if (SEND_GEAR == instruct ||                /* 发送挡位的状态 */
        SEND_BATTERY == instruct ||             /* 发送电池电量的状态 */
        SEND_BARKE == instruct ||               /* 发送刹车的状态 */
        SEND_LEFT_TURN == instruct ||           /* 发送左转向灯的状态 */
        SEND_RIGHT_TURN == instruct ||          /* 发送右转向灯的状态 */
        SEND_HIGH_BEAM == instruct ||           /* 发送远光灯的状态 */
        SEND_FUEL == instruct ||                /* 发送油量 */
        SEND_WATER_TEMP == instruct ||          /* 发送水温 */
        SEND_TEMP_OF_WATER_ALERT == instruct || /* 发送水温报警 */
        SEND_MALFUNCTION_STATUS == instruct ||  /* 发送故障的状态 */
        SEND_ABS_STATUS == instruct             /* 发送ABS的状态 */
        )                                       // 如果指令的总长度只有5个字节
    {
        uart0_sendbyte(0x05);     // 发送指令的总长度
        uart0_sendbyte(instruct); // 发送指令
        uart0_sendbyte(send_data);

        check_num += 0x05 + (u8)instruct + (u8)send_data;
    }
    else if (SEND_ENGINE_SPEED == instruct ||    /* 发送发动机的转速 */
             SEND_SPEED == instruct ||           /* 发送时速 */
             SEND_SPEED_WITH_MILE == instruct || /* （单位：英里每小时 mile/h） */
             SEND_VOLTAGE_OF_BATTERY == instruct /* 发送电池电压 */
             )                                   // 如果指令的总长度为6个字节
    {
        uart0_sendbyte(0x06);           // 发送指令的总长度
        uart0_sendbyte(instruct);       // 发送指令
        uart0_sendbyte(send_data >> 8); // 发送信息
        uart0_sendbyte(send_data);      // 发送信息

        check_num += 0x06 + (u8)instruct + (u8)(send_data >> 8) + (u8)(send_data);
    }
    else if (
        // SEND_SUBTOTAL_MILEAGE_2 == instruct ||                    /* 发送小计里程2 */
        // SEND_TOTAL_MILEAGE_WITH_MILE == instruct ||               /* 发送大计里程（总里程，单位：英里 mile） */
        // SEND_SUBTOTAL_MILEAGE_WITH_TENTH_OF_MILE == instruct ||   /* 发送小计里程（总里程，单位：0.1 英里 0.1 mile） */
        // SEND_SUBTOTAL_MILEAGE_2_WITH_TENTH_OF_MILE == instruct || /* 发送小计里程2（总里程，单位：0.1 英里 0.1 mile） */
        SEND_TOUCH_KEY_STATUS == instruct /* 发送触摸按键的状态 */
        )                                 // 如果指令的总长度为7个字节
    {

        uart0_sendbyte(0x07);     // 发送指令的总长度
        uart0_sendbyte(instruct); // 发送指令

        uart0_sendbyte(send_data >> 16); // 发送信息
        uart0_sendbyte(send_data >> 8);  // 发送信息
        uart0_sendbyte(send_data);       // 发送信息

        check_num += 0x07 + (u8)instruct + (u8)(send_data >> 16) + (u8)(send_data >> 8) + (u8)send_data;
    }
    // else if (SEND_DATE == instruct /* 发送日期(年月日) */
    //          )                     // 如果指令的总长度为8个字节
    // {
    //     uart0_sendbyte(0x08);     // 发送指令的总长度
    //     uart0_sendbyte(instruct); // 发送指令

    //     uart0_sendbyte(send_data >> 24); // 发送信息
    //     uart0_sendbyte(send_data >> 16); // 发送信息
    //     uart0_sendbyte(send_data >> 8);  // 发送信息
    //     uart0_sendbyte(send_data);       // 发送信息

    //     check_num += 0x08 + (u8)instruct + (u8)(send_data >> 24) + (u8)(send_data >> 16) + (u8)(send_data >> 8) + (u8)send_data;
    // }
    else if (SEND_TIME == instruct /* 发送时间 */)
    {
        uart0_sendbyte(0x0B);      // 发送指令的总长度
        uart0_sendbyte(SEND_TIME); // 发送指令

        // uart0_sendbyte(send_data >> 24); // 发送信息
        // uart0_sendbyte(send_data >> 16); // 发送信息
        // uart0_sendbyte(send_data >> 8);  // 发送信息
        // uart0_sendbyte(send_data);       // 发送信息

        uart0_sendbyte(fun_info.aip1302_saveinfo.year >> 8);
        uart0_sendbyte(fun_info.aip1302_saveinfo.year & 0xFF);
        uart0_sendbyte(fun_info.aip1302_saveinfo.month);
        uart0_sendbyte(fun_info.aip1302_saveinfo.day);
        uart0_sendbyte(fun_info.aip1302_saveinfo.time_hour);
        uart0_sendbyte(fun_info.aip1302_saveinfo.time_min);
        uart0_sendbyte(fun_info.aip1302_saveinfo.time_sec);

        check_num += 0x0B +
                     (u8)SEND_TIME +
                     (u8)(fun_info.aip1302_saveinfo.year >> 8) +
                     (u8)(fun_info.aip1302_saveinfo.year & 0xFF) +
                     (u8)(fun_info.aip1302_saveinfo.month) +
                     (u8)(fun_info.aip1302_saveinfo.day) +
                     (u8)(fun_info.aip1302_saveinfo.time_hour) +
                     (u8)(fun_info.aip1302_saveinfo.time_min) +
                     (u8)(fun_info.aip1302_saveinfo.time_sec);
    }

    // check_num &= 0x0F;         // 取前面的数字相加的低四位
    check_num &= 0xFF;         // 取前面的数字相加的低八位
    uart0_sendbyte(check_num); // 发送校验和

    delay_ms(1);
    // delay_ms(10); // 每次发送完成后，延时10ms
}
#endif

void send_data_packet(SEND_DATA_CMD_T cmd)
{
    /*
        发送所使用的变量需要改成 volatile 类型，
        测试发现，如果没有用 volatile 修饰，
        接收端在校验阶段有小概率会得到计算的校验和与存放的校验和不一致的情况
    */ 
    static volatile u8 send_data_packet[SEND_DATA_BUFF_MAX_LEN];
    volatile u8 send_data_packet_len = 0;
    volatile u8 check_sum = 0;
    volatile u8 i = 0;
    volatile u32 tmp_val = 0;

    send_data_packet[0] = FORMAT_HEAD; // [0] ， 格式头
    send_data_packet[2] = cmd;         // [2] ， 指令

    switch (cmd)
    {
    // ====================================================================
    // 发送档位
    case SEND_GEAR:
    {
        send_data_packet_len = 5; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.gear;
    }
    break;
    // ====================================================================
    // 发送电池电量
    case SEND_BATTERY:
    {
        send_data_packet_len = 5; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.battery;
    }
    break;
    // ====================================================================
    // 发送刹车的状态
    case SEND_BARKE:
    {
        send_data_packet_len = 5;
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.brake;
    }
    break;
    // ====================================================================
    // 发送左转向灯的状态
    case SEND_LEFT_TURN:
    {
        send_data_packet_len = 5;
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.left_turn;
    }
    break;

    // ====================================================================
    // 发送右转向灯的状态
    case SEND_RIGHT_TURN:
    {
        send_data_packet_len = 5;
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.right_turn;
    }
    break;
    // ====================================================================
    // 发送 远光灯 的状态
    case SEND_HIGH_BEAM:
    {
        send_data_packet_len = 5;
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.high_beam;
    }
    break;
    // ====================================================================
    // 发送 发动机的转速
    case SEND_ENGINE_SPEED:
    {
        send_data_packet_len = 6;
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.engine_speeed >> 8;
        send_data_packet[4] = fun_info.engine_speeed;
    }
    break;
    // ====================================================================
    // 发送 时速
    case SEND_SPEED:
    {
        // 限制要发送的时速:
        if (fun_info.speed > 255)
        {
            fun_info.speed = 255;
        }

        send_data_packet_len = 5;
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = (u8)(fun_info.speed & 0xFF);
    }
    break;
    // ====================================================================
    // 发送 油量
    case SEND_FUEL:
    {
        send_data_packet_len = 5;
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.fuel;
    }
    break;
    // ====================================================================
    // // 发送 水温
    // case SEND_WATER_TEMP:
    // {
    //     send_data_packet_len = 5;
    //     send_data_packet[1] = send_data_packet_len;
    //     send_data_packet[3] = fun_info.temp_of_water;
    // }
    // break;
    // ====================================================================
    // 发送大计里程， 数据 3 byte ， 数据直接用作屏幕显示，例如 0d 04 17 65 ，显示 041765 km
    case SEND_TOTAL_MILEAGE_TENTH_OF_KM:
    {
        send_data_packet_len = 7; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;

        /*
            发送的数据直接用作屏幕显示，这里需要提前做好转换
        */
        tmp_val = fun_info.save_info.total_mileage / 1000; // 存放以 km 为单位的数据
        send_data_packet[5] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[4] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[3] = tmp_val % 100;
    }
    break;
    // ====================================================================
    // 发送大计里程， 数据 3 byte ， 数据直接用作屏幕显示，例如 0d 04 17 65 ，显示 041765 mile
    case SEND_TOTAL_MILEAGE_TENTH_OF_MILE:
    {
        send_data_packet_len = 7; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;

        /*
            发送的数据直接用作屏幕显示，这里需要提前做好转换

            1 km == 0.621371 mile
            1 / 0.621371 约为 1.6093444978925633800096882538773，
            这里取 1.61 作为转换系数
            1000 m / 161 ，得到 以 0.1 mile 为单位的数据
        */
        tmp_val = fun_info.save_info.total_mileage / 1610 ; // 得到以 mile 为单位的数据
        send_data_packet[5] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[4] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[3] = tmp_val % 100;
    }
    break;
    // ====================================================================
    // 发送 时间（年月日，时分秒）
    case SEND_TIME:
    {
        send_data_packet_len = 11; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;

        send_data_packet[3] = fun_info.aip1302_saveinfo.year >> 8;
        send_data_packet[4] = fun_info.aip1302_saveinfo.year & 0xFF;
        send_data_packet[5] = fun_info.aip1302_saveinfo.month;
        send_data_packet[6] = fun_info.aip1302_saveinfo.day;
        send_data_packet[7] = fun_info.aip1302_saveinfo.time_hour;
        send_data_packet[8] = fun_info.aip1302_saveinfo.time_min;
        send_data_packet[9] = fun_info.aip1302_saveinfo.time_sec;
    }
    break;
    // ====================================================================
    // 发送 电池电压
    case SEND_VOLTAGE_OF_BATTERY:
    {
        send_data_packet_len = 6; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;

        send_data_packet[3] = fun_info.voltage_of_battery >> 8;
        send_data_packet[4] = fun_info.voltage_of_battery & 0xFF;
    }
    break;
    // ====================================================================
    // 发送 水温报警
    // case SEND_TEMP_OF_WATER_ALERT:
    // {
    //     send_data_packet_len = 5;
    //     send_data_packet[1] = send_data_packet_len;
    //     // send_data_packet[3] = ;
    // }
    // ====================================================================
    // 发送 发送小计里程， 数据 3 byte ， 数据直接用作屏幕显示，例如 0d 04 17 65 ，显示 04176.5 km
    case SEND_SUBTOTAL_MILEAGE_TENTH_OF_KM:
    {
        send_data_packet_len = 7; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;

        /*
            发送的数据直接用作屏幕显示，这里需要提前做好转换
        */
        tmp_val = fun_info.save_info.subtotal_mileage / 100; // 存放以百米为单位的数据
        send_data_packet[5] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[4] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[3] = tmp_val % 100;
    }
    break;
    // ====================================================================
    // 发送 发送小计里程， 数据 3 byte ， 数据直接用作屏幕显示，例如 0d 04 17 65 ，显示 04176.5 mile
    case SEND_SUBTOTAL_MILEAGE_TENTH_OF_MILE:
    {
        send_data_packet_len = 7; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;

        /*
            发送的数据直接用作屏幕显示，这里需要提前做好转换

            1 km == 0.621371 mile
            1 / 0.621371 约为 1.6093444978925633800096882538773，
            这里取 1.61 作为转换系数
            1000 m / 161 ，得到 以 0.1 mile 为单位的数据
        */
        tmp_val = fun_info.save_info.subtotal_mileage / 161; // 得到 以 0.1 mile 为单位的数据
        send_data_packet[5] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[4] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[3] = tmp_val % 100;
    }
    break;
    // ====================================================================
    /*
        发送 小计里程， 数据 2 byte ， 直接用作屏幕显示，
        例如 0d 1765 ，显示 1765 km
    */
    case SEND_SUBTOTAL_MILEAGE_KM:
    {
        send_data_packet_len = 6;
        send_data_packet[1] = send_data_packet_len;

        tmp_val = fun_info.save_info.subtotal_mileage / 1000; // 得到以 km 为单位的数据
        send_data_packet[4] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[3] = tmp_val % 100;
    }
    break;
    // ====================================================================
    /*
        发送 小计里程， 数据 2 byte ， 直接用作屏幕显示，
        例如 0d 1765 ，显示 1765 mile
    */
    case SEND_SUBTOTAL_MILEAGE_MILE:
    {
        send_data_packet_len = 6;
        send_data_packet[1] = send_data_packet_len;

        tmp_val = fun_info.save_info.subtotal_mileage / 1610; // 得到以 mile 为单位数据
        send_data_packet[4] = tmp_val % 100;
        tmp_val /= 100;
        send_data_packet[3] = tmp_val % 100;
    }
    break;
    // ====================================================================
    // 发送 时速（单位：英里每小时 mile/h）
    case SEND_SPEED_WITH_MILE:
    {
        tmp_val = fun_info.speed * 621 / 1000; // （单位：英里每小时 mile/h） 1km/h == 0.621427mile/h
        if (tmp_val > 255)
        {
            tmp_val = 255; // 限制发送的时速
        }

        send_data_packet_len = 5;
        send_data_packet[1] = send_data_packet_len;

        send_data_packet[3] = (u8)(tmp_val & 0xFF);
    }
    break;

    default:
        break;
    }

    for (i = 0; i < send_data_packet_len; i++)
    {
        if (i == send_data_packet_len - 1)
        {
            // 数据包的最后一个字节，存放校验和
            send_data_packet[i] = check_sum;
        }
        uart0_sendbyte(send_data_packet[i]);
        check_sum += (u8)send_data_packet[i]; // 只用1个字节存放校验和，溢出的部分不用处理
    }

    delay_ms(1);
}
