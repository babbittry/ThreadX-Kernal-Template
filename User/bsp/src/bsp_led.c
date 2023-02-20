/*
*********************************************************************************************************
*
*	ģ������ : LEDָʾ������ģ��
*	�ļ����� : bsp_led.c
*	��    �� : V1.0
*	˵    �� : ����LEDָʾ��
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2018-09-05 armfly  ��ʽ����
*
*	Copyright (C), 2015-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
    STM32-H7 �������LEDָʾ������74HC574�����ģ�������CPU��IOֱ��������
    74HC574��һ��8·���ڻ�����������FMC�����ϡ�
    74HC574����������Ϊ : bsp_fmc_io.c
*/

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitLed
*	����˵��: ����LEDָʾ����ص�GPIO,  �ú����� bsp_Init() ���á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
    /*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
    GPIO_InitTypeDef  GPIO_InitStruct;

    /*����LED��ص�GPIO����ʱ��*/
    LED1_GPIO_CLK_ENABLE();
    LED2_GPIO_CLK_ENABLE();
    LED3_GPIO_CLK_ENABLE();

    /*ѡ��Ҫ���Ƶ�GPIO����*/
    GPIO_InitStruct.Pin = LED1_PIN;

    /*�������ŵ��������Ϊ�������*/
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;

    /*��������Ϊ����ģʽ*/
    GPIO_InitStruct.Pull  = GPIO_PULLUP;

    /*������������Ϊ���� */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /*���ÿ⺯����ʹ���������õ�GPIO_InitStructure��ʼ��GPIO*/
    HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);

    /*ѡ��Ҫ���Ƶ�GPIO����*/
    GPIO_InitStruct.Pin = LED2_PIN;
    HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);

    /*ѡ��Ҫ���Ƶ�GPIO����*/
    GPIO_InitStruct.Pin = LED3_PIN;
    HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);

    /*�ر�RGB��*/
    LED_RGBOFF;
}


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
