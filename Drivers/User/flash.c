#include "flash.h"
#include "main.h"

/**
  * flash存储地址 程序存储从0x08000000开始 C8T6有64K FLASH 扇区大小为1K 为了不覆盖程序 使用FLASH从最后开始 0x08000000 + 63 * 1024
  */
#define FLASH_SAVE_ADDR 0x0800FC00

/**
  * Flash数据存储数量
  */
#define FLASH_DATA_STORAGE_COUNT 10

/**
  * Flash运行时存储缓冲
  */
unsigned int flashRuntimeBuffer[FLASH_DATA_STORAGE_COUNT] = {0,};

/**
  * @brief 存储数据(地址第一个位置为是否写入过数据检测)(私有)
  * @retval void
  */
void Private_Flash_SaveValue(void)
{ 
    HAL_FLASH_Unlock();   							//解锁FLASH
 
	FLASH_EraseInitTypeDef f;						//初始化FLASH_EraseInitTypeDef
	f.TypeErase = FLASH_TYPEERASE_PAGES; // 标明Flash执行页面只做擦除操作
	f.PageAddress = FLASH_SAVE_ADDR;          //声明要擦除的地址
	f.NbPages = 1;	                    //说明要擦除的页数，此参数必须是Min_Data = 1和Max_Data =(最大页数-初始页的值)之间的值
																			//一些MCU一页是1KB，一些MCU一页是2KB  容量小于256K的 F103, 扇区大小为1K字节  容量大于等于于256K的 F103, 扇区大小为2K字节
	uint32_t PageError = 0;  						//设置PageError,如果出现错误这个变量会被设置为出错的FLASH地址	
	HAL_FLASHEx_Erase(&f, &PageError);  //调用擦除函数

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_SAVE_ADDR, 0);  // 对FLASH烧写 保存状态值
    
    // 循环写入
    for(unsigned char i = 0; i < FLASH_DATA_STORAGE_COUNT; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_SAVE_ADDR + (i + 1) * 4, flashRuntimeBuffer[i]); // 对FLASH烧写
    }
    HAL_FLASH_Lock(); 									//锁住FLASH
}

/**
 * @brief 从指定地址读取一个字 (32位数据)
 * @param address 读取地址 (此地址必须为2的倍数!!)
 * @retval uint32_t 读取到的数据 (32位)
 */
unsigned int Private_Flash_ReadOneValue(unsigned int address)
{
    return *(volatile unsigned int*)(address); // *(__IO uint16_t *)是读取该地址的参数值,其值为16位数据,一次读取两个字节，*(__IO uint32_t *)就一次读4个字节
}

/**
  * @brief 读取数据(私有)
  * @retval unsigned char 0读取成功 1未写入过
  */
unsigned char Private_Flash_ReadValue(void)
{
    uint32_t saveStateValue = Private_Flash_ReadOneValue(FLASH_SAVE_ADDR); // 取状态值
    if(saveStateValue == 0) // 有写入
    {
        // 循环读取
        for(unsigned char i = 0; i < FLASH_DATA_STORAGE_COUNT; i++)
        {
            flashRuntimeBuffer[i] = Private_Flash_ReadOneValue(FLASH_SAVE_ADDR + (i + 1) * 4);
        }
        return 0;
    }
    else // 未写入过
    {
        return 1;
    }
}

/**
  * @brief 初始化
  * @retval unsigned char 0读取成功 1未写入过
  */
unsigned char Flash_Init(void)
{
    return Private_Flash_ReadValue();
}

/**
  * @brief 存储数据(32位数据)(地址第一个位置为是否写入过数据检测)
  * @param  num 存储第几个数据 0 - FLASH_DATA_STORAGE_COUNT
  * @param  value 值
  * @retval void
  */
void Flash_SaveValue(unsigned char num, unsigned int value)
{
    // 更新值
    flashRuntimeBuffer[num] = value;
    // 存储
    Private_Flash_SaveValue();
}

/**
  * @brief 读取一个数据 (32位数据)
  * @param  num 存储第几个数据 0 - FLASH_DATA_STORAGE_COUNT
  * @retval unsigned int (32位数据)
  */
unsigned int Flash_ReadValue(unsigned char num)
{
    return flashRuntimeBuffer[num];
}

