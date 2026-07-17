#include "MyFLASH.h"

#include <stdint.h>

#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"

/**
  * 函    数：FLASH读取一个32位的字
  * 参    数：Address 要读取数据的字地址
  * 返 回 值：指定地址下的数据
  */
uint32_t MyFLASH_ReadWord(uint32_t Address)
{
	return *((__IO uint32_t *)(Address));	//使用指针访问指定地址下的数据并返回
}

/**
  * 函    数：FLASH读取一个16位的半字
  * 参    数：Address 要读取数据的半字地址
  * 返 回 值：指定地址下的数据
  */
uint16_t MyFLASH_ReadHalfWord(uint32_t Address)
{
	return *((__IO uint16_t *)(Address));	//使用指针访问指定地址下的数据并返回
}

/**
  * 函    数：FLASH读取一个8位的字节
  * 参    数：Address 要读取数据的字节地址
  * 返 回 值：指定地址下的数据
  */
uint8_t MyFLASH_ReadByte(uint32_t Address)
{
	return *((__IO uint8_t *)(Address));	//使用指针访问指定地址下的数据并返回
}

/**
  * 函    数：FLASH全擦除
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，FLASH的所有页都会被擦除，包括程序文件本身，擦除后，程序将不复存在
  */
void MyFLASH_EraseAllPages(void)
{
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;		//定义FLASH擦除初始化结构体

	HAL_FLASH_Unlock();							//解锁

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_MASSERASE;	//擦除类型：全擦除
	HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);		//执行全擦除

	HAL_FLASH_Lock();							//加锁
}

/**
  * 函    数：FLASH页擦除
  * 参    数：PageAddress 要擦除页的页地址
  * 返 回 值：无
  */
void MyFLASH_ErasePage(uint32_t PageAddress)
{
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;		//定义FLASH擦除初始化结构体

	HAL_FLASH_Unlock();							//解锁

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;		//擦除类型：页擦除
	EraseInitStruct.PageAddress = PageAddress;				//指定要擦除的页地址
	EraseInitStruct.NbPages = 1;							//擦除页数：1页
	HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);		//执行页擦除

	HAL_FLASH_Lock();							//加锁
}

/**
  * 函    数：FLASH编程字
  * 参    数：Address 要写入数据的字地址
  * 参    数：Data 要写入的32位数据
  * 返 回 值：无
  */
void MyFLASH_ProgramWord(uint32_t Address, uint32_t Data)
{
	HAL_FLASH_Unlock();							//解锁
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, Data);	//编程字（32位）
	HAL_FLASH_Lock();							//加锁
}

/**
  * 函    数：FLASH编程半字
  * 参    数：Address 要写入数据的半字地址
  * 参    数：Data 要写入的16位数据
  * 返 回 值：无
  */
void MyFLASH_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
	HAL_FLASH_Unlock();							//解锁
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address, Data);	//编程半字（16位）
	HAL_FLASH_Lock();							//加锁
}
