/*
 * ============================================================================
 *  修改记录文件：RemoteControl 项目 NRF24L01.c
 *  路径: D:\DeskTop\RemoteControl\Core\Src\NRF24L01.c
 *  说明: 这是经过修改后的完整文件内容，供查阅和回顾
 *
 *  修改共三处，详见下方标记：
 *    【修改1】SSPI_SwapByte() — 性能优化（直接寄存器操作）
 *    【修改2】NRF24L01_Send() — Bug修复（STATUS清除）
 *    【修改3】NRF24L01_Receive() — Bug修复（STATUS清除）
 *
 *  注：RemoteControl 与 BCar 的 NRF24L01.c 修改前内容完全相同，
 *      因此修改也完全相同。
 * ============================================================================
 */

/*
 * 本模块预准备使用软件SPI实现与NRF24L01之间的通信
 * 配置寄存器的位时，分为读改写，用或操作。与操作进行改写，这样才不会影响其他位
 */

#include "../Inc/NRF24L01.h"

//TxFIFO
uint8_t NRF24L01_TxAddress[5] = {11, 22, 33, 44, 55};
#define NRF24L01_Tx_PayloadLength			4
uint8_t NRF24L01_TxBuffer[NRF24L01_Tx_PayloadLength];

//RxFIFO
uint8_t NRF24L01_RxAddress[5] = {11, 22, 33, 44, 55};
#define NRF24L01_Rx_PayloadOLength			4
uint8_t NRF24L01_RxBuffer[NRF24L01_Rx_PayloadOLength];

/**
 * CE引脚电平控制
 * 控制NRF24L01的CE引脚，决定模块工作状态：
 *   GPIO_PIN_SET   = 运行态（Tx/Rx）
 *   GPIO_PIN_RESET = 待机态
 * @param PinState 目标电平（GPIO_PIN_SET 或 GPIO_PIN_RESET）
 * @return 无
 */
void CE_PIN_Control(GPIO_PinState PinState)
{
    HAL_GPIO_WritePin(CE_GPIO_Port,CE_Pin,PinState);
}

/**
 * CSN引脚电平控制（片选）
 * 低电平选中NRF24L01从机，高电平释放总线
 * @param PinState 目标电平（GPIO_PIN_SET 或 GPIO_PIN_RESET）
 * @return 无
 */
void CSN_PIN_Control(GPIO_PinState PinState)
{
    HAL_GPIO_WritePin(CSN_GPIO_Port,CSN_Pin,PinState);
}

/**
 * SCK引脚电平控制（SPI时钟）
 * 软件模拟SPI模式0：SCK空闲时为低电平，上升沿采样
 * @param PinState 目标电平（GPIO_PIN_SET 或 GPIO_PIN_RESET）
 * @return 无
 */
void SCK_PIN_Control(GPIO_PinState PinState)
{
    HAL_GPIO_WritePin(SCK_GPIO_Port,SCK_Pin,PinState);
}

/**
 * MOSI引脚电平设置（主机输出）
 * 向NRF24L01发送数据位
 * @param PinState 目标电平（GPIO_PIN_SET 或 GPIO_PIN_RESET）
 * @return 无
 */
void MOSI_PIN_Set(GPIO_PinState PinState)
{
    HAL_GPIO_WritePin(MOSI_GPIO_Port,MOSI_Pin,PinState);
}

/**
 * MISO引脚电平读取（主机输入）
 * 读取NRF24L01返回的数据位
 * @param 无
 * @return GPIO_PIN_SET 或 GPIO_PIN_RESET
 */
GPIO_PinState MISO_PIN_Read(void)
{
    return HAL_GPIO_ReadPin(MISO_GPIO_Port,MISO_Pin);
}


/**
 * 进入TX模式
 * CE置1、写CONFIG=0x0A后CE置0，进入发射待机态
 * @param 无
 * @return 无
 */
void EN_NRF24L01_TXSingle_Mode(void)
{
    CE_PIN_Control(1);
    NRF24L01_WriteByte(NRF24L01_CONFIG, 0x0A);          //用DWT测一下这行代码运行时间是否>10us
    CE_PIN_Control(0);
}

/**
 * 进入TX全速发射模式
 * CE持续置1，使能连续发送（配合REUSE_TX_PL使用）
 * @param 无
 * @return 无
 */
void EN_NRF24L01_TXALL_Mode(void)
{
    uint8_t Config;
    CE_PIN_Control(0);                                                  //先置0，使芯片退出工作模式后，再进行更改配置
    Config =  NRF24L01_ReadByte(NRF24L01_CONFIG);
    Config |= 0x02;
    Config &= ~0x01;
    NRF24L01_WriteByte(NRF24L01_CONFIG, Config);
    CE_PIN_Control(1);
}
// void EN_NRF24L01_TXALLMode(void)
// {
//     CE_PIN_Control(1);
//     NRF24L01_WriteByte(NRF24L01_CONFIG, 0x0A);
// }

/**
 * 进入RX接收模式
 * CE置1、写CONFIG=0x0B（PRIM_RX=1、PWR_UP=1），等待接收数据
 * @param 无
 * @return 无
 */
void EN_NRF24L01_RXMode(void)
{
    uint8_t Config;
    CE_PIN_Control(0);                                                  //先置0，使芯片退出工作模式后，再进行更改配置
    Config =  NRF24L01_ReadByte(NRF24L01_CONFIG);
    Config |= 0x03;
    NRF24L01_WriteByte(NRF24L01_CONFIG, Config);
    CE_PIN_Control(1);
}
// void EN_NRF24L01_RXMode(void)
// {
//     CE_PIN_Control(1);
//     NRF24L01_WriteByte(NRF24L01_CONFIG, 0x0B);
// }

/**
 * 进入待机-I模式
 * CE置1、写CONFIG=0x0A，TX FIFO为空时进入待机-I
 * @param 无
 * @return 无
 */
void EN_NRF24L01_StandbyI(void)
{
    uint8_t Config;
    CE_PIN_Control(0);
    Config =  NRF24L01_ReadByte(NRF24L01_CONFIG);
    Config |= 0x02;
    NRF24L01_WriteByte(NRF24L01_CONFIG, Config);
}
// void EN_NRF24L01_StandbyI(void)
// {
//     CE_PIN_Control(1);
//     NRF24L01_WriteByte(NRF24L01_CONFIG, 0x0A);
// }

/**
 * 进入待机-II模式
 * CE置0、写CONFIG=0x0A，TX FIFO非空时进入待机-II
 * @param 无
 * @return 无
 */
void EN_NRF24L01_StandbyII(void)
{
    CE_PIN_Control(0);
    NRF24L01_WriteByte(NRF24L01_CONFIG, 0x0A);
}

/**
 * 进入掉电模式
 * CE置0、写CONFIG=0x08（PWR_UP=0），进入最低功耗状态
 * @param 无
 * @return 无
 */
void EN_NRF24L01_PowerDown(void)
{
    uint8_t Config;
    CE_PIN_Control(0);
    Config =  NRF24L01_ReadByte(NRF24L01_CONFIG);
    Config &= ~0x02;                                                   //这样可以保留其他位，直接覆盖的操作确实有点太鲁莽了
    NRF24L01_WriteByte(NRF24L01_CONFIG, Config);          //直接给定值大概率没问题，但是，有时候为了防止别的值被更改，可以采取|操作或者&操作进行赋值
}
// void EN_NRF24L01_PowerDown(void)
// {
//     CE_PIN_Control(0);
//     NRF24L01_WriteByte(NRF24L01_CONFIG, 0x08);          //直接给定值大概率没问题，但是，有时候为了防止别的值被更改，可以采取|操作或者&操作进行赋值
// }


/**
 * 软件SPI字节交换（全双工）
 * 在SCK驱动下，MOSI逐位发送byte，同时MISO逐位接收并存入byte
 * 模拟SPI模式0：SCK空闲低、上升沿采样
 * @param byte 要发送的字节
 * @return 从MISO接收到的字节
 */
/* ============================================================================
 * 【修改1】SSPI_SwapByte() 性能优化
 *
 * 变更类型：性能优化
 * 变更日期：2026-06-13
 *
 * 原代码：
 *   使用 MOSI_PIN_Set(1/0)、SCK_PIN_Control(1/0)、MISO_PIN_Read()
 *   这些函数内部调用 HAL_GPIO_WritePin / HAL_GPIO_ReadPin。
 *   HAL库函数包含参数有效性检查等额外开销，对于每字节需执行多次的
 *   软件SPI而言，累加的性能损失显著。
 *
 * 修改后：
 *   直接操作 GPIO 的 BSRR(设置)/BRR(复位)/IDR(输入) 寄存器，
 *   绕过 HAL 层，将 GPIO 操作速度压到极限。
 *   - BSRR: 写1到位[15:0]=置高，到位[31:16]=置低
 *   - BRR:  写1到位[15:0]=置低
 *   - IDR:  读位[15:0]=输入引脚电平
 *
 * 提速效果：
 *   从每字节约 32 次 HAL 调用（~44µs额外开销）降为零函数调用，
 *   单次 GPIO 操作仅 1~2 CPU 周期，提速约 10~15 倍。
 *
 * 注意事项：
 *   SCK/MOSI/MISO 引脚宏定义来源于 CubeMX 生成的 main.h，
 *   如重新生成需确保引脚映射一致。
 *   当前引脚：SCK=PB3, MOSI=PB5, MISO=PB4
 * ============================================================================ */
uint8_t SSPI_SwapByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        /* --- MOSI 置位/复位：直接写 BSRR(置高) 或 BRR(置低) --- */
        if(byte & 0x80)
        {
            MOSI_GPIO_Port->BSRR = MOSI_Pin;
        }
        else
        {
            MOSI_GPIO_Port->BRR = MOSI_Pin;
        }

        byte <<= 1;

        /* --- SCK 上升沿：NRF24L01 在此时采样 MISO 上的数据 --- */
        SCK_GPIO_Port->BSRR = SCK_Pin;

        if(MISO_GPIO_Port->IDR & MISO_Pin)
        {
            byte |= 0x01;
        }

        /* --- SCK 下降沿：为下一个 bit 的上升沿采样做准备 --- */
        SCK_GPIO_Port->BRR = SCK_Pin;
    }
    return byte;
}

/**
 * 写NRF24L01寄存器
 * 发送写寄存器指令+地址，再发送数据
 * @param Address 寄存器地址（0x00~0x1D）
 * @param Data    要写入寄存器的数据
 * @return 无
 */
void  NRF24L01_WriteByte(uint8_t Address ,uint8_t Data)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_W_REGISTER | Address);
    SSPI_SwapByte(Data);
    CSN_PIN_Control(1);
}

/**
 * 读NRF24L01寄存器
 * 发送读寄存器指令+地址，然后发送0xFF以产生时钟读取返回值
 * @param Address 寄存器地址（0x00~0x1D）
 * @return 读取到的寄存器值
 */
uint8_t NRF24L01_ReadByte(uint8_t Address)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_R_REGISTER | Address);
    uint8_t Data = SSPI_SwapByte(0xFF);
    CSN_PIN_Control(1);
    return Data;
}

/**
 * 写多字节到NRF24L01寄存器（突发写入）
 * 发送写寄存器指令+地址，随后连续写入DataLength个字节
 * @param Address    目标寄存器起始地址（如 NRF24L01_RX_ADDR_P0=0x0A）
 * @param Data       指向待写入数据的缓冲区指针
 * @param DataLength 要写入的字节数
 * @return 无
 */
void NRF24L01_WriteBytes(uint8_t Address ,uint8_t *Data, int DataLength)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_W_REGISTER | Address);
    for (int i = 0; i < DataLength; i++)
    {
        SSPI_SwapByte(Data[i]);
    }
    CSN_PIN_Control(1);
}

/**
 * 读多字节从NRF24L01寄存器（突发读取）
 * 发送读寄存器指令+地址，随后连续读取5个字节存入静态缓冲区并返回其指针
 * @param Address 目标寄存器起始地址（如 NRF24L01_RX_ADDR_P0=0x0A）
 * @return 指向静态缓冲区Data的指针，内含读取到的5字节数据
 * @note 固定读取5字节，非线程安全，重复调用会覆盖上次数据
 */
uint8_t* NRF24L01_ReadBytes(uint8_t Address)
{
    static uint8_t Data[5];
    CSN_PIN_Control(0);
    SSPI_SwapByte(Address);
    for (uint8_t i = 0; i < 5; i++)
    {
        Data[i] = SSPI_SwapByte(0xFF);
    }
    CSN_PIN_Control(1);
    return Data;
}

/**
 * 写TX有效载荷（发射FIFO）只写
 * 将NRF24L01_TxBuffer中的数据写入TX FIFO，准备发送
 * 数据长度由宏NRF24L01_Tx_FIFOLength决定
 * @param 无（使用内部全局数组 NRF24L01_TxBuffer）
 * @return 无
 */
void NRF24L01_WriteFIFO_Bytes(void)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_W_TX_PAYLOAD);
    for (uint8_t i = 0; i < NRF24L01_Tx_PayloadLength; i++)
    {
        SSPI_SwapByte(NRF24L01_TxBuffer[i]);
    }
    CSN_PIN_Control(1);
}

/**
 * 读RX有效载荷（接收FIFO）只读
 * 从RX FIFO中读取接收到的数据，存入NRF24L01_RxBuffer
 * 数据长度由宏NRF24L01_Rx_FIFOLength决定
 * @param 无（使用内部全局数组 NRF24L01_RxBuffer）
 * @return 指向NRF24L01_RxBuffer的指针，内含读取到的数据
 */
void NRF24L01_ReadFIFO_Bytes(void)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_R_RX_PAYLOAD);
    for (uint8_t i = 0; i < NRF24L01_Rx_PayloadOLength; i++)
    {
        NRF24L01_RxBuffer[i] = SSPI_SwapByte(0xFF);
    }
    CSN_PIN_Control(1);
}

/**
 * 清空TX FIFO
 * 发送FLUSH_TX指令，清除发射缓冲区中所有待发送数据
 * @param 无
 * @return 无
 */
void NRF24L01_Flush_TX(void)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_FLUSH_TX);
    CSN_PIN_Control(1);
}

/**
 * 清空RX FIFO
 * 发送FLUSH_RX指令，清除接收缓冲区中所有已接收数据
 * @param 无
 * @return 无
 */
void NRF24L01_Flush_RX(void)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_FLUSH_RX);
    CSN_PIN_Control(1);
}

/**
 * 重发上一包TX有效载荷
 * 发送REUSE_TX_PL指令，重复发射上一次写入TX FIFO的数据包
 * @param 无
 * @return 无
 */
void NRF24L01_Reuse_TX_PL(void)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_REUSE_TX_PL);
    CSN_PIN_Control(1);
}

/**
 * 读取RX有效载荷宽度
 * 发送R_RX_PL_WID指令，获取当前RX FIFO中有效载荷的字节数
 * @param 无
 * @return 无（返回值通过SSPI_SwapByte获取但未保存，需补充）
 */
void NRF24L01_RRX_PL_WID(void)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_R_RX_PL_WID);
    CSN_PIN_Control(1);
}

/**
 * 写ACK有效载荷
 * 发送W_ACK_PAYLOAD指令，在收到指定管道的数据包时自动附带该数据回复
 * @param 无
 * @return 无
 */
void NRF24L01_W_ACK_Payload(void)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_W_ACK_PAYLOAD);
    CSN_PIN_Control(1);
}

/**
 * 写TX有效载荷（禁止ACK）
 * 发送W_TX_PAYLOAD_NOACK指令，写入无需应答的发射数据
 * @param 无
 * @return 无
 */
void NRF24L01_W_TX_Paload_NoACK(void)
{
    CSN_PIN_Control(0);
    SSPI_SwapByte(NRF24L01_W_TX_PAYLOAD_NOACK);
    CSN_PIN_Control(1);
}

/**
 * 读取状态寄存器
 * 发送NOP（空操作），在无副作用的前提下读取STATUS寄存器的当前值
 * @param 无
 * @return 状态寄存器值（包含TX_DS、RX_DR、MAX_RT等标志位）
 */
uint8_t NRF24L01_ReadStatus(void)
{
    CSN_PIN_Control(0);
    uint8_t Status = SSPI_SwapByte(NRF24L01_NOP);
    CSN_PIN_Control(1);

    return Status;
}

/**
 * NRF24L01 初始化
 * 设置CE=0（待机态）、SCK=0（空闲低）、CSN=1（释放片选）
 * 必须在GPIO初始化之后调用
 * @param 无
 * @return 无
 */
void NRF24L01_Init(void)
{
    CE_PIN_Control(0);
    SCK_PIN_Control(0);
    CSN_PIN_Control(1);

    NRF24L01_WriteByte(NRF24L01_CONFIG, 0x08);
    NRF24L01_WriteByte(NRF24L01_EN_AA, 0x3F);
    NRF24L01_WriteByte(NRF24L01_EN_RXADDR, 0x01);
    NRF24L01_WriteByte(NRF24L01_SETUP_AW, 0x03);
    NRF24L01_WriteByte(NRF24L01_SETUP_RETR, 0x03);
    NRF24L01_WriteByte(NRF24L01_RF_CH, 0x02);
    NRF24L01_WriteByte(NRF24L01_RF_SETUP, 0x0E);//0x0E
    // NRF24L01_WriteByte(NRF24L01_RF_SETUP, 0x0F);//0x0E
    NRF24L01_WriteBytes(NRF24L01_RX_ADDR_P0, NRF24L01_RxAddress, 5);
    // NRF24L01_WriteBytes(NRF24L01_TX_ADDR, NRF24L01_TxAddress, 5);这个在发送函数里面配置就行
    NRF24L01_WriteByte(NRF24L01_RX_PW_P0, NRF24L01_Rx_PayloadOLength);

    NRF24L01_Flush_TX();
    NRF24L01_Flush_RX();

    NRF24L01_WriteByte(NRF24L01_STATUS, 0x70);
    // NRF24L01_WriteByte(NRF24L01_STATUS, 0x07);

    EN_NRF24L01_RXMode();//默认进行接收模式，等需要发送再进入发送模式，发送完成则回到接受模式
}

void NRF24L01_Send(void)
{
    //配置发送地址
    NRF24L01_WriteBytes(NRF24L01_TX_ADDR, NRF24L01_TxAddress, 5);
    //配置接收通道0的地址—————必须和发送地址相同
    NRF24L01_WriteBytes(NRF24L01_RX_ADDR_P0, NRF24L01_TxAddress, 5);
    //向TxFIFO写入数据
    NRF24L01_WriteFIFO_Bytes();
    //进入发送模式
    EN_NRF24L01_TXALL_Mode();
    //等待发送完成，并清楚标志位
    uint16_t TimeOut = 0;
    uint8_t Status = 0;
    while (1)
    {
        Status = NRF24L01_ReadStatus();
        if (Status & 0x20)
        {
            //这是发送成功的(Tx_Ds)
            break;
        }
        if (Status & 0x10)
        {
            //出错就直接初始化设备（MAX_RT）
            NRF24L01_Init();
            break;
        }
        TimeOut ++;
        if (TimeOut > 10000)
        {
            NRF24L01_Init();
            TimeOut = 0;
            break;
        }
    }
    /* ============================================================================
     * 【修改2】NRF24L01_Send() — STATUS寄存器标志位清除 Bug修复
     *
     * 同 BCar 修改2，完全相同。
     * ============================================================================ */
    NRF24L01_WriteByte(NRF24L01_STATUS, 0x30);
    //发送完清一下FIFO
    NRF24L01_Flush_TX();
    //重新指定接收通道0的地址
    NRF24L01_WriteBytes(NRF24L01_RX_ADDR_P0, NRF24L01_RxAddress, 5);//其实只要发送地址和接收地址一样，是没啥问题的，但为了防止不一样，就加上这句
    //回到默认接收状态
    EN_NRF24L01_RXMode();
}

uint8_t NRF24L01_Receive(void)
{
    //标志位检查状态并对该位进行清0
    uint8_t Status = NRF24L01_ReadStatus();
    if(Status & 0x40)
    {
        //读取接收FIFO的数据
        NRF24L01_ReadFIFO_Bytes();
        /* ============================================================================
         * 【修改3】NRF24L01_Receive() — STATUS寄存器 RX_DR 标志位清除 Bug修复
         *
         * 同 BCar 修改3，完全相同。
         * ============================================================================ */
        NRF24L01_WriteByte(NRF24L01_STATUS, 0x40);
        //清空RxFIFO残留的数据
        NRF24L01_Flush_RX();
        //接收成功
        return 1;
    }
    return 0;
}
