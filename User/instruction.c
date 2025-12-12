#include "instruction.h"

#include <string.h> // memset()

// 用bit定义，来节省空间
// 下面的标志位，有可能是从串口接收到了指令来置位，也有可能是其他的扫描函数更新了状态来置位
volatile bit flag_get_all_status; // 获取所有功能的状态
volatile bit flag_get_gear;       // 获取挡位状态 / 得到了挡位的状态
volatile bit flag_get_battery;    // 获取电池状态 / 得到了电池的状态（电池电量，单位：百分比）
// volatile bit flag_get_brake;        // 获取刹车状态 / 得到了刹车的状态
volatile bit flag_get_left_turn;  // 获取左转向灯的状态 / 得到了左转向灯的状态
volatile bit flag_get_right_turn; // 获取右转向灯的状态 / 得到了右转向灯的状态
// volatile bit flag_get_high_beam;    // 获取远光灯的状态 / 得到了远光灯的状态
volatile bit flag_get_engine_speed; // 获取发动机的转速 / 得到了发动机的转速
volatile bit flag_get_speed;        // 获取时速 / 得到了时速
volatile bit flag_get_fuel;         // 获取油量 / 得到了油量（单位：百分比）
#if TEMP_OF_WATER_SCAN_ENABLE
// volatile bit flag_get_temp_of_water = 0; // 获取水温 / 得到了水温
#endif
volatile bit flag_update_malfunction_status; // 标志位，更新故障状态
volatile bit flag_update_abs_status;         // 标志位，更新abs的状态

volatile bit flag_get_total_mileage;       // 获取大计里程 / 得到了大计里程(数据需要更新)
volatile bit flag_get_sub_total_mileage;   // 获取小计里程 / 得到了小计里程(数据需要更新)
volatile bit flag_get_sub_total_mileage_2; // 获取小计里程2 / 得到了小计里程2(数据需要更新)

// volatile bit flag_get_touch_key_status = 0; // 获取触摸按键的状态
// volatile bit flag_alter_date = 0; // 修改日期
volatile bit flag_alter_time; // 修改时间

volatile bit flag_get_voltage_of_battery; // 获取电池电压

// #if TEMP_OF_WATER_SCAN_ENABLE
volatile bit flag_set_temp_of_water_warning = 0; // 设置水温报警
// #endif

volatile bit flag_clear_total_mileage;       // 清除大计里程
volatile bit flag_clear_sub_total_mileage;   // 清除小计里程
volatile bit flag_clear_sub_total_mileage_2; // 清除小计里程2

volatile u8 synchronous_request_status;    // 同步请求状态机
volatile u16 synchronous_request_time_cnt; // 同步请求时间计时

volatile u8 update_time_status; // 更新时间的状态机
// volatile u8 update_date_status = 0; // 更新日期的状态机

volatile u16 update_time_cooling_cnt; // 更新时间的冷却计数
// volatile u16 update_date_cooling_cnt = 0; // 更新日期的冷却计数

// // 存放接收到的设置水温报警的指令中，对应的操作，默认为无操作
// volatile u8 operation_set_temp_of_water_warning = OPERATION_SET_TEMP_OF_WATER_WARNING_NONE;

// 检查接收是否正确的函数，如果接收正确，
// 根据接收到的数据中的指令，给对应的指令的标志位置一
void instruction_scan(void)
{
    u8 i = 0; // 注意要大于等于缓冲区能存放的指令数目
    for (i = 0; i < (UART0_RXBUF_LEN) / (FRAME_MAX_LEN); i++)
    {
        if (flagbuf_valid_instruction[i]) // 如果对应的位置有合法的数据帧
        {
            // 根据接收到数据中的指令，给对应的指令的标志位置一
            // if (4 == uart0_recv_buf[i][1]) // 如果是四位长度的指令
            {
                switch (uart0_recv_buf[i][2]) // 根据不同的指令来给对应的标志位置一
                {
                case INSTRUCTION_GET_ALL_STATUS: // 获取所有功能的状态
                    flag_get_all_status = 1;
                    break;

                case INSTRUCTION_GET_TOTAL_MILEAGE: // 获取大计里程
                    flag_get_total_mileage = 1;
                    break;

                case INSTRUCTION_GET_SUBTOTAL_MILEAGE: // 获取小计里程
                    flag_get_sub_total_mileage = 1;
                    break;

                case INSTRUCTION_GET_SUBTOTAL_MILEAGE_2: // 获取小计里程2
                    flag_get_sub_total_mileage_2 = 1;
                    break;

                case INSTRUCTION_ALTER_TIME: // 修改时间
                    flag_alter_time = 1;

                    fun_info.aip1302_saveinfo.year = (u16)((u32)uart0_recv_buf[i][3] << 8) +
                                                     (u16)uart0_recv_buf[i][4];
                    fun_info.aip1302_saveinfo.month = uart0_recv_buf[i][5];
                    fun_info.aip1302_saveinfo.day = uart0_recv_buf[i][6];

                    fun_info.aip1302_saveinfo.time_hour = uart0_recv_buf[i][7];
                    fun_info.aip1302_saveinfo.time_min = uart0_recv_buf[i][8];
                    fun_info.aip1302_saveinfo.time_sec = uart0_recv_buf[i][9];

#if USE_MY_DEBUG
#if 0 // 打印串口缓冲区的数据

                    {
                        u8 i = 0;
                        u8 j = 0;
                        for (i = 0; i < (UART0_RXBUF_LEN) / (FRAME_MAX_LEN); i++)
                        {
                            printf("buff_index %bu ", i);
                            for (j = 0; j < FRAME_MAX_LEN; j++)
                            {
                                printf("%bx ", uart0_recv_buf[i][j]);
                            }
                            printf("\n");
                        }
                    }

#endif // 打印串口缓冲区的数据
#endif

                    break;

                case INSTRUCTION_CLEAR_TOTAL_MILEAGE: // 清除大计里程
                    flag_clear_total_mileage = 1;
                    break;

                case INSTRUCTION_CLEAR_SUBTOTAL_MILEAGE: // 清除小计里程
                    flag_clear_sub_total_mileage = 1;
                    // printf("clear\n");
                    break;

                case INSTRUCTION_CLEAR_SUBTOTAL_MILEAGE_2: // 清除小计里程2
                    flag_clear_sub_total_mileage_2 = 1;
                    break;
                }

                if (recv_frame_cnt > 0) //
                {
                    recv_frame_cnt--; // 从串口接收的数据帧数目减一，表示指令已经从缓冲区取出
                }

                flagbuf_valid_instruction[i] = 0; // 清空缓冲区对应的元素，表示该下标的指令已经处理
                uart0_recv_len[i] = 0;
                recved_flagbuf[i] = 0;
                memset(uart0_recv_buf[i], 0, FRAME_MAX_LEN); // 清空缓冲区对应的元素
            } // if (4 == uart0_recv_buf[i][1])
        }
    }
}

void instruction_handle(void)
{ 
    if (flag_get_all_status)
    {
        // 如果要获取所有功能的状态
        // u32 temp_val = 0;        // 临时变量(在发送日期和时间使用到)
        flag_get_all_status = 0; // 清除标志位

#if USE_MY_DEBUG
        // printf(" flag_get_all_status\n");
#endif

        if (synchronous_request_status == SYN_REQUEST_STATUS_NONE)
        {
            // printf("handle get_all_status\n");// 测试通过，每次收到同步请求，都会等待冷却后，才处理下一次同步请求
            synchronous_request_status = SYN_REQUEST_STATUS_HANDLING;
            // 获取所有功能的状态，需要把这些功能对应的状态都发送出去
            send_data_packet(SEND_GEAR);            // 发送当前挡位的状态
            send_data_packet(SEND_BATTERY);         // 发送电池电量
            send_data_packet(SEND_ENGINE_SPEED);    // 发送发动机转速
            send_data_packet(SEND_SPEED);           // 发送当前采集到的车速（时速）
            send_data_packet(SEND_SPEED_WITH_MILE); // 发送时速（单位：英里每小时 mile/h） 1km/h == 0.621427mile/h
            send_data_packet(SEND_FUEL);            // 发送当前油量(单位：百分比)

            send_data_packet(SEND_TOTAL_MILEAGE_TENTH_OF_KM);   // 发送大计里程， 数据 3 byte ， 单位： 0.1 km
            send_data_packet(SEND_TOTAL_MILEAGE_TENTH_OF_MILE); // 发送大计里程， 数据 3 byte ， 单位： 0.1 mile

            send_data_packet(SEND_SUBTOTAL_MILEAGE_TENTH_OF_KM);   // 发送大计里程（总里程，单位：英里 mile），只发送1英里以上的数据
            send_data_packet(SEND_SUBTOTAL_MILEAGE_TENTH_OF_MILE); // 发送小计里程（总里程，单位：0.1 英里 0.1 mile），只发送0.1英里以上的数据

#if IC_1302_ENABLE
            aip1302_read_all(); // 先从aip1302时钟ic获取所有关于时间的信息，再发送

            send_data_packet(SEND_TIME);
            // printf("time send\n");
#endif // #if IC_1302_ENABLE

            // 发送当前的电池电压
            send_data_packet(SEND_VOLTAGE_OF_BATTERY);

            send_data_packet(SEND_TEMP_OF_WATER_ALERT);

            // send_data_packet(SEND_SUBTOTAL_MILEAGE_2);                    // 发送小计里程2(只发送百米及以上的数据)
            // send_data_packet(SEND_SUBTOTAL_MILEAGE_2_WITH_TENTH_OF_MILE); // 发送小计里程2(只发送0.1英里及以上的数据)
        }
    }

    if (flag_get_left_turn)
    {
        flag_get_left_turn = 0;
        send_data_packet(SEND_LEFT_TURN);
    }

    if (flag_get_right_turn)
    {
        flag_get_right_turn = 0;
        send_data_packet(SEND_RIGHT_TURN);
    }

    if (flag_get_gear) // 如果要获取挡位的状态
    {
        flag_get_gear = 0; //

#if USE_MY_DEBUG
        // printf(" flag_get_gear\n");
#endif

        send_data_packet(SEND_GEAR); // 发送当前挡位的状态
                                     // printf("cur gear %u\n", fun_info.gear);
    } //  if (flag_get_gear) // 如果要获取挡位的状态

    if (flag_get_battery)
    {
        // 如果要获取电池电量的状态
        flag_get_battery = 0;
#if USE_MY_DEBUG
        // printf(" flag_get_battery\n");
#endif

        send_data_packet(SEND_BATTERY); // 发送电池电量
    }

    if (flag_get_engine_speed)
    {
        // 如果要获取发动机的转速
        flag_get_engine_speed = 0;
#if USE_MY_DEBUG
        // printf(" flag_get_engine_speed \n");
        // printf(" cur engine speed %u rpm ", (u16)fun_info.engine_speeed);
#endif
        send_data_packet(SEND_ENGINE_SPEED); //
    }

    if (flag_get_speed)
    {
        // 如果要获取时速 （速度）
        flag_get_speed = 0;
#if USE_MY_DEBUG
        // printf(" flag_get_speed \n");
#endif

        send_data_packet(SEND_SPEED); // 发送当前采集到的车速（时速）
                                      // printf("cur speed %lu km/h\n", fun_info.speed);

        // 1km/h == 0.621427mile/h
        send_data_packet(SEND_SPEED_WITH_MILE);
    }

    if (flag_get_fuel)
    {
        // 如果要获取油量
        flag_get_fuel = 0;

#if USE_MY_DEBUG
        // printf(" flag_get_fuel \n");
#endif

        send_data_packet(SEND_FUEL);
    }

    if (flag_update_malfunction_status) // 更新故障的状态
    {
        flag_update_malfunction_status = 0;
        send_data_packet(SEND_MALFUNCTION_STATUS);

        // 没有引脚检测ABS，那么检测到故障之后，也发送ABS的状态
        // send_data_packet(SEND_ABS_STATUS);
    }

    if (flag_get_total_mileage) // 如果要获取大计里程 / 得到了大计里程新的数据
    {
        flag_get_total_mileage = 0;
#if USE_MY_DEBUG
        // printf(" flag_get_total_mileage \n");
        // printf(" cur total mileage %lu m \n", (u32)fun_info.save_info.total_mileage);
#endif

        // 只发送千米及以上的数据
        // send_data_packet(SEND_TOTAL_MILEAGE_TENTH_OF_KM);
        // send_data_packet(SEND_TOTAL_MILEAGE_WITH_MILE); // 1km == 0.621427mile
    }

    if (flag_get_sub_total_mileage) // 如果要获取小计里程 / 得到了小计里程新的数据
    {

        flag_get_sub_total_mileage = 0;
        // printf(" cur subtotal mileage %lu m \n", (u32)fun_info.save_info.subtotal_mileage);

        // 只发送百米及以上的数据
        send_data_packet(SEND_TOTAL_MILEAGE_TENTH_OF_KM);

        // 1km == 0.621427mile，1km == 6.21427 * 0.1 mile
        // send_data_packet(SEND_SUBTOTAL_MILEAGE_WITH_TENTH_OF_MILE);
    }

    if (flag_get_sub_total_mileage_2) // // 如果要获取小计里程2 / 得到了小计里程2新的数据
    {
        flag_get_sub_total_mileage_2 = 0;
        // 只发送百米及以上的数据
        // send_data_packet(SEND_SUBTOTAL_MILEAGE_2);
        // 1km == 0.621427mile，1km == 6.21427 * 0.1 mile
        // send_data_packet(SEND_SUBTOTAL_MILEAGE_2_WITH_TENTH_OF_MILE);
    }

    if (flag_set_temp_of_water_warning)
    {
        flag_set_temp_of_water_warning = 0;

        send_data_packet(SEND_TEMP_OF_WATER_ALERT);
    }

    if (flag_set_temp_of_water_warning)
    {
        flag_set_temp_of_water_warning = 0;

        send_data_packet(SEND_TEMP_OF_WATER_ALERT);
    }

#if IC_1302_ENABLE
    if (flag_alter_time)
    {
        // 如果要修改时间
        flag_alter_time = 0;

        // printf("get update time cmd\n");

#if USE_MY_DEBUG
        // printf(" flag_alter_time \n");
        // printf("==================================\n");
        // printf("year %u \n", fun_info.aip1302_saveinfo.year);
        // printf("month %bu \n", fun_info.aip1302_saveinfo.month);
        // printf("day %bu \n", fun_info.aip1302_saveinfo.day);
        // printf("hour %bu min %bu sec %bu \n", fun_info.aip1302_saveinfo.time_hour, fun_info.aip1302_saveinfo.time_min, fun_info.aip1302_saveinfo.time_sec);
#endif

        if (update_time_status == UPDATE_STATUS_NONE)
        {
            // printf("handle update_time \n");
            // 如果更新时间的状态已经冷却完毕，更新时间，并且进入冷却状态，
            // 切换状态，让定时器进行冷却计时
            aip1302_update_time(fun_info.aip1302_saveinfo); // 将时间更新到时钟ic
            update_time_status = UPDATE_STATUS_HANDLING;
            // printf("time updated\n");
        }
    }
#endif // #if IC_1302_ENABLE

    if (flag_get_voltage_of_battery)
    {
        // 如果要获取电池电压
        flag_get_voltage_of_battery = 0;
#if USE_MY_DEBUG
        // printf(" flag_get_voltage_of_battery \n");
        // printf(" cur voltage of battery %u \n", (u16)fun_info.voltage_of_battery);
#endif
        send_data_packet(SEND_VOLTAGE_OF_BATTERY);
    }

    if (flag_clear_total_mileage)
    {
        // 如果要清除大计里程
        flag_clear_total_mileage = 0;
        fun_info.save_info.total_mileage = 0;
        distance = 0;    //
        fun_info_save(); // 将信息写回flash
#if USE_MY_DEBUG
        // printf(" flag_clear_total_mileage \n");
#endif
    }

    if (flag_clear_sub_total_mileage)
    {
        // 如果要清除小计里程
        flag_clear_sub_total_mileage = 0;
        fun_info.save_info.subtotal_mileage = 0;
        distance = 0;
        fun_info_save(); // 将信息写回flash
#if USE_MY_DEBUG
        // printf(" flag_clear_sub_total_mileage \n");
#endif
    }

    if (flag_clear_sub_total_mileage_2) // 如果要清除小计里程2
    {
        flag_clear_sub_total_mileage_2 = 0;
        fun_info.save_info.subtotal_mileage_2 = 0;
        distance = 0;
        fun_info_save(); // 将信息写回flash
    }
}
