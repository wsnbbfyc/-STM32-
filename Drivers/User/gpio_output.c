#include "gpio_output.h"
#include "main.h"
#include "delay.h"

// 间隙时间
#define GAP_TIME 4

/* 引脚 定义 */
#define GpioOutput1_GPIO_Port GPIOB // 光照
#define GpioOutput1_Pin GPIO_PIN_4
#define GpioOutput2_GPIO_Port GPIOB // 风扇
#define GpioOutput2_Pin GPIO_PIN_3
#define GpioOutput3_GPIO_Port GPIOA // 水泵
#define GpioOutput3_Pin GPIO_PIN_15

//
unsigned char GpioOutputState[1] = {0, };

/**
  * @brief  GpioOutput初始化
  * @retval void
  */
void GpioOutput_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    HAL_GPIO_WritePin(GpioOutput1_GPIO_Port, GpioOutput1_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GpioOutput1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // 无上拉或下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
    HAL_GPIO_Init(GpioOutput1_GPIO_Port, &GPIO_InitStruct); // 初始化引脚
    
    HAL_GPIO_WritePin(GpioOutput2_GPIO_Port, GpioOutput2_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GpioOutput2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // 无上拉或下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
    HAL_GPIO_Init(GpioOutput2_GPIO_Port, &GPIO_InitStruct); // 初始化引脚
    
    HAL_GPIO_WritePin(GpioOutput3_GPIO_Port, GpioOutput3_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GpioOutput3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // 无上拉或下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
    HAL_GPIO_Init(GpioOutput3_GPIO_Port, &GPIO_InitStruct); // 初始化引脚
}

/**
  * @brief  GpioOutput 控制 按二进制控制 如 0x0001 就是第一个GPIO 控制 0x0003 就是第一个和第二个灯一起控制
  * @param  gpioOutputBit 位控制 最多16位
  * @param  gpioOutputState 状态
  * @retval void
  */
void GpioOutput_Control(unsigned short gpioOutputBit, unsigned char gpioOutputState)
{
    // 控制GpioOutput1
    if(gpioOutputBit & 0x0001) 
    {
        if(gpioOutputState)
        {
            HAL_GPIO_WritePin(GpioOutput1_GPIO_Port, GpioOutput1_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(GpioOutput1_GPIO_Port, GpioOutput1_Pin, GPIO_PIN_RESET);
        }  
    }
    
    // 控制GpioOutput2
    if(gpioOutputBit & 0x0002) 
    {
        if(gpioOutputState)
        {
            HAL_GPIO_WritePin(GpioOutput2_GPIO_Port, GpioOutput2_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(GpioOutput2_GPIO_Port, GpioOutput2_Pin, GPIO_PIN_RESET);
        }  
    }
    
    // 控制GpioOutput3
    if(gpioOutputBit & 0x0004) 
    {
        if(gpioOutputState)
        {
            HAL_GPIO_WritePin(GpioOutput3_GPIO_Port, GpioOutput3_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(GpioOutput3_GPIO_Port, GpioOutput3_Pin, GPIO_PIN_RESET);
        }  
    }
}

/**
  * @brief  GpioOutput 切换指定的引脚
  * @param  num 第几位Gpio态
  * @retval void
  */
void GpioOutput_ToggleControl(unsigned short num)
{
    GpioOutputState[num] = !GpioOutputState[num];
    GpioOutput_NumbetrControl(num, GpioOutputState[num]);
}

/**
  * @brief  GpioOutput 控制 数字控制 
  * @param  num 第几位Gpio
  * @param  gpioOutputState 状态
  * @retval void
  */
void GpioOutput_NumbetrControl(unsigned short num, unsigned char gpioOutputState)
{
    GpioOutputState[num] = gpioOutputState;
    GpioOutput_Control(0x0001 << num, gpioOutputState);
}

/**
  * @brief  GpioOutput间隙控制
  * @retval void
  */
void GpioOutput_GapControl(unsigned short gpioOutputBit)
{
    static unsigned int gapTimer = 0; // 间隙计数
    
    if(gapTimer == 1) // 开始 
    {
        GpioOutput_Control(gpioOutputBit, 0);
    }
    else if(gapTimer == GAP_TIME) // 达到间隙翻转
    {
        GpioOutput_Control(gpioOutputBit, 1);
    }
    else if(gapTimer == 2 * GAP_TIME) // 翻转间隙结束
    {
        gapTimer = 0;
    }
    gapTimer++; // 值增加1
}

