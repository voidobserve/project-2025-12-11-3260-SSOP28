#ifndef __IIC_SOFT_H
#define __IIC_SOFT_H

#include "my_config.h"

#if USE_EEPROM_SAVE_DATA

#define E2PROM_DEVICE_ADDR (0xA0) // 器件地址
#define IIC_DELAY()                                                                                                   \
    do                                                                                                                \
    {                                                                                                                 \
        /*  delay_5us(1);   */                                                                                        \
        delay(4);                                                                                                     \
        /*  delay_ms(1);  测试eeprom读写器件，如果突然断电，会不会影响数据                       \
        测试发现，延长时间后，会影响主循环，每轮distance-=1000时，distance还有剩余数据， \
        会重复进入distance-=1000，再触发每s保存里程，写入eeprom的功能） */                       \
    } while (0)

#define IIC_SCL P31 /* iic 时钟引脚 SCL */
#define IIC_SDA P26 /* iic 数据引脚 SDA */
                    /* SDA配置为输入模式 */
#define SDA_IN()                                                                   \
    do                                                                             \
    {                                                                              \
        P2_MD1 &= ~GPIO_P26_MODE_SEL(0x03); /* 清除寄存器配置，此时对应输入模式 */ \
        P2_PU |= GPIO_P26_PULL_UP(0x01);    /* 打开上拉 */                         \
    } while (0)

#define SDA_OUT()                                        \
    do                                                   \
    {                                                    \
        P2_PU &= ~GPIO_P26_PULL_UP(0x01); /* 关闭上拉 */ \
        P2_MD1 &= ~GPIO_P26_MODE_SEL(0x3);               \
        P2_MD1 |= GPIO_P26_MODE_SEL(0x1); /* 输出模式 */ \
        FOUT_S26 = GPIO_FOUT_AF_FUNC;                    \
    } while (0)

#define IIC_WRITE_CMD ((u8)0x00) // iic写命令
#define IIC_READ_CMD ((u8)0x01)  // iic读命令

#define EEPROM_MAX_ERASE_COUNTS_PER_PAGE ((u32)300000) // 程序限制的，每组页面最大的擦写次数
// DEBUG:
// #define EEPROM_MAX_ERASE_COUNTS_PER_PAGE ((u32)10)                        // 程序限制的，每组页面最大的擦写次数 -- 测试用（最大写到 n 次，包括第 n 次）
#define EEPROM_DATA_VALID_VAL ((u8)0xC5)                                  // 数据有效时，对应位置的变量存放到的数值
#define EEPROM_PAGE_NUMS ((u8)128)                                        // eeprom 页数量
#define EEPROM_PAGE_SIZE ((u8)(32))                                       // eeprom 页地址大小，单位：字节
#define EEPROM_PAGE_X_ADDR(x) ((u16)(0x00 + (u16)(x) * EEPROM_PAGE_SIZE)) // 将页面id转换成对应的地址

typedef struct
{
    // 总里程表（单位：m，使用英制单位时，只需要再发送时进行转换）
    // （大计里程，范围：0~ 99999.9 KM）
    u32 total_mileage;
    // 短距离里程表(单位：m，使用英制单位时，只需要再发送时进行转换)
    // （小计里程，范围：0~9999.9 KM）
    u32 subtotal_mileage;
    u32 subtotal_mileage_2; // 小计里程2

    u32 erase_cnt; // 擦写次数

    u8 is_data_valid;
} eeprom_saveinfo_t;

typedef struct
{
    u8 cur_write_page_id; // 当前写入的扇区ID （只在扇区目录中写入/读取时使用）
    u8 is_data_valid;     // DATA_VALID_ID--数据有效，其他--数据无效（只在扇区目录中写入/读取时使用）
} eeprom_menu_t;

extern volatile eeprom_menu_t eeprom_menu_prev; // 存放从eeprom读出的目录数据
extern volatile eeprom_menu_t eeprom_menu_next;
extern volatile eeprom_saveinfo_t eeprom_saveinfo_prev; // 存放从eeprom读出的数据，prev，同一组数据的前一页数据
extern volatile eeprom_saveinfo_t eeprom_saveinfo_next; // 存放从eeprom读出的数据，next，同一组数据的后一页数据
extern volatile eeprom_saveinfo_t eeprom_saveinfo;

#if USE_MY_DEBUG

extern volatile eeprom_menu_t eeprom_menu;
extern volatile bit flag_is_printf_eeprom_data;

void eeprom_printf_all(void);

// void iic_eeprom_clear(void);

#endif // #if USE_MY_DEBUG

void eeprom_24cxx_clear(void);

// void eeprom_menu_write(void);
// void eeprom_menu_read(void);
void eeprom_menu_init(void); // 上电使用eeprom前，先初始化它的目录
void eeprom_menu_write(void);
void eeprom_menu_read(void);

void eeprom_data_save(void);

u8 iic_eeprom_write(u16 device_addr, u8 *pBuffer, u16 num_write_data);
u8 iic_eeprom_read(u16 device_addr, u8 *pBuffer, u16 num_read_data);

void eeprom_24cxx_write(u8 page_id, u8 *p_buf, u16 len);
void eeprom_24cxx_read(u8 page_id, u8 *p_buf, u16 len);

void iic_config(void);
#endif

#endif
