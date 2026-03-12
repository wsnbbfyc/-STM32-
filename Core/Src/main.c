/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <stdlib.h>
#include "delay.h"
#include "beep.h"
#include "ds18b20.h"
#include "gpio_output.h"
#include "lcd1602.h"
#include "key.h"
#include "flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// 显示缓冲区
char showBuffer[50] = {0, };

unsigned short adGzValue = 0; // 光照值
unsigned short adSdValue = 0; // 湿度值
unsigned short newAdSdValue = 0; // 新湿度值
unsigned short temperture = 0; // 温度
int adSdValueArray[3] = {0};

unsigned short setTemperture = 273; // 设置温度
unsigned short setHumidity = 70; // 设置湿度度上限
unsigned short setLighting = 130; // 设置光照

int controlMode = 0; // 控制模式
unsigned char dgState = 0; // 灯光状态
unsigned char sbState = 0; // 水泵状态
unsigned char fsState = 0; // 风扇状态

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  GPIO 打开所有时钟
  * @retval void
  */
void Main_GPIO_OpenAll_RCC_CLK()
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/**
  * @brief  GPIO 初始化
  * @retval void
  */
void Main_GPIO_Init()
{
    Main_GPIO_OpenAll_RCC_CLK();
}

// 定义map函数
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    // 检查输入范围是否有效
    if(in_min == in_max)
    {
        return -1;  // 或者返回其他错误码
    }

    // 计算映射后的值
    long mapped = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    // 返回结果
    return mapped;
}

/**
  * @brief  获取AD数据
  * @retval void
  */
void GetAdValue(void)
{
    HAL_ADC_Start(&hadc1);
    // 通道8 湿度
    HAL_ADC_PollForConversion(&hadc1, 50);

    if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
    {
        int AD_Value = HAL_ADC_GetValue(&hadc1);
        newAdSdValue = map(AD_Value, 0, 4095, 0, 255);
    }

    HAL_ADC_Start(&hadc1);
    // 通道9 光照强度
    HAL_ADC_PollForConversion(&hadc1, 50);

    if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
    {
        int AD_Value = HAL_ADC_GetValue(&hadc1);
        adGzValue = map(AD_Value, 0, 4095, 0, 255);
    }
}

/**
  * @brief  关闭所有
  * @retval void
  */
void Close(void)
{
    Beep_Control(0); // 关闭报警
    GpioOutput_NumbetrControl(0, 0); // 关闭光照
    GpioOutput_NumbetrControl(2, 0); // 关闭水泵
    GpioOutput_NumbetrControl(1, 0); // 关闭风扇
    dgState = 0; // 灯光关
    sbState = 0; // 水泵关
    fsState = 0; // 风扇关
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */
    unsigned char sbWarnFlag = 0, wdWarnFlag = 0; // 警告标志
    unsigned char timerValue = 254;
    unsigned char setFlag = 0; // 设置标志
    unsigned char setIndex = 0; // 设置位置
    unsigned char showFlag = 1; // 显示标志 
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */
    __HAL_RCC_AFIO_CLK_ENABLE();        //端口重定义功能时钟
    __HAL_AFIO_REMAP_SWJ_NOJTAG();     //PB3用到了默认为JTAG-JTDO,用作PB3,PB3,PA15时须重定义，我通常使用ST-Link调试，因此SWJ需保留，只禁用JTAG即可。

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM4_Init();
    MX_USART1_UART_Init();
    MX_ADC1_Init();
    MX_USART3_UART_Init();
    /* USER CODE BEGIN 2 */
    Main_GPIO_Init();
    lcd1602_init();
    if(ds18b20_init())
    {
        lcd1602_show_string(0, 0, (unsigned char*)"ERR ");
        while(1);
    }
    Key_Init();
    Beep_Init();
    GpioOutput_Init();
    if(Flash_Init() == 0) // 存储过
    {
        setHumidity = Flash_ReadValue(0);    
        setTemperture = Flash_ReadValue(1);
        setLighting = Flash_ReadValue(2);
        controlMode = Flash_ReadValue(3);
    }
    else
    {
        Flash_SaveValue(0, setHumidity);
        Flash_SaveValue(1, setTemperture);
        Flash_SaveValue(2, setLighting);
        Flash_SaveValue(3, controlMode);
    }
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while(1)
    {
        if(setFlag) // 设置
        {
            unsigned char key = Key_Scan(1);
            int addNum = 0; // 记录增加还是减小
            if(key == 2) // 加按键
            {
                addNum = 1; // 增加
            }
            else if(key == 3) // 减按键
            {
                addNum = -1; // 减小
            }
            else if(key == 1) // 设置键
            {
                if(++setIndex > 2)
                {
                    setFlag = 0; // 设置标志为假
                    timerValue = 254;
                }
                while(Key_Scan(1)); // 等待所有按钮松开
            }
            else if(key == 4) // 退出按键
            {
                setFlag = 0; // 设置标志为假
                timerValue = 254;
                while(Key_Scan(1)); // 等待所有按钮松开
            }
            
            if(setIndex == 0) // 设置湿度
            {
                setHumidity += addNum;
                if(setHumidity > 99)
                {
                    setHumidity = 99;
                }
                else if(setHumidity < 1)
                {
                    setHumidity = 1;
                } 
                lcd1602_show_string(0, 0, (unsigned char*)"  Set Humidity  ");
                sprintf(showBuffer, "      %02d%%       ", setHumidity);
                if(addNum != 0)
                {
                    Flash_SaveValue(0, setHumidity);;
                } 
            }
            else if(setIndex == 1)  // 设置温度
            {
                setTemperture += addNum;
                if(setTemperture > 999)
                {
                    setTemperture = 999;
                }
                else if(setTemperture < 1)
                {
                    setTemperture = 1;
                } 
                lcd1602_show_string(0, 0, (unsigned char*)" Set Temperture ");
                sprintf(showBuffer, "      %04.1f      ", setTemperture / 10.0);
                if(addNum != 0)
                {
                    Flash_SaveValue(1, setTemperture);
                }
            }
            else // 设置光照
            {
                setLighting += addNum;
                if(setLighting > 255)
                {
                    setLighting = 255;
                }
                else if(setLighting < 1)
                {
                    setLighting = 1;
                } 
                lcd1602_show_string(0, 0, (unsigned char*)"  Set Lighting  ");
                sprintf(showBuffer, "      %03d       ", setLighting);
                if(addNum != 0)
                {
                    Flash_SaveValue(2, setLighting);
                }
            }
            lcd1602_show_string(0, 1, (unsigned char*)showBuffer);
        }
        else // 运行
        {
            unsigned char key = Key_Scan(0);
            
            if(controlMode == 1 && key == 1) // 自动模式且设置按钮
            {
                setFlag = 1; // 设置标志为真
                setIndex = 0;
                Close(); 
                sbWarnFlag = wdWarnFlag = 0; // 警告标志为假   
                while(Key_Scan(1)); // 等待所有按钮松开 
                continue; // 是设置就不向下运行
            }
            else if(key == 4) // 模式切换
            {
                Close();
                controlMode = !controlMode; // 取反模式
                Flash_SaveValue(3, controlMode);
                timerValue = 254;
            }  
               
            if(++timerValue > 40) // 显示
            {
                timerValue = 0;
                if(sbWarnFlag || wdWarnFlag) // 处于警告 取反显示
                {
                    showFlag = !showFlag; 
                }
                GetAdValue(); // 读取湿度值 
                adSdValueArray[0] = adSdValueArray[1];
                adSdValueArray[1] = adSdValueArray[2];
                adSdValueArray[2] = newAdSdValue;
                adSdValueArray[2] = (255 - adSdValueArray[2]) / 200.0 * 100;
                if(adSdValueArray[2] > 99)
                {
                    adSdValueArray[2] = 99;
                }
               
                if(abs(adSdValueArray[0] - adSdValueArray[1]) <= 5 && abs(adSdValueArray[1] - adSdValueArray[2]) <= 5)
                {
                    adSdValue = adSdValueArray[2];
                }
                
                temperture = ds18b20_get_temperature(); // 读取风扇值
                sprintf(showBuffer, "ZD SD:%02d%% G G G ", (int)adSdValue);
                if(controlMode == 0) // 手动模式
                {
                    showBuffer[0] = 'S';
                }
                if(controlMode && sbWarnFlag && showFlag == 0) // 不显示
                {
                    showBuffer[6] = ' ';
                    showBuffer[7] = ' ';
                    showBuffer[8] = ' ';
                }
                if(dgState) // 
                {
                    showBuffer[10] = 'K';
                }
                if(fsState) // 
                {
                    showBuffer[12] = 'K';
                }
                if(sbState) // 
                {
                    showBuffer[14] = 'K';
                }
                
                lcd1602_show_string(0, 0, (unsigned char*)showBuffer);
                
                sprintf(showBuffer, "WD:%04.1f\001C GZ:%03d", temperture / 10.0, adGzValue);
                if(controlMode && wdWarnFlag && showFlag == 0) // 不显示 
                {
                    showBuffer[3] = ' ';
                    showBuffer[4] = ' ';
                    showBuffer[5] = ' ';
                    showBuffer[6] = ' ';
                    showBuffer[7] = ' ';
                    showBuffer[8] = ' ';
                }
                lcd1602_show_string(0, 1, (unsigned char*)showBuffer);
                
                
            }
            
            if(controlMode == 0) // 手动模式
            {
                sbWarnFlag = wdWarnFlag  = 0; // 警告标志为假   
                if(key == 1)
                {
                    dgState = !dgState;
                    timerValue = 254;
                }
                else if(key == 3) // 
                {
                    sbState = !sbState;
                    timerValue = 254;
                }
                else if(key == 2) // 
                {
                    fsState = !fsState;
                    timerValue = 254;
                }
            }
            else // 自动模式
            {
                // 检测光照
                if(adGzValue < setLighting) // 光线暗度大于设置
                {
                    dgState = 1; // 灯光开
                }
                else
                {
                    dgState = 0; // 灯光关
                } 
                
                // 检测风扇
                if(temperture > setTemperture) // 温度大于设置
                {
                    fsState = 1; // 风扇开     
                    wdWarnFlag = 1; // 警告标志为真
                }
                else
                {
                    fsState = 0; // 风扇关         
                    wdWarnFlag = 0; // 警告标志为假
                } 
                
                // 检测湿度
                if(adSdValue < setHumidity) // 湿度小于设置湿度 开启水泵
                {
                    sbState = 1; // 水泵开
                    sbWarnFlag = 1; // 警告标志为真
                }
                else // 湿度大于设置湿度 关闭水泵
                {
                    sbState = 0; // 水泵关
                    sbWarnFlag = 0; // 警告标志为假
                }
            }
        }
        
        if(dgState) // 灯光状态
        {
            GpioOutput_NumbetrControl(0, 1); // 开启灯
        }
        else
        {
            GpioOutput_NumbetrControl(0, 0); // 关闭灯
        }
        if(fsState) // 风扇状态
        {
            GpioOutput_NumbetrControl(1, 1); // 开启风扇
        }
        else
        {
            GpioOutput_NumbetrControl(1, 0); // 关闭风扇
        }
        
        if(sbState) // 水泵状态
        {
            GpioOutput_NumbetrControl(2, 1); // 开启水泵
        }
        else
        {
            GpioOutput_NumbetrControl(2, 0); // 关闭水泵
        }
        
        // 警告判断
        if(sbWarnFlag || wdWarnFlag)
        {
            Beep_GapControl();
        }
        else
        {
            Beep_Control(0);
        }
        Delay_Ms(10);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }

    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;

    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();

    while(1)
    {
    }

    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
