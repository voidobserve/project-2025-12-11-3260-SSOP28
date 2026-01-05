#include "send_data.h"

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
    // 发送 远光灯 的状态（目前没有检测远光的引脚，用它来存放摩托车大灯的状态）
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
    // 发送大计里程， 数据 3 byte
    case SEND_TOTAL_MILEAGE_TENTH_OF_KM:
    {
        send_data_packet_len = 7; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;

        /*
            发送的数据直接用作屏幕显示，这里需要提前做好转换

            一个字节存储两位十进制数

            (tmp_val % 100 / 10) ，取十位的数，放在低4位
            (tmp_val % 10) << 4 ,取个位的数，放在高4位
        */
        tmp_val = fun_info.save_info.total_mileage / 1000; // 存放以 km 为单位的数据
        send_data_packet[5] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
        tmp_val /= 100;
        send_data_packet[4] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
        tmp_val /= 100;
        send_data_packet[3] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
    }
    break;
    // ====================================================================
    // 发送大计里程， 数据 3 byte
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
        tmp_val = fun_info.save_info.total_mileage / 1610; // 得到以 mile 为单位的数据
        send_data_packet[5] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
        tmp_val /= 100;
        send_data_packet[4] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
        tmp_val /= 100;
        send_data_packet[3] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
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
    case SEND_TEMP_OF_WATER_ALERT:
    {
        send_data_packet_len = 5;
        send_data_packet[1] = send_data_packet_len;
        send_data_packet[3] = fun_info.flag_is_in_water_temp_warning;
    }
    break;

    // ====================================================================
    // 发送 故障状态
    // case SEND_MALFUNCTION_STATUS:
    // {
    //     send_data_packet_len = 5;
    //     send_data_packet[1] = send_data_packet_len;
    //     send_data_packet[3] = fun_info.flag_is_detect_malfunction;
    // }
    // break;
    // ====================================================================
    // 发送 发送小计里程， 数据 3 byte
    case SEND_SUBTOTAL_MILEAGE_TENTH_OF_KM:
    {
        send_data_packet_len = 7; // 发送指令的总长度
        send_data_packet[1] = send_data_packet_len;

        /*
            发送的数据直接用作屏幕显示，这里需要提前做好转换
        */
        tmp_val = fun_info.save_info.subtotal_mileage / 100; // 存放以百米为单位的数据
        send_data_packet[5] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
        tmp_val /= 100;
        send_data_packet[4] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
        tmp_val /= 100;
        send_data_packet[3] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
    }
    break;
    // ====================================================================
    // 发送 发送小计里程， 数据 3 byte
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
        send_data_packet[5] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
        tmp_val /= 100;
        send_data_packet[4] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
        tmp_val /= 100;
        send_data_packet[3] = (tmp_val % 100 / 10) | (tmp_val % 10) << 4;
    }
    break;
    // ====================================================================
    /*
        发送 小计里程， 数据 2 byte
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
