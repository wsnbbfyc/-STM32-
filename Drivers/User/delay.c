#include "delay.h"
#include "main.h"

/**
  * @brief  软件延时微秒 适用于72MHz
  * @param  us 延时时间
  * @retval void
  */
void Delay_Us(unsigned int us)
{
    unsigned int delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
    while (delay--)
	{
		;
	}
}

/**
  * @brief  软件延时微秒
  * @param  us 延时时间
  * @retval void
  */
void Delay_Us_SysTick(unsigned int us)
{
    unsigned int ticks;
    unsigned int told, tnow, tcnt = 0;
    unsigned int reload = SysTick->LOAD; // SysTick装置值
    ticks = us * 72; // 需要的节拍数
    
    told = SysTick->VAL; // 刚进入时的计数器值
    while (1)
    {
        tnow = SysTick->VAL; // 读取滴答定时器计数值
        if (tnow != told) // 不相等
        {
            // 这里注意一下SYSTICK是一个递减的计数器就可以了 装置值 -> 0
            if (tnow < told) // 正常减
            {
                tcnt += told - tnow;
            }
            else // 如果大就相当于减完装置值一轮了
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) 
            {
                break; // 时间超过/等于要延迟的时间,则退出
            }
        }
    }
}

/**
  * @brief  软件延时毫秒
  * @param  ms 延时时间
  * @retval void
  */
void Delay_Ms(unsigned int ms)
{
    HAL_Delay(ms);
}

