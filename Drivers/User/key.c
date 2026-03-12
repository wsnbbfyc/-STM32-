#include "key.h"
#include "main.h"
#include "delay.h"

/* 引脚 定义 */
#define KEY1_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_12
#define KEY2_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_13
#define KEY3_GPIO_Port GPIOB
#define KEY3_Pin GPIO_PIN_14
#define KEY4_GPIO_Port GPIOB
#define KEY4_Pin GPIO_PIN_15

// 按钮位定义
#define KEY1 HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin)
#define KEY2 HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)
#define KEY3 HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin)
#define KEY4 HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin)

/**
  * @brief  按钮初始化
  * @retval void
  */
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = KEY1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // 输入模式
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 上拉
    HAL_GPIO_Init(KEY1_GPIO_Port, &GPIO_InitStruct); // 初始化引脚
    
    GPIO_InitStruct.Pin = KEY2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // 输入模式
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 上拉
    HAL_GPIO_Init(KEY2_GPIO_Port, &GPIO_InitStruct); // 初始化引脚
    
    GPIO_InitStruct.Pin = KEY3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // 输入模式
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 上拉
    HAL_GPIO_Init(KEY3_GPIO_Port, &GPIO_InitStruct); // 初始化引脚
    
    GPIO_InitStruct.Pin = KEY4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // 输入模式
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 上拉
    HAL_GPIO_Init(KEY4_GPIO_Port, &GPIO_InitStruct); // 初始化引脚
}

/**
  * @brief  按键扫描
  * @param  mode:0 / 1  
  * @retval unsigned char 返回按下的按钮 0为未按下
  */
unsigned char Key_Scan(unsigned char mode)
{
    static unsigned char keyUpFlag = 1; // 按钮松开标志
    
    if (mode) // 支持连按
    {
        keyUpFlag = 1;
    } 
    
	if(keyUpFlag == 1 && (KEY1 == 0 || KEY2 == 0 || KEY3 == 0 || KEY4 == 0))//任意按键按下
	{
		Delay_Ms(10);// 消抖
        
		keyUpFlag = 0; // 按钮按下
        
        // 判断按下按钮
		if(KEY1 == 0)
        {
			return 1;
        }
		else if(KEY2 == 0)
        {
			return 2;
        }
		else if(KEY3 == 0)
        {
			return 3;
        }
		else if(KEY4 == 0)
        {
			return 4;	
        }
	}
	else if(KEY1 == 1 && KEY2 == 1 && KEY3 == 1 && KEY4 == 1)	//无按键按下
	{
		keyUpFlag = 1; // 按钮松开		
	}
	return 0;
}

