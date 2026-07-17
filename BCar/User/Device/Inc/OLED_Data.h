#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include <stdint.h>

/*字符集选择*/
/*同一时间仅能选择其中一种，注释另一种*/
#define OLED_CHARSET_UTF8			//选择字符集为UTF8
//#define OLED_CHARSET_GB2312		//选择字符集为GB2312

/*字模结构体*/
typedef struct
{

#ifdef OLED_CHARSET_UTF8			//选择字符集为UTF8
	char Index[5];					//汉字索引占用为5字节
#endif

#ifdef OLED_CHARSET_GB2312			//选择字符集为GB2312
	char Index[3];					//汉字索引占用为3字节
#endif

	uint8_t Data[32];				//字模数据
} ChineseCell_t;

/*ASCII字模数据数组*/
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];

/*汉字字模数据数组*/
/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH			3		//UTF-8编码格式给3，

/*字模基本单元*/
typedef struct
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[32];						//字模数据
} ChineseCell_t_16X16;
typedef struct
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[24];						//字模数据
} ChineseCell_t_12X12;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];
extern const uint8_t OLED_F12x24[][36];
/*汉字字模数据声明*/
extern const ChineseCell_t_16X16 OLED_CF16x16[];
extern const ChineseCell_t_12X12 OLED_CF12x12[];

/*图像数据数组*/
extern const uint8_t Diode[];
extern const uint8_t	Guo[];
extern const uint8_t	monkey[];
/*按照同样的格式，在对应位置添加你的图像数据数组*/
//...

#endif


/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/