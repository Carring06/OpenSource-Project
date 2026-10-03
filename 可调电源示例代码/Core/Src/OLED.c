/***************************************************************************************
  * 通信协议栈为江科大自制，开源免费
  * 基于HAL库开发，适配STM32F103C8T6
  * 原版为标准库，此处修改为HAL库版本
  ***************************************************************************************
  */

// 替换标准库头文件为HAL库头文件
#include "stm32f1xx_hal.h"
#include "OLED.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/**
  * 数据存储格式：
  * 每8列，高位在下，先从低字节开始
  * 每一个Bit对应一个像素点
  *
  *      B0 B0                  B0 B0
  *      B1 B1                  B1 B1
  *      B2 B2                  B2 B2
  *      B3 B3  ------------->  B3 B3 --
  *      B4 B4                  B4 B4  |
  *      B5 B5                  B5 B5  |
  *      B6 B6                  B6 B6  |
  *      B7 B7                  B7 B7  |
  *                                    |
  *  -----------------------------------
  *  |
  *  |   B0 B0                  B0 B0
  *  |   B1 B1                  B1 B1
  *  |   B2 B2                  B2 B2
  *  --> B3 B3  ------------->  B3 B3
  *      B4 B4                  B4 B4
  *      B5 B5                  B5 B5
  *      B6 B6                  B6 B6
  *      B7 B7                  B7 B7
  *
  * 坐标定义：
  * 左上角为(0, 0)点
  * 横向为X轴，取值范围0~127
  * 纵向为Y轴，取值范围0~63
  *
  *       0             X轴           127
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   63 |
  *      v
  *
  */


/*全局变量*********************/

/**
  * OLED显存数组
  * 所有的显示操作，都只是对显存数组进行修改
  * 必须调用OLED_Update函数或OLED_UpdateArea函数
  * 才会将显存数组的数据发送到OLED硬件进行显示
  */
uint8_t OLED_DisplayBuf[8][128];

/*********************全局变量*/


/*底层驱动*********************/
/**
  * 函    数：OLED写SCL高低电平
  * 参    数：要写入SCL的电平值，范围0/1
  * 返 回 值：无
  * 说    明：原版为库函数，修改为HAL_GPIO_WritePin函数
  */
void OLED_W_SCL(uint8_t BitValue)
{
	/*根据BitValue的值，将SCL置高电平或低电平*/
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);

	/*单片机速度过快，用于延时保证时序，避免I2C通信的速度过快*/
	//HAL_Delay(1);
}

/**
  * 函    数：OLED写SDA高低电平
  * 参    数：要写入SDA的电平值，范围0/1
  * 返 回 值：无
  * 说    明：原版为库函数，修改为HAL_GPIO_WritePin函数
  */
void OLED_W_SDA(uint8_t BitValue)
{
	/*根据BitValue的值，将SDA置高电平或低电平*/
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);

	/*单片机速度过快，用于延时保证时序，避免I2C通信的速度过快*/
	//HAL_Delay(1);
}

///**
//  * 函    数：OLED写SCL高低电平
//  * 参    数：要写入SCL的电平值，范围0/1
//  * 返 回 值：无
//  * 说    明：原版为库函数，修改为HAL_GPIO_WritePin函数
//  */
//void OLED_W_SCL(uint8_t BitValue)
//{
//	/*根据BitValue的值，将SCL置高电平或低电平*/
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);

//	/*单片机速度过快，用于延时保证时序，避免I2C通信的速度过快*/
//	//HAL_Delay(1);
//}

///**
//  * 函    数：OLED写SDA高低电平
//  * 参    数：要写入SDA的电平值，范围0/1
//  * 返 回 值：无
//  * 说    明：原版为库函数，修改为HAL_GPIO_WritePin函数
//  */
//void OLED_W_SDA(uint8_t BitValue)
//{
//	/*根据BitValue的值，将SDA置高电平或低电平*/
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);

//	/*单片机速度过快，用于延时保证时序，避免I2C通信的速度过快*/
//	//HAL_Delay(1);
//}

/*
(1)	注意：如果 CubeMX 中 PB8/PB9 只配置了 Alternate Function Open Drain 一种模式，说明硬件I2C被占用，
	导致I2C通信无法响应，这时候必须按照注释中的方式重新配置，需要在 CubeMX 中重新配置。
(2) CubeMX 中重新配置只需要将这两个引脚，修改为通用推挽输出模式
	然后程序直接操作寄存器控制 PB8/PB9 的电平。 切换为 软件模拟I2C必须 CubeMX 正确配置。
	所以 OLED_W_SCL 和 OLED_W_SDA 函数才能正常控制电平，模拟I2C通信成功。
*/

/**

	@brief 当CubeMX中原本无法使用I2C的SDA,SCL引脚为软件模拟引脚时，在I2C初始化后，调用此函数
	@param  无
	@return 无
*/
void OLED_GPIO_ForceInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* 使能 GPIOB 时钟，必须开启 */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* 配置 PB8 (SCL) */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;	  // 开漏输出
	GPIO_InitStruct.Pull = GPIO_PULLUP;			  // 上拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* 配置 PB9 (SDA) */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* 初始化总线，SCL/SDA 置高 */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
}

/**
  * 函    数：OLED引脚初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：使用CubeMX配置硬件初始化，此处配置总线时序，释放引脚
  */
void OLED_GPIO_Init(void)
{
	uint32_t i, j;

	/*在初始化前适当延时一段时间，让OLED模块更稳定*/
	for (i = 0; i < 1000; i ++)
	{
		for (j = 0; j < 1000; j ++);
	}

	/*CubeMX配置GPIO初始化，此处释放SCL、SDA为高电平*/
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/*********************底层驱动*/


/*通信协议*********************/

/**
  * 函    数：I2C起始
  * 参    数：无
  * 返 回 值：无
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);		//释放SDA，确保SDA为高电平
	OLED_W_SCL(1);		//释放SCL，确保SCL为高电平
	OLED_W_SDA(0);		//在SCL高电平期间，拉低SDA，产生起始信号
	OLED_W_SCL(0);		//起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

/**
  * 函    数：I2C终止
  * 参    数：无
  * 返 回 值：无
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);		//拉低SDA，确保SDA为低电平
	OLED_W_SCL(1);		//释放SCL，使SCL呈现高电平
	OLED_W_SDA(1);		//在SCL高电平期间，释放SDA，产生终止信号
}

/**
  * 函    数：I2C发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围0x00~0xFF
  * 返 回 值：无
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;

	/*循环8次，依次发送数据的每一位*/
	for (i = 0; i < 8; i++)
	{
		/*使用移位的方式取出Byte的指定一位数据并写入到SDA线*/
		/*两个!!是为了保证取出的值为1*/
		OLED_W_SDA(!!(Byte & (0x80 >> i)));
		OLED_W_SCL(1);	//释放SCL，上升沿，SCL高电平期间读取SDA
		OLED_W_SCL(0);	//拉低SCL，开始发送下一位
	}

	OLED_W_SCL(1);		//额外一个时钟，等待应答信号
	OLED_W_SCL(0);
}

/**
  * 函    数：OLED写命令
  * 参    数：Command 要写入的命令值，范围0x00~0xFF
  * 返 回 值：无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();				//I2C起始
	OLED_I2C_SendByte(0x78);		//发送OLED的I2C从机地址
	OLED_I2C_SendByte(0x00);		//第一个字节，0x00表示接下来写命令
	OLED_I2C_SendByte(Command);		//写入指令参数
	OLED_I2C_Stop();				//I2C终止
}

/**
  * 函    数：OLED写数据
  * 参    数：Data 要写入数据的起始地址
  * 参    数：Count 要写入数据的数量
  * 返 回 值：无
  */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
	uint8_t i;

	OLED_I2C_Start();				//I2C起始
	OLED_I2C_SendByte(0x78);		//发送OLED的I2C从机地址
	OLED_I2C_SendByte(0x40);		//第一个字节，0x40表示接下来写数据
	/*循环Count次，依次进行数据的写入*/
	for (i = 0; i < Count; i ++)
	{
		OLED_I2C_SendByte(Data[i]);	//依次发送Data的每一个数据
	}
	OLED_I2C_Stop();				//I2C终止
}

/*********************通信协议*/


/*硬件配置*********************/

/**
  * 函    数：OLED初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：使用前，必须调用此初始化函数
  */
void OLED_Init(void)
{
	HAL_Delay(150);
	
	OLED_GPIO_Init();			//先进行底层的端口初始化

	/*写入一系列的指令，对OLED进行初始化配置*/
	OLED_WriteCommand(0xAE);	//关闭显示，休眠/唤醒，0xAE休眠，0xAF开启

	OLED_WriteCommand(0xD5);	//设置显示时钟分频/震荡频率
	OLED_WriteCommand(0x80);	//0x00~0xFF

	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);	//0x0E~0x3F

	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);	//0x00~0x7F

	OLED_WriteCommand(0x40);	//设置显示开始行，0x40~0x7F

	OLED_WriteCommand(0xA1);	//设置左右反转，0xA1不反转，0xA0反转

	OLED_WriteCommand(0xC8);	//设置上下反转，0xC8不反转，0xC0反转

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);

	OLED_WriteCommand(0x81);	//设置对比度
	OLED_WriteCommand(0xCF);	//0x00~0xFF

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示点亮/熄灭

	OLED_WriteCommand(0xA6);	//设置正常/反色显示，0xA6正常，0xA7反色

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示

	OLED_Clear();				//清空显存数组
	OLED_Update();				//更新显示，清屏，防止初始化后未显示内容时花屏
}

/**
  * 函    数：OLED设置显示光标位置
  * 参    数：Page 指定位图的页数，范围0~7
  * 参    数：X 指定位图的X坐标点，范围0~127
  * 返 回 值：无
  * 说    明：OLED默认的Y轴，以8位Bit为一个写入单位，1页代表8个Y坐标
  */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
	/*注意：使用此函数，1.3寸OLED显示屏需要特别注意*/
	/*因为1.3寸OLED驱动芯片为SH1106，自带132列*/
	/*屏幕起始位置占了2列，并非从0开始*/
	/*所以需要将X+2才能正常显示*/
//	X += 2;

	/*通过指令设置页地址和列地址*/
	OLED_WriteCommand(0xB0 | Page);					//设置页位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/*********************硬件配置*/


/*辅助函数*********************/

/*辅助函数仅内部函数调用使用*/

/**
  * 函    数：数字幂运算
  * 参    数：X 底数
  * 参    数：Y 指数
  * 返 回 值：X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//结果默认为1
	while (Y --)			//累乘Y次
	{
		Result *= X;		//每次把X累乘到结果上
	}
	return Result;
}

/**
  * 函    数：判断点是否在指定多边形内部
  * 参    数：nvert 多边形的顶点数
  * 参    数：vertx verty 多边形的x和y坐标数组
  * 参    数：testx testy 测试的X和y坐标
  * 返 回 值：点是否在指定多边形内部，1在内部，0不在内部
  */
uint8_t OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
	int16_t i, j, c = 0;

	/*算法为W. Randolph Franklin提出*/
	/*参考网址：https://wrfranklin.org/Research/Short_Notes/pnpoly.html*/
	for (i = 0, j = nvert - 1; i < nvert; j = i++)
	{
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
		{
			c = !c;
		}
	}
	return c;
}

/**
  * 函    数：判断点是否在指定扇形内部
  * 参    数：X Y 指定坐标
  * 参    数：StartAngle EndAngle 起始角度和终止角度，范围-180~180
  *           水平向右为0度，水平向左为180度或-180度，向下为正，向上为负，顺时针旋转
  * 返 回 值：点是否在指定扇形内部，1在内部，0不在内部
  */
uint8_t OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
	int16_t PointAngle;
	PointAngle = atan2(Y, X) / 3.14 * 180;	//计算坐标的弧度，转换为角度表示
	if (StartAngle < EndAngle)	//起始角度小于终止角度时
	{
		/*若点角度在起始和终止角度之间，则判定点在指定角度内*/
		if (PointAngle >= StartAngle && PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	else			//起始角度大于终止角度时
	{
		/*若点角度大于等于起始角度或小于等于终止角度，则判定点在指定角度内*/
		if (PointAngle >= StartAngle || PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	return 0;		//如果以上条件都不满足，则判定点不在指定角度内
}

/*********************辅助函数*/


/*显示函数*********************/

/**
  * 函    数：将OLED显存数组更新到OLED屏幕
  * 参    数：无
  * 返 回 值：无
  * 说    明：所有的显示操作，都只是对OLED显存数组进行修改
  *           必须调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件进行显示
  *           真正的显示内容，必须调用该函数才能刷写到屏幕上
  */
void OLED_Update(void)
{
	uint8_t j;
	/*遍历每一页*/
	for (j = 0; j < 8; j ++)
	{
		/*设置光标位置为每一页的第一个*/
		OLED_SetCursor(j, 0);
		/*依次写入128个数据，将显存数组写入到OLED硬件*/
		OLED_WriteData(OLED_DisplayBuf[j], 128);
	}
}

/**
  * 函    数：将OLED显存数组部分更新到OLED屏幕
  * 参    数：X 指定区域左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Width 指定区域的宽度，范围0~128
  * 参    数：Height 指定区域的高度，范围0~64
  * 返 回 值：无
  * 说    明：此函数可以只更新指定区域，提高刷新速度
  *           但是Y坐标只计算整页，同一页剩余部分会一并刷新
  * 说    明：所有的显示操作，都只是对OLED显存数组进行修改
  *           必须调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件进行显示
  *           真正的显示内容，必须调用该函数才能刷写到屏幕上
  */
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t j;
	int16_t Page, Page1;

	/*坐标在计算页地址时需要加一个偏移*/
	/*(Y + Height - 1) / 8 + 1的目的是(Y + Height) / 8向上取整*/
	Page = Y / 8;
	Page1 = (Y + Height - 1) / 8 + 1;
	if (Y < 0)
	{
		Page -= 1;
		Page1 -= 1;
	}

	/*遍历指定区域涉及的页*/
	for (j = Page; j < Page1; j ++)
	{
		if (X >= 0 && X <= 127 && j >= 0 && j <= 7)		//在屏幕范围内进行显示
		{
			/*设置光标位置为该页指定位置*/
			OLED_SetCursor(j, X);
			/*依次写入Width个数据，将显存数组写入到OLED硬件*/
			OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
		}
	}
}

/**
  * 函    数：将OLED显存数组全部清零
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正清屏
  */
void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//遍历8页
	{
		for (i = 0; i < 128; i ++)			//遍历128列
		{
			OLED_DisplayBuf[j][i] = 0x00;	//将显存数组全部清零
		}
	}
}

/**
  * 函    数：将OLED显存数组部分清零
  * 参    数：X 指定区域左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Width 指定区域的宽度，范围0~128
  * 参    数：Height 指定区域的高度，范围0~64
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正清屏
  */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t i, j;

	for (j = Y; j < Y + Height; j ++)		//遍历指定页
	{
		for (i = X; i < X + Width; i ++)	//遍历指定列
		{
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)				//在屏幕范围内进行显示
			{
				OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));	//将显存数组指定像素点清零
			}
		}
	}
}

/**
  * 函    数：将OLED显存数组全部取反
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正取反
  */
void OLED_Reverse(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//遍历8页
	{
		for (i = 0; i < 128; i ++)			//遍历128列
		{
			OLED_DisplayBuf[j][i] ^= 0xFF;	//将显存数组全部取反
		}
	}
}

/**
  * 函    数：将OLED显存数组部分取反
  * 参    数：X 指定区域左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Width 指定区域的宽度，范围0~128
  * 参    数：Height 指定区域的高度，范围0~64
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正取反
  */
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t i, j;

	for (j = Y; j < Y + Height; j ++)		//遍历指定页
	{
		for (i = X; i < X + Width; i ++)	//遍历指定列
		{
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)			//在屏幕范围内进行显示
			{
				OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);	//将显存数组指定像素点取反
			}
		}
	}
}

/**
  * 函    数：OLED显示一个字符
  * 参    数：X 指定字符左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定字符左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Char 指定要显示的字符，范围为ASCII码可见字符
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
	if (FontSize == OLED_8X16)		//字体为宽8像素，高16像素
	{
		/*从ASCII字库OLED_F8x16中指定位置取出8*16图像以图片形式显示*/
		OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
	}
	else if(FontSize == OLED_6X8)	//字体为宽6像素，高8像素
	{
		/*从ASCII字库OLED_F6x8中指定位置取出6*8图像以图片形式显示*/
		OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
	}
}

/**
  * 函    数：OLED显示字符串，支持ASCII码（屏幕滚动）
  * 参    数：X 指定字符串左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定字符串左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：String 指定要显示的字符串，范围为ASCII码可见字符组成的字符串
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：显示中文需要在OLED_Data.c中OLED_CF16x16数组定义
  *           未找到指定汉字时，显示默认图像，一个问号或者一个空格
  *           字体大小为OLED_8X16时，汉字显示16*16点阵显示
  *           字体大小为OLED_6X8时，汉字显示6*8点阵显示'?'
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
	uint16_t i = 0;
	char SingleChar[5];
	uint8_t CharLength = 0;
	uint16_t XOffset = 0;
	uint16_t pIndex;

	while (String[i] != '\0')	//遍历字符串
	{

#ifdef OLED_CHARSET_UTF8						//指定字符串编码为UTF8
		/*此处的目的是，取出UTF8字符串中的一个字符，转换到SingleChar字符串*/
		/*判断UTF8编码第一个字节的标志位*/
		if ((String[i] & 0x80) == 0x00)			//第一个字节为0xxxxxxx
		{
			CharLength = 1;						//字符为1字节
			SingleChar[0] = String[i ++];		//第一个字节写入SingleChar0位置，i指向下一字节
			SingleChar[1] = '\0';				//为SingleChar字符串添加结束标志位
		}
		else if ((String[i] & 0xE0) == 0xC0)	//第一个字节为110xxxxx
		{
			CharLength = 2;						//字符为2字节
			SingleChar[0] = String[i ++];		//第一个字节写入SingleChar0位置，i指向下一字节
			if (String[i] == '\0') {break;}		//防止字符串结束，跳出循环
			SingleChar[1] = String[i ++];		//第二个字节写入SingleChar1位置，i指向下一字节
			SingleChar[2] = '\0';				//为SingleChar字符串添加结束标志位
		}
		else if ((String[i] & 0xF0) == 0xE0)	//第一个字节为1110xxxx
		{
			CharLength = 3;						//字符为3字节
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[2] = String[i ++];
			SingleChar[3] = '\0';
		}
		else if ((String[i] & 0xF8) == 0xF0)	//第一个字节为11110xxx
		{
			CharLength = 4;						//字符为4字节
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[2] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[3] = String[i ++];
			SingleChar[4] = '\0';
		}
		else
		{
			i ++;			//否则i指向下一字节，跳过该字节，继续判断下一个字节
			continue;
		}
#endif

#ifdef OLED_CHARSET_GB2312						//指定字符串编码为GB2312
		/*此处的目的是，取出GB2312字符串中的一个字符，转换到SingleChar字符串*/
		/*判断GB2312字节的高位标志位*/
		if ((String[i] & 0x80) == 0x00)			//高位为0
		{
			CharLength = 1;						//字符为1字节
			SingleChar[0] = String[i ++];		//第一个字节写入SingleChar0位置，i指向下一字节
			SingleChar[1] = '\0';				//为SingleChar字符串添加结束标志位
		}
		else									//高位为1
		{
			CharLength = 2;						//字符为2字节
			SingleChar[0] = String[i ++];		//第一个字节写入SingleChar0位置，i指向下一字节
			if (String[i] == '\0') {break;}		//防止字符串结束，跳出循环
			SingleChar[1] = String[i ++];		//第二个字节写入SingleChar1位置，i指向下一字节
			SingleChar[2] = '\0';				//为SingleChar字符串添加结束标志位
		}
#endif

		/*显示取出的单个字符SingleChar*/
		if (CharLength == 1)	//如果是单字节字符
		{
			/*使用OLED_ShowChar显示该字符*/
			OLED_ShowChar(X + XOffset, Y, SingleChar[0], FontSize);
			XOffset += FontSize;
		}
		else					//否则，多字节字符
		{
			/*遍历汉字字库，在字库中寻找该字符的位置*/
			/*找到最后一个字符，即汉字库结束，显示字符未定义字库，停止查找*/
			for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex ++)
			{
				/*找到匹配的字符*/
				if (strcmp(OLED_CF16x16[pIndex].Index, SingleChar) == 0)
				{
					break;		//跳出循环，此时pIndex的值为指定字符的位置
				}
			}
			if (FontSize == OLED_8X16)		//字体为8*16点阵
			{
				/*从字库OLED_CF16x16中指定位置取出16*16图像以图片形式显示*/
				OLED_ShowImage(X + XOffset, Y, 16, 16, OLED_CF16x16[pIndex].Data);
				XOffset += 16;
			}
			else if (FontSize == OLED_6X8)	//字体为6*8点阵
			{
				/*空间不足，高位位置显示'?'*/
				OLED_ShowChar(X + XOffset, Y, '?', OLED_6X8);
				XOffset += OLED_6X8;
			}
		}
	}
}

/**
  * 函    数：OLED显示数字，十进制，无符号数
  * 参    数：X 指定数字左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Number 指定要显示的数字，范围0~4294967295
  * 参    数：Length 指定数字的长度，范围0~10
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//遍历数字的每一位
	{
		/*使用OLED_ShowChar逐个显示每一位数字*/
		/*Number / OLED_Pow(10, Length - i - 1) % 10 十进制提取数字的每一位*/
		/*+ '0' 将数字转换为字符形式*/
		OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示有符号数字，十进制，有符号数
  * 参    数：X 指定数字左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Number 指定要显示的数字，范围-2147483648~2147483647
  * 参    数：Length 指定数字的长度，范围0~10
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	uint32_t Number1;

	if (Number >= 0)						//数字大于等于0
	{
		OLED_ShowChar(X, Y, '+', FontSize);	//显示+号
		Number1 = Number;					//Number1直接等于Number
	}
	else									//数字小于0
	{
		OLED_ShowChar(X, Y, '-', FontSize);	//显示-号
		Number1 = -Number;					//Number1等于Number取反
	}

	for (i = 0; i < Length; i++)			//遍历数字的每一位
	{
		/*使用OLED_ShowChar逐个显示每一位数字*/
		/*Number1 / OLED_Pow(10, Length - i - 1) % 10 十进制提取数字的每一位*/
		/*+ '0' 将数字转换为字符形式*/
		OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示十六进制数字，十六进制，无符号数
  * 参    数：X 指定数字左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Number 指定要显示的数字，范围0x00000000~0xFFFFFFFF
  * 参    数：Length 指定数字的长度，范围0~8
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)		//遍历数字的每一位
	{
		/*从十六进制提取数字的每一位*/
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;

		if (SingleNumber < 10)			//数字小于10
		{
			/*使用OLED_ShowChar显示该数字*/
			/*+ '0' 将数字转换为字符形式*/
			OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
		}
		else							//数字大于等于10
		{
			/*使用OLED_ShowChar显示该数字*/
			/*+ 'A' 将数字转换为以A开头的十六进制字符*/
			OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
		}
	}
}

/**
  * 函    数：OLED显示二进制数字，二进制，无符号数
  * 参    数：X 指定数字左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Number 指定要显示的数字，范围0x00000000~0xFFFFFFFF
  * 参    数：Length 指定数字的长度，范围0~16
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//遍历数字的每一位
	{
		/*使用OLED_ShowChar逐个显示每一位数字*/
		/*Number / OLED_Pow(2, Length - i - 1) % 2 从二进制提取数字的每一位*/
		/*+ '0' 将数字转换为字符形式*/
		OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示浮点数字，十进制，小数点
  * 参    数：X 指定数字左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Number 指定要显示的数字，范围-4294967295.0~4294967295.0
  * 参    数：IntLength 指定数字的整数位长度，范围0~10
  * 参    数：FraLength 指定数字的小数位长度，范围0~9，小数点后不足显示0
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
	uint32_t PowNum, IntNum, FraNum;

	if (Number >= 0)						//数字大于等于0
	{
		OLED_ShowChar(X, Y, '+', FontSize);	//显示+号
	}
	else									//数字小于0
	{
		OLED_ShowChar(X, Y, '-', FontSize);	//显示-号
		Number = -Number;					//Number取反
	}

	/*提取整数部分和小数部分*/
	IntNum = Number;						//直接赋值给整型，提取整数
	Number -= IntNum;						//将Number的整数减掉，剩下小数部分
	PowNum = OLED_Pow(10, FraLength);		//根据指定小数位数，确定放大倍数
	FraNum = round(Number * PowNum);		//将小数部分同时四舍五入，避免显示误差
	IntNum += FraNum / PowNum;				//处理四舍五入造成的进位问题

	/*显示整数部分*/
	OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);

	/*显示小数点*/
	OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);

	/*显示小数部分*/
	OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}

/**
  * 函    数：OLED显示图像
  * 参    数：X 指定图像左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定图像左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Width 指定图像的宽度，范围0~128
  * 参    数：Height 指定图像的高度，范围0~64
  * 参    数：Image 指定要显示的图像
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
	uint8_t i = 0, j = 0;
	int16_t Page, Shift;

	/*将图像区域清空*/
	OLED_ClearArea(X, Y, Width, Height);

	/*遍历指定图像涉及的页*/
	/*(Height - 1) / 8 + 1的目的是Height / 8向上取整*/
	for (j = 0; j < (Height - 1) / 8 + 1; j ++)
	{
		/*遍历指定图像涉及的列*/
		for (i = 0; i < Width; i ++)
		{
			if (X + i >= 0 && X + i <= 127)		//在屏幕范围内进行显示
			{
				/*坐标在计算页地址和位移时需要加一个偏移*/
				Page = Y / 8;
				Shift = Y % 8;
				if (Y < 0)
				{
					Page -= 1;
					Shift += 8;
				}

				if (Page + j >= 0 && Page + j <= 7)		//在屏幕范围内进行显示
				{
					/*显示图像在当前页的内容*/
					OLED_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
				}

				if (Page + j + 1 >= 0 && Page + j + 1 <= 7)		//在屏幕范围内进行显示
				{
					/*显示图像在下一页的内容*/
					OLED_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
				}
			}
		}
	}
}

/**
  * 函    数：OLED使用printf函数打印格式化字符串，支持ASCII码（屏幕滚动）
  * 参    数：X 指定格式化字符串左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定格式化字符串左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 参    数：format 指定要显示的格式化字符串，范围为ASCII码可见字符组成的字符串
  * 参    数：... 格式化字符串参数列表
  * 返 回 值：无
  * 说    明：显示中文需要在OLED_Data.c中OLED_CF16x16数组定义
  *           未找到指定汉字时，显示默认图像，一个问号或者一个空格
  *           字体大小为OLED_8X16时，汉字显示16*16点阵显示
  *           字体大小为OLED_6X8时，汉字显示6*8点阵显示'?'
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
	char String[256];						//定义字符串数组
	va_list arg;							//定义可变参数列表数据类型的变量arg
	va_start(arg, format);					//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符串数组中
	va_end(arg);							//结束变量arg
	OLED_ShowString(X, Y, String, FontSize);//OLED显示字符串
}

/**
  * 函    数：OLED在指定位置画一个点
  * 参    数：X 指定位置的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定位置的纵坐标，范围-32768~32767，屏幕内0~63
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_DrawPoint(int16_t X, int16_t Y)
{
	if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)		//在屏幕范围内进行显示
	{
		/*将显存数组指定位置的一个Bit位置1*/
		OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
	}
}

/**
  * 函    数：OLED获取指定位置的点值
  * 参    数：X 指定位置的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定位置的纵坐标，范围-32768~32767，屏幕内0~63
  * 返 回 值：指定位置的点是否点亮状态，1点亮，0熄灭
  */
uint8_t OLED_GetPoint(int16_t X, int16_t Y)
{
	if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)		//在屏幕范围内进行读取
	{
		/*判断指定位置的点状态*/
		if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
		{
			return 1;	//状态为1，返回1
		}
	}

	return 0;		//否则，返回0
}

/**
  * 函    数：OLED画线
  * 参    数：X0 指定第一个点的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y0 指定第一个点的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：X1 指定第二个点的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y1 指定第二个点的纵坐标，范围-32768~32767，屏幕内0~63
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
	int16_t x, y, dx, dy, d, incrE, incrNE, temp;
	int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
	uint8_t yflag = 0, xyflag = 0;

	if (y0 == y1)		//横线
	{
		/*0点的X坐标大于1点的X坐标，交换两个X坐标*/
		if (x0 > x1) {temp = x0; x0 = x1; x1 = temp;}

		/*遍历X坐标*/
		for (x = x0; x <= x1; x ++)
		{
			OLED_DrawPoint(x, y0);	//画点
		}
	}
	else if (x0 == x1)	//竖线
	{
		/*0点的Y坐标大于1点的Y坐标，交换两个Y坐标*/
		if (y0 > y1) {temp = y0; y0 = y1; y1 = temp;}

		/*遍历Y坐标*/
		for (y = y0; y <= y1; y ++)
		{
			OLED_DrawPoint(x0, y);	//画点
		}
	}
	else				//斜线
	{
		/*使用Bresenham算法画直线，可以保证最快的计算速度，效率最高*/
		/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
		/*参考视频：https://www.bilibili.com/video/BV1364y1d7Lo*/

		if (x0 > x1)	//0点的X坐标大于1点的X坐标
		{
			/*交换两个坐标点*/
			/*交换不影响画线，只是将画线方向从左向右统一*/
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}

		if (y0 > y1)	//0点的Y坐标大于1点的Y坐标
		{
			/*将Y坐标取反*/
			/*取反不影响画线，只是将画线方向统一为第一象限*/
			y0 = -y0;
			y1 = -y1;

			/*使用标志位yflag存储当前互换状态，后续画点时还原*/
			yflag = 1;
		}

		if (y1 - y0 > x1 - x0)	//斜线斜率大于1
		{
			/*将X坐标和Y坐标互换*/
			/*互换不影响画线，只是将画线方向统一为第一象限0~45度范围*/
			temp = x0; x0 = y0; y0 = temp;
			temp = x1; x1 = y1; y1 = temp;

			/*使用标志位xyflag存储当前互换状态，后续画点时还原*/
			xyflag = 1;
		}

		/*此时已经统一为Bresenham算法画直线*/
		/*算法要求，画线方向统一为第一象限0~45度范围*/
		dx = x1 - x0;
		dy = y1 - y0;
		incrE = 2 * dy;
		incrNE = 2 * (dy - dx);
		d = 2 * dy - dx;
		x = x0;
		y = y0;

		/*画起始点，同时判断标志位，还原坐标*/
		if (yflag && xyflag){OLED_DrawPoint(y, -x);}
		else if (yflag)		{OLED_DrawPoint(x, -y);}
		else if (xyflag)	{OLED_DrawPoint(y, x);}
		else				{OLED_DrawPoint(x, y);}

		while (x < x1)		//遍历X坐标
		{
			x ++;
			if (d < 0)		//下一个点在当前点下方
			{
				d += incrE;
			}
			else			//下一个点在当前点上方
			{
				y ++;
				d += incrNE;
			}

			/*画每一个点，同时判断标志位，还原坐标*/
			if (yflag && xyflag){OLED_DrawPoint(y, -x);}
			else if (yflag)		{OLED_DrawPoint(x, -y);}
			else if (xyflag)	{OLED_DrawPoint(y, x);}
			else				{OLED_DrawPoint(x, y);}
		}
	}
}

/**
  * 函    数：OLED画矩形
  * 参    数：X 指定矩形左上角的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定矩形左上角的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Width 指定矩形的宽度，范围0~128
  * 参    数：Height 指定矩形的高度，范围0~64
  * 参    数：IsFilled 指定矩形是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
	int16_t i, j;
	if (!IsFilled)		//指定矩形为空心
	{
		/*遍历上侧X坐标，画上下两条边*/
		for (i = X; i < X + Width; i ++)
		{
			OLED_DrawPoint(i, Y);
			OLED_DrawPoint(i, Y + Height - 1);
		}
		/*遍历左侧Y坐标，画左右两条边*/
		for (i = Y; i < Y + Height; i ++)
		{
			OLED_DrawPoint(X, i);
			OLED_DrawPoint(X + Width - 1, i);
		}
	}
	else				//指定矩形为实心
	{
		/*遍历X坐标*/
		for (i = X; i < X + Width; i ++)
		{
			/*遍历Y坐标*/
			for (j = Y; j < Y + Height; j ++)
			{
				/*在指定区域画点，填充矩形*/
				OLED_DrawPoint(i, j);
			}
		}
	}
}

/**
  * 函    数：OLED画三角形
  * 参    数：X0 指定第一个点的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y0 指定第一个点的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：X1 指定第二个点的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y1 指定第二个点的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：X2 指定第三个点的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y2 指定第三个点的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：IsFilled 指定三角形是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled)
{
	int16_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
	int16_t i, j;
	int16_t vx[] = {X0, X1, X2};
	int16_t vy[] = {Y0, Y1, Y2};

	if (!IsFilled)			//指定三角形为空心
	{
		/*调用画线函数，依次画三条直线*/
		OLED_DrawLine(X0, Y0, X1, Y1);
		OLED_DrawLine(X0, Y0, X2, Y2);
		OLED_DrawLine(X1, Y1, X2, Y2);
	}
	else					//指定三角形为实心
	{
		/*找到三角形最小的X、Y坐标*/
		if (X1 < minx) {minx = X1;}
		if (X2 < minx) {minx = X2;}
		if (Y1 < miny) {miny = Y1;}
		if (Y2 < miny) {miny = Y2;}

		/*找到三角形最大的X、Y坐标*/
		if (X1 > maxx) {maxx = X1;}
		if (X2 > maxx) {maxx = X2;}
		if (Y1 > maxy) {maxy = Y1;}
		if (Y2 > maxy) {maxy = Y2;}

		/*在最小和最大坐标之间的区域，就是需要填充的区域*/
		/*遍历三角形内的所有点*/
		/*遍历X坐标*/
		for (i = minx; i <= maxx; i ++)
		{
			/*遍历Y坐标*/
			for (j = miny; j <= maxy; j ++)
			{
				/*调用OLED_pnpoly判断点是否在指定多边形之间*/
				/*在内部，则画点，不在，则不画*/
				if (OLED_pnpoly(3, vx, vy, i, j)) {OLED_DrawPoint(i, j);}
			}
		}
	}
}

/**
  * 函    数：OLED画圆
  * 参    数：X 指定圆圆心的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定圆圆心的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Radius 指定圆的半径，范围0~255
  * 参    数：IsFilled 指定圆是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled)
{
	int16_t x, y, d, j;

	/*使用Bresenham算法画圆，可以保证最快的计算速度，效率最高*/
	/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
	/*参考视频：https://www.bilibili.com/video/BV1VM4y1u7wJ*/

	d = 1 - Radius;
	x = 0;
	y = Radius;

	/*画出圆八分之一对称点*/
	OLED_DrawPoint(X + x, Y + y);
	OLED_DrawPoint(X - x, Y - y);
	OLED_DrawPoint(X + y, Y + x);
	OLED_DrawPoint(X - y, Y - x);

	if (IsFilled)		//指定圆为实心
	{
		/*遍历初始Y坐标*/
		for (j = -y; j < y; j ++)
		{
			/*在指定区域画点，填充圆*/
			OLED_DrawPoint(X, Y + j);
		}
	}

	while (x < y)		//遍历X坐标
	{
		x ++;
		if (d < 0)		//下一个点在当前点下方
		{
			d += 2 * x + 1;
		}
		else			//下一个点在当前点上方
		{
			y --;
			d += 2 * (x - y) + 1;
		}

		/*画出圆八分之一对称点*/
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X + y, Y + x);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - y, Y - x);
		OLED_DrawPoint(X + x, Y - y);
		OLED_DrawPoint(X + y, Y - x);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X - y, Y + x);

		if (IsFilled)	//指定圆为实心
		{
			/*中间填充*/
			for (j = -y; j < y; j ++)
			{
				/*在指定区域画点，填充圆*/
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}

			/*边缘填充*/
			for (j = -x; j < x; j ++)
			{
				/*在指定区域画点，填充圆*/
				OLED_DrawPoint(X - y, Y + j);
				OLED_DrawPoint(X + y, Y + j);
			}
		}
	}
}

/**
  * 函    数：OLED画椭圆
  * 参    数：X 指定椭圆圆心的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定椭圆圆心的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：A 指定椭圆的长轴长半轴长，范围0~255
  * 参    数：B 指定椭圆的短轴长半轴长，范围0~255
  * 参    数：IsFilled 指定椭圆是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
{
	int16_t x, y, j;
	int16_t a = A, b = B;
	float d1, d2;

	/*使用Bresenham算法画椭圆，可以保证分割和最快的计算速度，效率最高*/
	/*参考网址：https://blog.csdn.net/myf_666/article/details/128167392*/

	x = 0;
	y = b;
	d1 = b * b + a * a * (-b + 0.5);

	if (IsFilled)	//指定椭圆为实心
	{
		/*遍历初始Y坐标*/
		for (j = -y; j < y; j ++)
		{
			/*在指定区域画点，填充椭圆*/
			OLED_DrawPoint(X, Y + j);
			OLED_DrawPoint(X, Y + j);
		}
	}

	/*画椭圆初始点*/
	OLED_DrawPoint(X + x, Y + y);
	OLED_DrawPoint(X - x, Y - y);
	OLED_DrawPoint(X - x, Y + y);
	OLED_DrawPoint(X + x, Y - y);

	/*画椭圆中间部分*/
	while (b * b * (x + 1) < a * a * (y - 0.5))
	{
		if (d1 <= 0)		//下一个点在当前点下方
		{
			d1 += b * b * (2 * x + 3);
		}
		else				//下一个点在当前点上方
		{
			d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
			y --;
		}
		x ++;

		if (IsFilled)	//指定椭圆为实心
		{
			/*中间填充*/
			for (j = -y; j < y; j ++)
			{
				/*在指定区域画点，填充椭圆*/
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}
		}

		/*画椭圆中间部分圆弧*/
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X + x, Y - y);
	}

	/*画椭圆边缘部分*/
	d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;

	while (y > 0)
	{
		if (d2 <= 0)		//下一个点在当前点下方
		{
			d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
			x ++;

		}
		else				//下一个点在当前点上方
		{
			d2 += a * a * (-2 * y + 3);
		}
		y --;

		if (IsFilled)	//指定椭圆为实心
		{
			/*边缘填充*/
			for (j = -y; j < y; j ++)
			{
				/*在指定区域画点，填充椭圆*/
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}
		}

		/*画椭圆边缘部分圆弧*/
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X + x, Y - y);
	}
}

/**
  * 函    数：OLED画圆弧
  * 参    数：X 指定圆弧圆心的横坐标，范围-32768~32767，屏幕内0~127
  * 参    数：Y 指定圆弧圆心的纵坐标，范围-32768~32767，屏幕内0~63
  * 参    数：Radius 指定圆弧的半径，范围0~255
  * 参    数：StartAngle 指定圆弧的起始角度，范围-180~180
  *           水平向右为0度，水平向左为180度或-180度，向下为正，向上为负，顺时针旋转
  * 参    数：EndAngle 指定圆弧的终止角度，范围-180~180
  *           水平向右为0度，水平向左为180度或-180度，向下为正，向上为负，顺时针旋转
  * 参    数：IsFilled 指定圆弧是否填充，填充即为扇形
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，必须要调用OLED_Update函数才能真正显示
  */
void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
	int16_t x, y, d, j;

	/*此函数使用Bresenham算法画圆的方式*/

	d = 1 - Radius;
	x = 0;
	y = Radius;

	/*在画圆弧的每一个点时，判断点是否在指定角度内，在，则画点，不在，则不画*/
	if (OLED_IsInAngle(x, y, StartAngle, EndAngle))	{OLED_DrawPoint(X + x, Y + y);}
	if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
	if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
	if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}

	if (IsFilled)	//指定圆弧为扇形
	{
		/*遍历初始Y坐标*/
		for (j = -y; j < y; j ++)
		{
			/*在画圆弧的每一个点时，判断点是否在指定角度内，在，则画点，不在，则不画*/
			if (OLED_IsInAngle(0, j, StartAngle, EndAngle)) {OLED_DrawPoint(X, Y + j);}
		}
	}

	while (x < y)		//遍历X坐标
	{
		x ++;
		if (d < 0)		//下一个点在当前点下方
		{
			d += 2 * x + 1;
		}
		else			//下一个点在当前点上方
		{
			y --;
			d += 2 * (x - y) + 1;
		}

		/*在画圆弧的每一个点时，判断点是否在指定角度内，在，则画点，不在，则不画*/
		if (OLED_IsInAngle(x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + y);}
		if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
		if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
		if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
		if (OLED_IsInAngle(x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y - y);}
		if (OLED_IsInAngle(y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y - x);}
		if (OLED_IsInAngle(-x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + y);}
		if (OLED_IsInAngle(-y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + x);}

		if (IsFilled)	//指定圆弧为扇形
		{
			/*中间填充*/
			for (j = -y; j < y; j ++)
			{
				/*在画圆弧的每一个点时，判断点是否在指定角度内，在，则画点，不在，则不画*/
				if (OLED_IsInAngle(x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + j);}
				if (OLED_IsInAngle(-x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + j);}
			}

			/*边缘填充*/
			for (j = -x; j < x; j ++)
			{
				/*在画圆弧的每一个点时，判断点是否在指定角度内，在，则画点，不在，则不画*/
				if (OLED_IsInAngle(-y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + j);}
				if (OLED_IsInAngle(y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + j);}
			}
		}
	}
}

/*********************显示函数*/


/*****************协议栈|版权所有****************/
/*****************jiangxiekeji.com*****************/
