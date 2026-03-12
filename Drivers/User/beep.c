#include "beep.h"
#include "main.h"

// 间隙时间
#define GAP_TIME 40

/*引脚 定义 */
#define BEEP_GPIO_Port GPIOA
#define BEEP_Pin GPIO_PIN_8

// 蜂鸣器位定义
#define BEEP_DQ_OUT(x) do{ x ? \
                                HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET) : \
                                HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET); \
                            }while(0)   

                            
/**
  * @brief  蜂鸣器初始化
  * @retval void
  */
void Beep_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = BEEP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // 无上拉或下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
    HAL_GPIO_Init(BEEP_GPIO_Port, &GPIO_InitStruct); // 初始化引脚
    
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
}


/**
  * @brief  蜂鸣器控制
  * @param  sw 开关
  * @retval void
  */
void Beep_Control(unsigned char sw)
{
    BEEP_DQ_OUT(sw);
}

/**
  * @brief  蜂鸣器间隙控制
  * @retval void
  */
void Beep_GapControl(void)
{
    static unsigned int gapTimer = 0; // 间隙计数
    
    if(gapTimer == 1) // 开始 
    {
        BEEP_DQ_OUT(0);
    }
    else if(gapTimer == GAP_TIME) // 达到间隙翻转
    {
        BEEP_DQ_OUT(1);
    }
    else if(gapTimer == 2 * GAP_TIME) // 翻转间隙结束
    {
        gapTimer = 0;
    }
    gapTimer++; // 值增加1
}

