#include "uart0.h"

#include <string.h> // memset()

// 设置的波特率需要适配单片机的时钟，这里直接使用了官方的代码
#define USER_UART0_BAUD ((SYSCLK - UART0_BAUDRATE) / (UART0_BAUDRATE))

// 用来存放接收的数据帧的缓冲区
volatile u8 uart0_recv_buf[(UART0_RXBUF_LEN) / (FRAME_MAX_LEN)][FRAME_MAX_LEN];
// 用于记录接收缓冲区的对应的数据帧的长度
volatile u8 uart0_recv_len[(UART0_RXBUF_LEN) / (FRAME_MAX_LEN)];
// 记录缓冲区中对应的位置是否有数据的标志位（数组）
// 例如，缓冲区下标0的位置有指令，标志位数组下标0的元素值为1，没有指令，元素的值为0
volatile u8 recved_flagbuf[(UART0_RXBUF_LEN) / (FRAME_MAX_LEN)];

volatile u8 flagbuf_valid_instruction[(UART0_RXBUF_LEN) / (FRAME_MAX_LEN)]; // 存放有合法指令的标志位数组

// 串口中断服务函数中，接收一帧的标志位，0--准备接收一帧的第一个字节，1--正在接收该帧的数据
static volatile bit flag_is_recving_data;
static volatile u8 frame_len;    // 记录串口中断服务函数中，当前要接收的数据帧的字节数
static volatile u8 cur_recv_len; // 记录串口中断服务函数中，当前已接收的数据帧的字节数
volatile u8 recv_frame_cnt;      // 接收到的数据帧的个数

static volatile u32 blank_index; // 记录当前存放数据帧的缓冲区的空的地方(缓冲区下标)，准备存放一帧的数据

/*
    如果串口收到了1字节数据，却未收完整一帧数据，会使能串口接收超时
    如果收到了完整的一帧数据，会关闭串口接收超时
*/
volatile bit flag_is_uart0_receive_timeout;        // 标志位，串口是否接收超时，0--未超时，1--已超时
volatile bit flag_is_uart0_receive_timeout_enable; // 标志位，是否使能串口接收超时，0--否，1--串口接收了数据，却未收到完整一帧数据，使能串口接收超时
volatile u8 uart0_receive_timeout_cnt;            // 存放串口接收超时计数值

#if USE_MY_DEBUG
#if 1 // 将uart0用作串口打印
// 重写putchar()函数
extern void uart0_sendbyte(u8 senddata); // 函数声明
char putchar(char c)
{
    uart0_sendbyte(c);
    return c;
}
#endif // 将uart0用作串口打印
#endif // #if USE_MY_DEBUG

static void uart0_receive_timeout_enable(void)
{
    uart0_receive_timeout_cnt = 0;            // 超时计数值清零
    flag_is_uart0_receive_timeout = 0;        // 表示未接收超时
    flag_is_uart0_receive_timeout_enable = 1; // 使能接收超时
}

static void uart0_receive_timeout_disable(void)
{
    flag_is_uart0_receive_timeout_enable = 0; // 不使能接收超时
    uart0_receive_timeout_cnt = 0;            // 超时计数值清零
    flag_is_uart0_receive_timeout = 0;        // 表示未接收超时
}

// uart0初始化
// 波特率由宏 UART0_BAUDRATE 来决定
void uart0_config(void)
{
// P03 为单片机TX，P00 为单片机RX
#if 1
    P0_MD0 &= ~(GPIO_P03_MODE_SEL(0x03)); // 清空寄存器配置
    P0_MD0 &= ~(GPIO_P00_MODE_SEL(0x03)); // 清空寄存器配置
    P0_MD0 |= GPIO_P03_MODE_SEL(0x01);    // 输出模式
    FOUT_S03 |= GPIO_FOUT_UART0_TX;       // 配置P03为UART0_TX
    FIN_S7 |= GPIO_FIN_SEL_P00;           // 配置P00为UART0_RX
#endif

    // // 旧版电路板上，P11为发送引脚，P12为接收引脚
    // P1_MD0 &= (~GPIO_P11_MODE_SEL(0x3) | ~GPIO_P12_MODE_SEL(0x3));
    // P1_MD0 |= GPIO_P11_MODE_SEL(0x1); // 输出模式
    // FOUT_S11 |= GPIO_FOUT_UART0_TX;   // 配置P11为UART0_TX
    // FIN_S7 |= GPIO_FIN_SEL_P12;       // 配置P12为UART0_RX

#if 0
    // 测试时，使用开发板上面的P25来打印输出
    P2_MD1 &= ~(GPIO_P25_MODE_SEL(0x03)); // 清空寄存器配置
    P2_MD1 |= GPIO_P25_MODE_SEL(0x01);    // 输出模式
    FOUT_S25 |= GPIO_FOUT_UART0_TX;       // 配置为UART0_TX
    // 测试时，使用开发板上面的P22来作为接收：
    P2_MD0 &= ~(GPIO_P22_MODE_SEL(0x03)); // 清空寄存器配置，输入模式
    FIN_S7 |= GPIO_FIN_SEL_P22;           // 配置为UART0_RX
#endif

    __EnableIRQ(UART0_IRQn); // 打开UART模块中断
    IE_EA = 1;               // 打开总中断

    UART0_BAUD1 = (USER_UART0_BAUD >> 8) & 0xFF; // 配置波特率高八位
    UART0_BAUD0 = USER_UART0_BAUD & 0xFF;        // 配置波特率低八位
    UART0_CON0 = UART_STOP_BIT(0x0) |
                 UART_RX_IRQ_EN(0x1) |
                 UART_EN(0x1); // 8bit数据，1bit停止位，使能RX中断
}

// UART0中断服务函数（接收中断）
void UART0_IRQHandler(void) interrupt UART0_IRQn
{
    volatile u8 uart0_tmp_val = 0;

    // 进入中断设置IP，不可删除
    __IRQnIPnPush(UART0_IRQn);
    // P20 = 1; // 测试中断持续时间

    // ---------------- 用户函数处理 -------------------
    // RX接收完成中断
    if (UART0_STA & UART_RX_DONE(0x1))
    {
        // test_bit = 1;
        uart0_tmp_val = UART0_DATA; // 临时存放接收到的数据

        if ((0 == flag_is_recving_data && uart0_tmp_val != 0xA5) ||
            (recv_frame_cnt >= ((UART0_RXBUF_LEN) / (FRAME_MAX_LEN))))
        {
            // 1. 如果是新的一帧数据，却不是以0xA5开头，说明这一帧数据无效
            // 2. 如果缓冲区已满，存不下更多的数据帧
            // 直接退出中断，不处理数据
            // tmr0_disable();
            // tmr0_cnt = 0;

            uart0_receive_timeout_disable(); // 不使能串口接收超时检测功能

            __IRQnIPnPop(UART0_IRQn);
            return;
        }
        // if ((0 == flag_is_recving_data && UART0_DATA == 0xA5) ||
        //     (1 == flag_is_recving_data && UART0_DATA == 0xA5))
        else if (uart0_tmp_val == FORMAT_HEAD)
        {
            // 1. 如果是新的一帧数据（以格式头0xA5开始），打开定时器，为超时判断做准备
            // 2. 如果正在接收一帧数据，却又收到了一次格式头，舍弃之前收到的数据，重新接收这一帧
            flag_is_recving_data = 1;
            cur_recv_len = 0;
            frame_len = 0;

            // tmr0_disable();
            // tmr0_cnt = 0;
            // tmr0_enable();
            uart0_receive_timeout_enable();
        }

        if (1 == cur_recv_len)
        {
            // 如果接收到一帧的第二个字节的数据，记录下要接收的数据长度
            frame_len = uart0_tmp_val;
            // test_val = frame_len;
            // printf("frame_len %d\n", (int32)frame_len);
        }

        // 程序运行到这里，说明正在接收一个数据帧
        if (0 == recved_flagbuf[blank_index])
        {
            // 如果缓冲区中有剩余位置来存放一个帧，才放入数据
            uart0_recv_buf[blank_index][cur_recv_len++] = uart0_tmp_val;
            // uart0_recv_buf[0][cur_recv_len++] = UART0_DATA; // 测试用

            // tmr0_cnt = 0; // 每接收到一个字节数据，清空超时的计时
            uart0_receive_timeout_cnt = 0; // 每接收到一个字节数据，清空超时的计时

            if ((frame_len != 0) &&
                (cur_recv_len) == frame_len)
            {
                // 如果接收完了一帧数据
                uart0_recv_len[blank_index] = frame_len; // 更新当前数据帧的长度
                // printf("frame_len %d\n", (int32)frame_len);
                flag_is_recving_data = 0; // 标志位清零，准备接收下一帧数据
                cur_recv_len = 0;
                frame_len = 0;

                recved_flagbuf[blank_index] = 1; // 对应的接收完成标志位置一
                recv_frame_cnt++;                // 接收到完整的一帧，计数值加一
                // test_bit = 1; // 测试用
                blank_index++;
                if (blank_index >= ((UART0_RXBUF_LEN) / (FRAME_MAX_LEN)))
                {
                    blank_index = 0;
                }

                // if (recv_frame_cnt < ((UART0_RXBUF_LEN) / (FRAME_MAX_LEN)))
                // {
                //     // 如果数组中有空位（接收的数据帧个数 < 缓冲区总共能接收的数据帧个数）

                //     // 偏移到数组中空的地方，准备填入下一个数据帧
                //     while (1)
                //     {
                //         blank_index++;

                //         if (blank_index >= ((UART0_RXBUF_LEN) / (FRAME_MAX_LEN)))
                //         {
                //             // 如果下标超出了缓冲区能容纳的指令个数
                //             blank_index = 0;
                //         }

                //         if (0 == recved_flagbuf[blank_index])
                //         {
                //             // 如果是空的一处缓冲区，退出，准备给下一次接收数据帧
                //             break;
                //         }
                //     }
                // }

                {
                    // tmr0_disable();
                    // tmr0_cnt = 0;
                    
                    uart0_receive_timeout_disable(); // 不使能接收超时检测功能
                    __IRQnIPnPop(UART0_IRQn);
                    return;
                }
            } // if ((cur_recv_len) == frame_len)
        } // if (0 == recved_flagbuf[blank_index])
        // else // 如果当前缓冲区中没有剩余位置来存放一个帧，
        // {
        // }

    } // if (UART0_STA & UART_RX_DONE(0x1))

    // P20 = 0; // 测试中断持续时间
    // 退出中断设置IP，不可删除
    __IRQnIPnPop(UART0_IRQn);
}

// UART0发送一个字节数据的函数
void uart0_sendbyte(u8 senddata)
{
    while (!(UART0_STA & UART_TX_DONE(0x01)))
        ;
    UART0_DATA = senddata;
    while (!(UART0_STA & UART_TX_DONE(0x01))) // 等待这次发送完成
        ;
}

#if 0  // void uart0_send_buff(u8 *buf, u8 len)
// 通过uart0发送若干数据
void uart0_send_buff(u8 *buf, u8 len)
{
    u8 i = 0;
    for (; i < len; i++)
    {
        uart0_sendbyte(buf[i]);
    }
}
#endif // void uart0_send_buff(u8 *buf, u8 len)

// 对UART0接收缓冲区的数据进行验证（超时验证、长度验证、校验和计算）
extern void __uart_buff_check(void);
void uart0_scan_handle(void)
{
    u8 i = 0;                                  // 循环计数值（注意循环次数要大于等于数组能存放的指令数目）
    volatile u8 checksum = 0;                  // 存放临时的校验和
    volatile bit __flag_is_crc_or_len_err = 0; // 标志位，校验和 / 数据长度 是否错误,0--未出错，1--出错

#if 1 // 接收超时处理

    //     if (tmr0_cnt > 10) // 一帧内，超过10ms没有收到数据
    //     {
    //         // 如果超时
    //         // uart0_sendstr("Time out!\n");

    //         tmr0_disable(); // 关闭定时器
    //         tmr0_cnt = 0;   // 清空定时器计数值

    //         // 当前的数据帧作废
    //         cur_recv_len = 0;         // 当前接收到的帧的长度清零
    //         frame_len = 0;            // 要接收的帧的长度，清零
    //         flag_is_recving_data = 0; // 重新开始接收数据

    //         // if (0 == recved_flagbuf[blank_index])
    //         // {
    //         //     memset(uart0_recv_buf[blank_index], 0, FRAME_MAX_LEN); // 清空超时的指令对应的接收缓冲区
    //         // }

    //         // printf("recv time out! \n");

    // #if 0 // 打印串口缓冲区的数据

    //         {
    //             u8 i = 0;
    //             u8 j = 0;
    //             for (i = 0; i < (UART0_RXBUF_LEN) / (FRAME_MAX_LEN); i++)
    //             {
    //                 printf("buff_index %bu ", i);
    //                 for (j = 0; j < FRAME_MAX_LEN; j++)
    //                 {
    //                     printf("%bx ", uart0_recv_buf[i][j]);
    //                 }
    //                 printf("\n");
    //             }
    //         }

    // #endif // 打印串口缓冲区的数据

    //         return;
    //     }

    if (uart0_receive_timeout_cnt > 10) // 一帧内，超过10ms没有收到数据
    {
        // 如果超时
        //    printf("timeout\n");

        uart0_receive_timeout_disable(); // 关闭串口接收超时检测功能

        // 当前的数据帧作废
        cur_recv_len = 0;         // 当前接收到的帧的长度清零
        frame_len = 0;            // 要接收的帧的长度，清零
        flag_is_recving_data = 0; // 重新开始接收数据

        return;
    }
#endif // 接收超时处理

    // 对收到的数据帧进行验证
    for (i = 0; i < ((UART0_RXBUF_LEN) / (FRAME_MAX_LEN)); i++)
    {
        if (recved_flagbuf[i])
        {
            // 如果对应的位置有完整的数据帧
            // __uart_buff_check(); // 测试用

            if (uart0_recv_len[i] != uart0_recv_buf[i][1])
            {
                // 如果数据帧的长度不正确

                // printf("format len invalid!\n");
                // printf("%d  %d \n", (u32)uart0_recv_len[i], (u32)uart0_recv_buf[i][1]);
                // __uart_buff_check(); // 打印串口接收缓冲区中的数据

                recved_flagbuf[i] = 0;
                recv_frame_cnt--;
                memset(uart0_recv_buf[i], 0, FRAME_MAX_LEN);

                return;
            }

            {                          // 计算校验和
                u8 __loop_crc_cnt = 0; // 用于计算校验和的循环计数值
                checksum = 0;
                for (__loop_crc_cnt = 0; __loop_crc_cnt < (uart0_recv_len[i] - 1); __loop_crc_cnt++)
                {
                    checksum += uart0_recv_buf[i][__loop_crc_cnt];
                }

                // checksum &= 0x0F; // 取低4位作为校验
                checksum &= 0xFF; // 取8位作为校验
                if (checksum != (uart0_recv_buf[i][uart0_recv_len[i] - 1]))
                {
                    // 如果计算的校验和与收到的校验和不一致
                    __flag_is_crc_or_len_err = 1;
                }
            } // 计算校验和

            if (__flag_is_crc_or_len_err)
            {

#if 0 // 打印串口缓冲区的数据
                printf("recv crc or len err! \n");
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

                // flag_is_recving_data = 0;
                __flag_is_crc_or_len_err = 0; //
                recved_flagbuf[i] = 0;
                recv_frame_cnt--;
                memset(uart0_recv_buf[i], 0, FRAME_MAX_LEN); // 清空校验和错误的指令对应的缓冲区
            }

            // 如果运行到这里，数据都正常
            flagbuf_valid_instruction[i] = 1; // 对应的标志位置一，表示收到了合法的指令
        } // if (recved_flagbuf[i])
    } // for (i = 0; i < ((UART0_RXBUF_LEN) / (FRAME_MAX_LEN)); i++)
}

#if 0
// 清除串口接收缓冲区中，单个已经处理好的指令
void uart_clear_single_instruction(u8 index)
{
    flagbuf_valid_instruction[index] = 0;            // 清空缓冲区对应的元素，表示该下标的指令已经处理
    uart0_recv_len[index] = 0;                       // 清除缓冲区中指令对应的长度
    recved_flagbuf[index] = 0;                       // 清除接收到指令的标志位
    memset(uart0_recv_buf[index], 0, FRAME_MAX_LEN); // 清空缓冲区对应的指令
}
#endif

#ifdef USE_MY_DEBUG
#if USE_MY_DEBUG
// 测试用的程序：
// 显示串口缓冲区中的数据：
// void __uart_buff_check(void)
// {
//     u8 i = 0;

//     u8 k = 0;
//     // if (test_bit)
//     // {
//     //     test_bit = 0;
//     for (i = 0; i < (UART0_RXBUF_LEN) / (FRAME_MAX_LEN); i++)
//     {
//         uart0_sendbyte(i + '0');
//         uart0_sendbyte('\n');
//         for (k = 0; k < FRAME_MAX_LEN; k++)
//         {
//             printf("%2x ", (u16)uart0_recv_buf[i][k]);
//         }

//         printf("\n==========================\n");
//     }
//     // }
// }
#endif
#endif // void __uart_buff_check(void)

#if 0
            switch (uart0_recv_len[i])
            {
            case 4:

                // 如果是四个字节的数据
                checksum = (uart0_recv_buf[i][0] + uart0_recv_buf[i][1] + uart0_recv_buf[i][2]) & 0x0F;

                if (checksum != uart0_recv_buf[i][3])
                {
                    // 如果计算得出的校验和与数据帧中的校验和不一致
                    // 说明传输的数据有误
                    // uart0_sendstr("checknum err_4Bytes\n");
                    // recved_flagbuf[i] = 0;
                    // recv_frame_cnt--;
                    // memset(uart0_recv_buf[i], 0, FRAME_MAX_LEN);
                    __flag_is_crc_or_len_err = 1;
                }

                break;

            case 5:

                // 如果是五个字节的数据
                checksum = (uart0_recv_buf[i][0] + uart0_recv_buf[i][1] + uart0_recv_buf[i][2] + uart0_recv_buf[i][3]) & 0x0F;
                if (checksum != uart0_recv_buf[i][4])
                {
                    // 如果计算得出的校验和与数据帧中的校验和不一致
                    // 说明传输的数据有误
                    // uart0_sendstr("checknum err_5Bytes\n");
                    // recved_flagbuf[i] = 0;
                    // recv_frame_cnt--;
                    // memset(uart0_recv_buf[i], 0, FRAME_MAX_LEN);
                    __flag_is_crc_or_len_err = 1;
                }

                break;
            case 6:

                // 如果是六个字节的数据
                checksum = (uart0_recv_buf[i][0] + uart0_recv_buf[i][1] + uart0_recv_buf[i][2] + uart0_recv_buf[i][3] + uart0_recv_buf[i][4]) & 0x0F;

                if (checksum != uart0_recv_buf[i][5])
                {
                    // 如果计算得出的校验和与数据帧中的校验和不一致
                    // 说明传输的数据有误
                    // uart0_sendstr("checknum err_6Bytes\n");
                    // recved_flagbuf[i] = 0;
                    // recv_frame_cnt--;
                    // memset(uart0_recv_buf[i], 0, FRAME_MAX_LEN);
                    __flag_is_crc_or_len_err = 1;
                }

                break;
            case 7: // 如果是7个字节的数据
                checksum = (uart0_recv_buf[i][0] + uart0_recv_buf[i][1] + uart0_recv_buf[i][2] + uart0_recv_buf[i][3] + uart0_recv_buf[i][4] + uart0_recv_buf[i][5]) & 0x0F;
                if (checksum != uart0_recv_buf[i][6])
                {
                    // 如果计算得出的校验和与数据帧中的校验和不一致
                    // 说明传输的数据有误
                    // recved_flagbuf[i] = 0;
                    // recv_frame_cnt--;

                    __flag_is_crc_or_len_err = 1;
                }
                break;
            case 8: // 如果是8个字节的数据
                checksum = (uart0_recv_buf[i][0] + uart0_recv_buf[i][1] + uart0_recv_buf[i][2] + uart0_recv_buf[i][3] + uart0_recv_buf[i][4] + uart0_recv_buf[i][5] + uart0_recv_buf[i][6]) & 0x0F;
                if (checksum != uart0_recv_buf[i][7])
                {
                    // 如果计算得出的校验和与数据帧中的校验和不一致
                    // 说明传输的数据有误
                    // recved_flagbuf[i] = 0;
                    // recv_frame_cnt--;
                    __flag_is_crc_or_len_err = 1;
                }
                break;

            default:
                // 如果不是四、五、六、七、八个字节的数据，说明接收有误，直接抛弃这一帧数据
#if USE_MY_DEBUG
                // printf("recv cnt err\n");
                __flag_is_crc_or_len_err = 1;
#endif
                // recved_flagbuf[i] = 0;
                // recv_frame_cnt--;
                // memset(uart0_recv_buf[i], 0, FRAME_MAX_LEN);

                break;
            } // switch (uart0_recv_len[i])
#endif