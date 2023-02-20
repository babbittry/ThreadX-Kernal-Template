/*
*********************************************************************************************************
*
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.c
*	版    本 : V1.0
*	说    明 : 驱动LED指示灯
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-09-05 armfly  正式发布
*
*	Copyright (C), 2015-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
    STM32-H7 开发板的LED指示灯是由74HC574驱动的，不是用CPU的IO直接驱动。
    74HC574是一个8路并口缓冲器，挂在FMC总线上。
    74HC574的驱动程序为 : bsp_fmc_io.c
*/

/*
*********************************************************************************************************
*	函 数 名: bsp_InitLed
*	功能说明: 配置LED指示灯相关的GPIO,  该函数被 bsp_Init() 调用。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef  GPIO_InitStruct;

    /*开启LED相关的GPIO外设时钟*/
    LED1_GPIO_CLK_ENABLE();
    LED2_GPIO_CLK_ENABLE();
    LED3_GPIO_CLK_ENABLE();

    /*选择要控制的GPIO引脚*/
    GPIO_InitStruct.Pin = LED1_PIN;

    /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;

    /*设置引脚为上拉模式*/
    GPIO_InitStruct.Pull  = GPIO_PULLUP;

    /*设置引脚速率为高速 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
    HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);

    /*选择要控制的GPIO引脚*/
    GPIO_InitStruct.Pin = LED2_PIN;
    HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);

    /*选择要控制的GPIO引脚*/
    GPIO_InitStruct.Pin = LED3_PIN;
    HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);

    /*关闭RGB灯*/
    LED_RGBOFF;
}


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
