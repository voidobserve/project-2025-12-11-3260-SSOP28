#include "iic_soft.h"

#if USE_EEPROM_SAVE_DATA

volatile eeprom_menu_t eeprom_menu_prev;
volatile eeprom_menu_t eeprom_menu_next;
volatile eeprom_saveinfo_t eeprom_saveinfo;

// 更新eeprom中目录相关的数据，需要提前写好 eeprom_menu_prev 和 eeprom_menu_next
void eeprom_menu_write(void)
{
    eeprom_24cxx_write(0, (u8 *)&eeprom_menu_prev, sizeof(eeprom_menu_t));
    WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending
    eeprom_24cxx_write(1, (u8 *)&eeprom_menu_next, sizeof(eeprom_menu_t));
}

void eeprom_menu_read(void)
{
    eeprom_24cxx_read(0, (u8 *)&eeprom_menu_prev, sizeof(eeprom_menu_t));
    WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending
    eeprom_24cxx_read(1, (u8 *)&eeprom_menu_next, sizeof(eeprom_menu_t));
}

void eeprom_menu_init(void)
{
    eeprom_menu_read();

    if (eeprom_menu_prev.is_data_valid != EEPROM_DATA_VALID_VAL && eeprom_menu_next.is_data_valid != EEPROM_DATA_VALID_VAL)
    {
        // 如果两个页 的目录读到的数据都是无效的，说明是第一次使用eeprom，进行初始化
        eeprom_menu_prev.cur_write_page_id = 2; // 0为目录prev，1为目录next，真正的数据从2开始
        eeprom_menu_prev.is_data_valid = EEPROM_DATA_VALID_VAL;

        eeprom_menu_next = eeprom_menu_prev;
        eeprom_menu_write();

        // printf("all menus are invalid\n");
    }
    else if ((eeprom_menu_prev.is_data_valid == EEPROM_DATA_VALID_VAL && eeprom_menu_next.is_data_valid != EEPROM_DATA_VALID_VAL) ||
             (eeprom_menu_prev.is_data_valid != EEPROM_DATA_VALID_VAL && eeprom_menu_next.is_data_valid == EEPROM_DATA_VALID_VAL))
    {
        // 如果两个页的目录读到的数据有一个页是有效的，有一个页是无效的
        // 用有效的那个页的数据进行初始化

        if (eeprom_menu_prev.is_data_valid == EEPROM_DATA_VALID_VAL)
        {
            eeprom_menu_next = eeprom_menu_prev;
            eeprom_24cxx_write(1, (u8 *)&eeprom_menu_next, sizeof(eeprom_menu_t));
        }
        else if (eeprom_menu_next.is_data_valid == EEPROM_DATA_VALID_VAL)
        {
            eeprom_menu_prev = eeprom_menu_next;
            eeprom_24cxx_write(0, (u8 *)&eeprom_menu_prev, sizeof(eeprom_menu_t));
        }

        // printf("one of menus is valid\n");
    }
    else
    {
        // 如果两个页的目录中的数据都有效
        // printf("menus are valid\n");

        /*
            判断目录中 cur_write_page_id 当前要写入里程数据的页面编号是否一致，
            如果不一致，以最大的id作为最新的页面编号，再写回eeprom
        */
        if (eeprom_menu_prev.cur_write_page_id > eeprom_menu_next.cur_write_page_id)
        {
            eeprom_menu_next.cur_write_page_id = eeprom_menu_prev.cur_write_page_id;
            eeprom_menu_write();
        }
        else if (eeprom_menu_prev.cur_write_page_id < eeprom_menu_next.cur_write_page_id)
        {
            eeprom_menu_prev.cur_write_page_id = eeprom_menu_next.cur_write_page_id;
            eeprom_menu_write();
        }
        else // (eeprom_menu_prev.cur_write_page_id == eeprom_menu_next.cur_write_page_id)
        {
            // 数据一致
        }
    }
}

#if USE_MY_DEBUG
volatile bit flag_is_printf_eeprom_data;

volatile eeprom_menu_t eeprom_menu;

void eeprom_printf_all(void)
{
    u16 i;

    // eeprom_menu_t eeprom_menu = {0};
    eeprom_menu.cur_write_page_id = 0;
    eeprom_menu.is_data_valid = 0;
    eeprom_24cxx_read(0, (u8 *)&eeprom_menu, sizeof(eeprom_menu_t));
    printf("eeprom_menu_prev.cur_write_page_id = %bu\n", eeprom_menu.cur_write_page_id);
    printf("eeprom_menu_prev.is_data_valid = 0x %x\n", (u16)eeprom_menu.is_data_valid);

    eeprom_menu.cur_write_page_id = 0;
    eeprom_menu.is_data_valid = 0;
    eeprom_24cxx_read(1, (u8 *)&eeprom_menu, sizeof(eeprom_menu_t));
    printf("eeprom_menu_next.cur_write_page_id = %bu\n", eeprom_menu.cur_write_page_id);
    printf("eeprom_menu_next.is_data_valid = 0x %x\n", (u16)eeprom_menu.is_data_valid);

    printf("====================================================\n");

    // for (i = 2; i < 128; i++)
    // for (i = 2; i < EEPROM_PAGE_NUMS; i++)
    for (i = 2; i < 10; i++)
    {
        eeprom_24cxx_read(i, (u8 *)&eeprom_saveinfo, sizeof(eeprom_saveinfo_t));
        printf("cur_page_id %u\n", i);
        printf("eeprom_saveinfo.erase_cnt = %lu\n", eeprom_saveinfo.erase_cnt);
        printf("eeprom_saveinfo.toal_mileage = %lu\n", eeprom_saveinfo.total_mileage);
        printf("eeprom_saveinfo.subtotal_mileage = %lu\n", eeprom_saveinfo.subtotal_mileage);
        printf("eeprom_saveinfo.subtotal_mileage_2 = %lu\n", eeprom_saveinfo.subtotal_mileage_2);
        printf("====================================================\n");
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending
    }
}

#endif

void eeprom_24cxx_clear(void)
{
    u16 i;
    // const u8 clear_data = 0x00; // 要填充的数据 
    const u8 clear_data = 0xFF; // 要填充的数据 
    for (i = 0; i < ((u16)128 * 32); i++)
    {
        while (iic_eeprom_write(i, (u8 *)&clear_data, 1))
        {
            WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending
        }
    }
}

// 向eeprom写入需要保存的数据
volatile eeprom_saveinfo_t eeprom_saveinfo_prev; // 存放从eeprom读出的数据，prev，同一组数据的前一页数据
volatile eeprom_saveinfo_t eeprom_saveinfo_next; // 存放从eeprom读出的数据，next，同一组数据的后一页数据
/*
    函数内部会把 fun_info.save_info 的数据写入eeprom
*/
void eeprom_data_save(void)
{
    u8 cur_save_info_id_prev; // 当前保存数据的页id，prev，同一组数据的前一页（为了节省程序空间，不进行初始化，由函数内部赋值）
    u8 cur_save_info_id_next; // 当前保存数据的页id，next，同一组数据的后一页

    u8 cur_write_page_id = 0; // 存放待写入数据的地址对应id
    u32 cur_erase_cnt;        // 存放读取到的擦写次数

    /*
        写入前，读取目录，根据目录找到要写入的地址

        如果目录的数据不一致，以最后写入的地址作为新写入地址（目录中的两个地址，最后写入的地址应该大于另一个地址）
    */

    eeprom_menu_read();

    // if (eeprom_menu_prev.is_data_valid == EEPROM_DATA_VALID_VAL && eeprom_menu_next.is_data_valid == EEPROM_DATA_VALID_VAL)
    if (eeprom_menu_prev.cur_write_page_id == eeprom_menu_next.cur_write_page_id)
    {
        cur_write_page_id = eeprom_menu_prev.cur_write_page_id;
    }
    else
    {
        // 如果目录中的数据不一致，判断哪个地址更大，哪个就作为待写入的地址的id
        if (eeprom_menu_prev.cur_write_page_id > eeprom_menu_next.cur_write_page_id)
        {
            cur_write_page_id = eeprom_menu_prev.cur_write_page_id;
        }
        else
        {
            cur_write_page_id = eeprom_menu_next.cur_write_page_id;
        }
    }

    /*
        page[0]和page[1]已经作为目录使用（id==0和id==1），
        剩下的页面，每两个页面作为一组数据，
        那么偶数页面作为同一组页面的前一个页面，奇数页面作为同一组页面的后一个页面

        例如 id == 2 作为第 1 组页面的前一个页面 prev，
            id == 3 作为第 1 组页面的后一个页面 next
    */
    if (cur_write_page_id % 2 == 0)
    {
        /*
            如果得到的是偶数页面，
            得到的偶数，作为同一组页面的第一个页面 prev
            得到的偶数 + 1，作为同一组页面的第二个页面 next
        */
        cur_save_info_id_prev = cur_write_page_id;
        cur_save_info_id_next = cur_write_page_id + 1;
    }
    else
    {
        /*
            如果得到的是奇数页面，

            得到的奇数 - 1，作为同一组页面的第一个页面 prev
            得到的奇数，作为同一组页面的第二个页面 next
        */
        cur_save_info_id_prev = cur_write_page_id - 1;
        cur_save_info_id_next = cur_write_page_id;
    }

    /*
        得到同一组页面的前、后两个页面的id后，读取这两个页面对应的数据

        将两个页面的数据进行比较， 用总里程较大的数据 覆盖 总里程较小的数据

        如果超过了限制的擦写次数，要换下一组页面，并且更新到目录中
        如果写完之后，记录的页面id与目录中的id不一致，要更新目录中记录的页面id
    */
    eeprom_24cxx_read((cur_save_info_id_prev), (u8 *)&eeprom_saveinfo_prev, sizeof(eeprom_saveinfo_t));
    eeprom_24cxx_read((cur_save_info_id_next), (u8 *)&eeprom_saveinfo_next, sizeof(eeprom_saveinfo_t));

    // 数据无效，可能是第一次读取，或者由于写入时掉电导致数据丢失
    if (eeprom_saveinfo_prev.is_data_valid != EEPROM_DATA_VALID_VAL)
    {
        eeprom_saveinfo_prev.erase_cnt = 0;
        eeprom_saveinfo_prev.total_mileage = 0;
        eeprom_saveinfo_prev.subtotal_mileage = 0;
        eeprom_saveinfo_prev.subtotal_mileage_2 = 0;
        eeprom_saveinfo_prev.is_data_valid = EEPROM_DATA_VALID_VAL;
    }

    // 数据无效，可能是第一次读取，或者由于写入时掉电导致数据丢失
    if (eeprom_saveinfo_next.is_data_valid != EEPROM_DATA_VALID_VAL)
    {
        eeprom_saveinfo_next = eeprom_saveinfo_prev;
    }

    if (eeprom_saveinfo_prev.total_mileage < eeprom_saveinfo_next.total_mileage)
    {
        /*
            如果前一个页面的总里程要小于后一个页面的总里程，
            后一个页面的总里程 加上 新增的总里程增量，得到的总里程保存到前一个页面中，
            后一个页面作为备份，也保存下来
        */
        eeprom_saveinfo_next.total_mileage = fun_info.save_info.total_mileage;
        eeprom_saveinfo_next.subtotal_mileage = fun_info.save_info.subtotal_mileage;
        eeprom_saveinfo_next.subtotal_mileage_2 = fun_info.save_info.subtotal_mileage_2;

        cur_erase_cnt = eeprom_saveinfo_next.erase_cnt; // 存放擦写次数
        eeprom_saveinfo_prev = eeprom_saveinfo_next;
    }
    else
    {
        /*
            如果前一个页面的总里程要 大于或等于 后一个页面的总里程，
            前一个页面的总里程 加上 新增的总里程增量，得到的总里程保存到后一个页面中，
            前一个页面作为备份，也保存下来
        */

        eeprom_saveinfo_prev.total_mileage = fun_info.save_info.total_mileage;
        eeprom_saveinfo_prev.subtotal_mileage = fun_info.save_info.subtotal_mileage;
        eeprom_saveinfo_prev.subtotal_mileage_2 = fun_info.save_info.subtotal_mileage_2;

        cur_erase_cnt = eeprom_saveinfo_prev.erase_cnt; // 存放擦写次数
        eeprom_saveinfo_next = eeprom_saveinfo_prev;
    }

    cur_erase_cnt++; // 擦写次数累加
    eeprom_saveinfo_prev.erase_cnt = cur_erase_cnt;
    eeprom_saveinfo_next.erase_cnt = cur_erase_cnt;
    if (cur_erase_cnt > EEPROM_MAX_ERASE_COUNTS_PER_PAGE)
    {
        // 如果超过了一组页面的最大擦写次数，更换下一组页面

        cur_save_info_id_prev = cur_save_info_id_next + 1; // 当前为奇数id，加一，为偶数id，是下一组页面的第一个页面
        cur_save_info_id_next = cur_save_info_id_prev + 1;

        if (cur_save_info_id_prev >= (EEPROM_PAGE_NUMS - 2))
        {
            // 防止目录和擦写的位置越界，超出了eeprom能存放的位置
            cur_save_info_id_prev = EEPROM_PAGE_NUMS - 2; // eeprom的倒数第2页
            cur_save_info_id_next = EEPROM_PAGE_NUMS - 1; // eeprom的倒数第1页，最后一页
        }

        // 擦写次数清零
        eeprom_saveinfo_prev.erase_cnt = 0;
        eeprom_saveinfo_next.erase_cnt = 0;
    }

    eeprom_24cxx_write(cur_save_info_id_prev, (u8 *)&eeprom_saveinfo_prev, sizeof(eeprom_saveinfo_t));
    eeprom_24cxx_write(cur_save_info_id_next, (u8 *)&eeprom_saveinfo_next, sizeof(eeprom_saveinfo_t));

    /*
        如果写入的页面id与目录记录的id不一样，则更新目录记录的id
    */
    if (cur_save_info_id_prev != eeprom_menu_prev.cur_write_page_id)
    {
        eeprom_menu_prev.cur_write_page_id = cur_save_info_id_prev;
        eeprom_menu_prev.is_data_valid = EEPROM_DATA_VALID_VAL;

        // eeprom_menu_next.cur_write_page_id = cur_save_info_id_prev;
        // eeprom_menu_next.is_data_valid = EEPROM_DATA_VALID_VAL;

        eeprom_menu_next = eeprom_menu_prev;

        eeprom_menu_write();
    }
}

// static void delay_5us(u32 us)
// {
//     u32 b = us * 1;
//     // u32 b = us * 2;
//     while (b--)
//     {
//     }
// }

/**
 * @brief  iic start function
 * @param  None
 * @retval None
 */
void iic_start(void)
{
    // 起始时，系统时钟线（SCLK）和系统数据线（SDATA）都处于高电平
    SDA_OUT(); // SDA线输出
    IIC_SDA = 1;
    IIC_SCL = 1;
    // 延时函数是为了等待配置完成，当SCLK为高电平时，SDATA跳变为低电平
    // 紧接着SCLK也跳变为低电平
    // 产生一个起始信号
    IIC_DELAY();
    IIC_SDA = 0; // START:when CLK is high,DATA change form high to low
    IIC_DELAY();
    IIC_SCL = 0;
}

/**
 * @brief  iic stop function
 * @param  None
 * @retval None
 */
void iic_stop(void)
{
    SDA_OUT(); // SDA线输出

    // 配置时钟线和数据线均为低电平
    IIC_SCL = 0;
    IIC_SDA = 0; // STOP:when CLK is high DATA change form low to high
    IIC_DELAY();

    // 配置时钟线为高电平
    IIC_SCL = 1;
    IIC_DELAY();

    // 在时钟线为高电平时，输出的数据线从低电平跳变到高电平，则产生了停止信号
    IIC_SDA = 1;
    IIC_DELAY();
}

/**
 * @brief  wait ack function
 * @param  None
 * @retval 1:no ack 0:ack
 */
u8 iic_wait_ack(void)
{
    u8 timeout = 0;

    SDA_IN(); // SDA设置为输入

    // 配置为高电平
    IIC_SDA = 1;
    IIC_DELAY();

    // 配置为高电平
    IIC_SCL = 1;
    IIC_DELAY();

    while (IIC_SDA)
    {
        // 等待SDA拉低，表示应答带来，不然一直while循环，直到超时
        timeout++;
        if (timeout > 250)
        {
            iic_stop(); // 等待超时，应答失败，发出停止信号
            return 1;   // 返回1，表示失败
        }
        IIC_DELAY();
    }
    IIC_SCL = 0; // SCL拉低

    return 0; // 返回0，表示成功
}

/**
 * @brief  iic ack function
 * @param  None
 * @retval None
 */
void iic_ack(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 0;
    IIC_DELAY();
    IIC_SCL = 1;
    IIC_DELAY();
    IIC_SCL = 0;
}

/**
 * @brief  iic no ack function
 * @param  None
 * @retval None
 */
void iic_nack(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 1;
    IIC_DELAY();
    IIC_SCL = 1;
    IIC_DELAY();
    IIC_SCL = 0;
}

/**
 * @brief  iic send 1 byte data function
 * @param  None
 * @retval 1:Ack 0:No Ack
 */
void iic_send_byte(u8 txd)
{
    u8 i;

    // 配置P03为输出模式
    SDA_OUT(); // SDA线输出模式

    // 只有在时钟线为0的情况下，数据线才可以进行高低电平的跳变
    IIC_SCL = 0; // 拉低时钟开始数据传输

    for (i = 0; i < 8; i++)
    {                                // for循环，一位一位的发送，从最高位 位7开始
        IIC_SDA = (txd & 0x80) >> 7; // 除了位7外，其余全屏蔽为0，然后右移到位0，给SDA数据线
        txd <<= 1;                   // 左移一位，准备下一次发送
        IIC_DELAY();

        // 发送完成之后，时钟线拉高
        IIC_SCL = 1;
        IIC_DELAY();

        // 紧接着拉低
        IIC_SCL = 0;
        IIC_DELAY();

        // 发送完一个数据都将时钟线进行一次的拉高和拉低，完成一个位的传输
    }
}

/**
 * @brief  iic read 1 byte function
 * @param  none
 * @retval ack=1,send ack=0;ack=0,send no ack
 */
u8 iic_read_byte(unsigned char ack)
{
    // unsigned char i, receive = 0;
    u8 i;
    u8 receive = 0;

    SDA_IN(); // SDA设置为输入

    for (i = 0; i < 8; i++)
    { // for循环，一位一位的读取，从最高位 位7开始
        IIC_SCL = 0;
        IIC_DELAY();
        IIC_SCL = 1;
        receive <<= 1; // 左移一位，准备下次的读取

        if (IIC_SDA)
        {
            receive++;
        }
        IIC_DELAY();
    }

    if (!ack)
    {               // 不需要发送
        iic_nack(); // 发送nACK
    }
    else
    {              // 需要发送
        iic_ack(); // 发送ACK
    }

    return receive;
}

/**
 * @brief  在AT24CXX里面的指定地址开始读出指定个数的数据
 * @param  addr : 开始读出的地址 对24c512为0~65535
 * @param  pBuffer  : 数据数组首地址
 * @param  num_read_data: 要读出数据的个数
 * @retval u8  0--成功，1--失败
 */
u8 iic_eeprom_read(u16 addr, u8 *pBuffer, u16 num_read_data)
{
    // 起始信号
    iic_start();

    // 发送设备地址
    // iic_send_byte(E2PROM_DEVICE_ADDR + ((device_addr / 256) << 1));
    iic_send_byte(E2PROM_DEVICE_ADDR | IIC_WRITE_CMD);

    // 等待应答
    if (iic_wait_ack())
    {
        return 1;
    }

    // iic_send_byte(addr / 256 >> 8); // 发送高地址
    iic_send_byte(addr >> 8); // 发送高地址

    // 等待应答
    if (iic_wait_ack())
    {
        return 1;
    }

    // iic_send_byte(addr % 256); // 发送低地址
    iic_send_byte(addr & 0xFF); // 发送低地址

    // 等待应答
    if (iic_wait_ack())
    {
        return 1;
    }

    IIC_DELAY();

    // 起始信号
    iic_start();
    iic_send_byte(E2PROM_DEVICE_ADDR | IIC_READ_CMD); // 进入接收模式

    // 等待应答
    if (iic_wait_ack())
    {
        return 1;
    }

    num_read_data--;
    while (num_read_data)
    {
        *pBuffer++ = iic_read_byte(1);
        num_read_data--;
    }
    *pBuffer = iic_read_byte(0);
    // 停止信号
    iic_stop(); // 产生一个停止条件

    return 0;
}

/**
 * @brief  在AT24CXX里面的指定地址开始写入指定个数的数据
 * @param  addr : 开始写入的地址 对24c512为0~65535
 * @param  pBuffer   : 数据数组首地址
 * @param  num_write_data: 要写入数据的个数
 * @retval 0--操作成功，1--操作失败
 */
u8 iic_eeprom_write(u16 addr, u8 *pBuffer, u16 num_write_data)
{
    // 总线发送一个开始信号
    iic_start();

    // 发送设备地址
    // iic_send_byte(E2PROM_DEVICE_ADDR + ((device_addr / 256) << 1));
    iic_send_byte(E2PROM_DEVICE_ADDR | IIC_WRITE_CMD);

    // 应答信号，等待从机应答
    if (iic_wait_ack())
    {
        return 1; // 表示操作失败，没有收到应答
    }

    // iic_send_byte(addr / 256 >> 8); // 发送高地址
    iic_send_byte(addr >> 8); // 发送高地址

    // 应答信号，等待从机应答
    if (iic_wait_ack())
    {
        return 1; // 表示操作失败，没有收到应答
    }

    // iic_send_byte(addr % 256); // 发送低地址
    iic_send_byte(addr & 0xFF); // 发送低地址

    // 应答信号，等待从机应答
    if (iic_wait_ack())
    {
        return 1; // 表示操作失败，没有收到应答
    }

    while (num_write_data--)
    {
        iic_send_byte(*pBuffer); // 发送字节
        if (iic_wait_ack())
        {
            return 1; // 表示操作失败，没有收到应答
        }

        addr++;
        pBuffer++;
    }

    IIC_DELAY();
    // 总线发送一个停止信号
    iic_stop(); // 产生一个停止条件

    return 0; // 表示操作成功
}

void eeprom_24cxx_write(u8 page_id, u8 *p_buf, u16 len)
{
    u16 addr = EEPROM_PAGE_X_ADDR(page_id);

    while (iic_eeprom_write(addr, p_buf, len))
    {
        // printf("time out\n");
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending
    };
}

void eeprom_24cxx_read(u8 page_id, u8 *p_buf, u16 len)
{
    u16 addr = EEPROM_PAGE_X_ADDR(page_id);
    while (iic_eeprom_read(addr, p_buf, len))
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending
    };
}

void iic_config(void)
{
    // 初始化IIC
    IIC_SDA = 1; // 设置为高电平
    IIC_SCL = 1; // 设置为高电平

    // P31 芯片22脚，用作SCL
    // P26 芯片25脚，用作SDA
    P3_MD0 &= ~GPIO_P31_MODE_SEL(0x03);
    P3_MD0 |= GPIO_P31_MODE_SEL(0x01);
    FOUT_S31 = GPIO_FOUT_AF_FUNC; // 选择AF功能输出

    P2_MD1 &= ~GPIO_P26_MODE_SEL(0x03);
    P2_MD1 |= GPIO_P26_MODE_SEL(0x1);
    FOUT_S26 = GPIO_FOUT_AF_FUNC; // 选择AF功能输出
}

#endif
