#ifndef __UART0_H
#define __UART0_H

#include "include.h" // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

// 串口0波特率
#define UART0_BAUDRATE (115200)
// #define UART0_BAUDRATE (1500000)
// 串口1波特率
// #define UART1_BAUDRATE 115200 // 这款芯片（TX3260）不能使用UART1的DMA来接收不定长的数据

// 串口0接收缓冲区的大小（单位：字节）
#define UART0_RXBUF_LEN (45) // 最大不能超过255，超过之后，串口接收会出错
// 定义一帧最大的长度
#define FRAME_MAX_LEN 15
// 注意 【UART0_RXBUF_LEN / FRAME_MAX_LEN】 不能小于1



// 用来存放接收的数据帧的缓冲区
extern volatile u8 uart0_recv_buf[(UART0_RXBUF_LEN) / (FRAME_MAX_LEN)][FRAME_MAX_LEN];
extern volatile u8 uart0_recv_len[(UART0_RXBUF_LEN) / (FRAME_MAX_LEN)];
extern volatile u8 recved_flagbuf[(UART0_RXBUF_LEN) / (FRAME_MAX_LEN)];
extern volatile u8 recv_frame_cnt; // 接收到的数据帧的个数
extern volatile u8 flagbuf_valid_instruction[(UART0_RXBUF_LEN) / (FRAME_MAX_LEN)]; // 存放有合法指令的标志位数组

extern volatile bit flag_is_uart0_receive_timeout; // 标志位，串口是否接收超时，0--未超时，1--已超时
extern volatile bit flag_is_uart0_receive_timeout_enable; // 标志位，是否使能串口接收超时，0--否，1--串口接收了数据，却未收到完整一帧数据，使能串口接收超时
extern volatile u8 uart0_receive_timeout_cnt;            // 存放串口接收超时计数值

// 串口0波特率
#ifndef UART0_BAUDRATE
#define UART0_BAUDRATE 115200
#endif // UART0_BAUDRATE

// 串口0接收缓冲区的大小
#ifndef UART0_RXBUF_LEN
#define UART0_RXBUF_LEN 256
#endif

void uart0_config(void);
void uart0_sendbyte(u8 senddata);
void uart0_send_buff(u8* buf, u8 len); // 通过uart0发送若干数据
// void uart0_sendbytes(u8 *buf, u32 buf_len);
// void uart0_sendstr(u8 *str); // UART0发送字符串的函数

void uart0_scan_handle(void); // 检验串口接收缓冲区的数据是否符合指令的函数

#endif
