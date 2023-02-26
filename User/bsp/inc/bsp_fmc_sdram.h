/*
*********************************************************************************************************
*
*	模块名称 : 外部SDRAM驱动模块
*	文件名称 : bsp_fmc_sdram.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2014-05-04 armfly  ST固件库版本 V1.3.0
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_FMC_SDRAM_H
#define _BSP_FMC_SDRAM_H

#include "bsp.h"
#include "includes.h"

#define EXT_SDRAM_ADDR  	((uint32_t)0xD0000000)
#define EXT_SDRAM_SIZE		(64 * 1024 * 1024)  // 64M

/* SDRAM bank */  
#define FMC_BANK_SDRAM            FMC_Bank2_SDRAM  
#define FMC_COMMAND_TARGET_BANK   FMC_SDRAM_CMD_TARGET_BANK2

/* LCD显存,第1页, 分配2M字节 */
#define SDRAM_LCD_BUF1 		EXT_SDRAM_ADDR

/* LCD显存,第2页, 分配2M字节 */
#define SDRAM_LCD_BUF2		(EXT_SDRAM_ADDR + SDRAM_LCD_SIZE)

#define SDRAM_LCD_SIZE		(2 * 1024 * 1024)		/* 每层2M */
#define SDRAM_LCD_LAYER		2						/* 2层 */

/* 剩下的64 - 2 * 2 = 60M字节，提供给应用程序使用 */
#define SDRAM_APP_BUF		(EXT_SDRAM_ADDR + SDRAM_LCD_SIZE * SDRAM_LCD_LAYER)
#define SDRAM_APP_SIZE		(EXT_SDRAM_SIZE - SDRAM_LCD_SIZE * SDRAM_LCD_LAYER)

  

/* info output */
#define SDRAM_DEBUG_ON         1

#define SDRAM_INFO(fmt,arg...)           printf("<<-SDRAM-INFO->> "fmt"\n",##arg)
#define SDRAM_ERROR(fmt,arg...)          printf("<<-SDRAM-ERROR->> "fmt"\n",##arg)
#define SDRAM_DEBUG(fmt,arg...)          do{\
                                          if(SDRAM_DEBUG_ON)\
                                          printf("<<-SDRAM-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                          }while(0)


/* address line */  
#define FMC_A0_GPIO_PORT         GPIOF
#define FMC_A0_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A0_GPIO_PIN          GPIO_PIN_0

#define FMC_A1_GPIO_PORT         GPIOF
#define FMC_A1_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A1_GPIO_PIN          GPIO_PIN_1

#define FMC_A2_GPIO_PORT         GPIOF
#define FMC_A2_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A2_GPIO_PIN          GPIO_PIN_2

#define FMC_A3_GPIO_PORT         GPIOF
#define FMC_A3_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A3_GPIO_PIN          GPIO_PIN_3

#define FMC_A4_GPIO_PORT         GPIOF
#define FMC_A4_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A4_GPIO_PIN          GPIO_PIN_4

#define FMC_A5_GPIO_PORT         GPIOF
#define FMC_A5_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A5_GPIO_PIN          GPIO_PIN_5

#define FMC_A6_GPIO_PORT         GPIOF
#define FMC_A6_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A6_GPIO_PIN          GPIO_PIN_12

#define FMC_A7_GPIO_PORT         GPIOF
#define FMC_A7_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A7_GPIO_PIN          GPIO_PIN_13

#define FMC_A8_GPIO_PORT         GPIOF
#define FMC_A8_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A8_GPIO_PIN          GPIO_PIN_14

#define FMC_A9_GPIO_PORT         GPIOF
#define FMC_A9_GPIO_CLK()        __GPIOF_CLK_ENABLE()
#define FMC_A9_GPIO_PIN          GPIO_PIN_15

#define FMC_A10_GPIO_PORT        GPIOG
#define FMC_A10_GPIO_CLK()       __GPIOG_CLK_ENABLE()
#define FMC_A10_GPIO_PIN         GPIO_PIN_0

#define FMC_A11_GPIO_PORT        GPIOG
#define FMC_A11_GPIO_CLK()       __GPIOG_CLK_ENABLE()
#define FMC_A11_GPIO_PIN         GPIO_PIN_1

#define FMC_A12_GPIO_PORT        GPIOG
#define FMC_A12_GPIO_CLK()       __GPIOG_CLK_ENABLE()
#define FMC_A12_GPIO_PIN         GPIO_PIN_2
/* data line */
#define FMC_D0_GPIO_PORT         GPIOD
#define FMC_D0_GPIO_CLK()        __GPIOD_CLK_ENABLE()
#define FMC_D0_GPIO_PIN          GPIO_PIN_14

#define FMC_D1_GPIO_PORT         GPIOD
#define FMC_D1_GPIO_CLK()        __GPIOD_CLK_ENABLE()
#define FMC_D1_GPIO_PIN          GPIO_PIN_15

#define FMC_D2_GPIO_PORT         GPIOD
#define FMC_D2_GPIO_CLK()        __GPIOD_CLK_ENABLE()
#define FMC_D2_GPIO_PIN          GPIO_PIN_0

#define FMC_D3_GPIO_PORT         GPIOD
#define FMC_D3_GPIO_CLK()        __GPIOD_CLK_ENABLE()
#define FMC_D3_GPIO_PIN          GPIO_PIN_1

#define FMC_D4_GPIO_PORT         GPIOE
#define FMC_D4_GPIO_CLK()        __GPIOE_CLK_ENABLE()
#define FMC_D4_GPIO_PIN          GPIO_PIN_7

#define FMC_D5_GPIO_PORT         GPIOE
#define FMC_D5_GPIO_CLK()        __GPIOE_CLK_ENABLE()
#define FMC_D5_GPIO_PIN          GPIO_PIN_8

#define FMC_D6_GPIO_PORT         GPIOE
#define FMC_D6_GPIO_CLK()        __GPIOE_CLK_ENABLE()
#define FMC_D6_GPIO_PIN          GPIO_PIN_9

#define FMC_D7_GPIO_PORT         GPIOE
#define FMC_D7_GPIO_CLK()        __GPIOE_CLK_ENABLE()
#define FMC_D7_GPIO_PIN          GPIO_PIN_10

#define FMC_D8_GPIO_PORT         GPIOE
#define FMC_D8_GPIO_CLK()        __GPIOE_CLK_ENABLE()
#define FMC_D8_GPIO_PIN          GPIO_PIN_11

#define FMC_D9_GPIO_PORT         GPIOE
#define FMC_D9_GPIO_CLK()        __GPIOE_CLK_ENABLE()
#define FMC_D9_GPIO_PIN          GPIO_PIN_12

#define FMC_D10_GPIO_PORT        GPIOE
#define FMC_D10_GPIO_CLK()       __GPIOE_CLK_ENABLE()
#define FMC_D10_GPIO_PIN         GPIO_PIN_13

#define FMC_D11_GPIO_PORT        GPIOE
#define FMC_D11_GPIO_CLK()       __GPIOE_CLK_ENABLE()
#define FMC_D11_GPIO_PIN         GPIO_PIN_14

#define FMC_D12_GPIO_PORT        GPIOE
#define FMC_D12_GPIO_CLK()       __GPIOE_CLK_ENABLE()
#define FMC_D12_GPIO_PIN         GPIO_PIN_15

#define FMC_D13_GPIO_PORT        GPIOD
#define FMC_D13_GPIO_CLK()       __GPIOD_CLK_ENABLE()
#define FMC_D13_GPIO_PIN         GPIO_PIN_8

#define FMC_D14_GPIO_PORT        GPIOD
#define FMC_D14_GPIO_CLK()       __GPIOD_CLK_ENABLE()
#define FMC_D14_GPIO_PIN         GPIO_PIN_9

#define FMC_D15_GPIO_PORT        GPIOD
#define FMC_D15_GPIO_CLK()       __GPIOD_CLK_ENABLE()
#define FMC_D15_GPIO_PIN         GPIO_PIN_10

#define FMC_D16_GPIO_PORT         GPIOH
#define FMC_D16_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_D16_GPIO_PIN          GPIO_PIN_8

#define FMC_D17_GPIO_PORT         GPIOH
#define FMC_D17_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_D17_GPIO_PIN          GPIO_PIN_9

#define FMC_D18_GPIO_PORT         GPIOH
#define FMC_D18_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_D18_GPIO_PIN          GPIO_PIN_10

#define FMC_D19_GPIO_PORT         GPIOH
#define FMC_D19_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_D19_GPIO_PIN          GPIO_PIN_11

#define FMC_D20_GPIO_PORT         GPIOH
#define FMC_D20_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_D20_GPIO_PIN          GPIO_PIN_12

#define FMC_D21_GPIO_PORT         GPIOH
#define FMC_D21_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_D21_GPIO_PIN          GPIO_PIN_13

#define FMC_D22_GPIO_PORT         GPIOH
#define FMC_D22_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_D22_GPIO_PIN          GPIO_PIN_14

#define FMC_D23_GPIO_PORT         GPIOH
#define FMC_D23_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_D23_GPIO_PIN          GPIO_PIN_15

#define FMC_D24_GPIO_PORT         GPIOI
#define FMC_D24_GPIO_CLK()        __GPIOI_CLK_ENABLE()
#define FMC_D24_GPIO_PIN          GPIO_PIN_0

#define FMC_D25_GPIO_PORT         GPIOI
#define FMC_D25_GPIO_CLK()        __GPIOI_CLK_ENABLE()
#define FMC_D25_GPIO_PIN          GPIO_PIN_1

#define FMC_D26_GPIO_PORT        GPIOI
#define FMC_D26_GPIO_CLK()       __GPIOI_CLK_ENABLE()
#define FMC_D26_GPIO_PIN         GPIO_PIN_2

#define FMC_D27_GPIO_PORT        GPIOI
#define FMC_D27_GPIO_CLK()       __GPIOI_CLK_ENABLE()
#define FMC_D27_GPIO_PIN         GPIO_PIN_3

#define FMC_D28_GPIO_PORT        GPIOI
#define FMC_D28_GPIO_CLK()       __GPIOI_CLK_ENABLE()
#define FMC_D28_GPIO_PIN         GPIO_PIN_6

#define FMC_D29_GPIO_PORT        GPIOI
#define FMC_D29_GPIO_CLK()       __GPIOI_CLK_ENABLE()
#define FMC_D29_GPIO_PIN         GPIO_PIN_7

#define FMC_D30_GPIO_PORT        GPIOI
#define FMC_D30_GPIO_CLK()       __GPIOI_CLK_ENABLE()
#define FMC_D30_GPIO_PIN         GPIO_PIN_9

#define FMC_D31_GPIO_PORT        GPIOI
#define FMC_D31_GPIO_CLK()       __GPIOI_CLK_ENABLE()
#define FMC_D31_GPIO_PIN         GPIO_PIN_10

/* control line */  
#define FMC_CS_GPIO_PORT         GPIOH
#define FMC_CS_GPIO_CLK()        __GPIOH_CLK_ENABLE()
#define FMC_CS_GPIO_PIN          GPIO_PIN_6

#define FMC_BA0_GPIO_PORT        GPIOG
#define FMC_BA0_GPIO_CLK()       __GPIOG_CLK_ENABLE()
#define FMC_BA0_GPIO_PIN         GPIO_PIN_4

#define FMC_BA1_GPIO_PORT        GPIOG
#define FMC_BA1_GPIO_CLK()       __GPIOG_CLK_ENABLE()
#define FMC_BA1_GPIO_PIN         GPIO_PIN_5

#define FMC_WE_GPIO_PORT         GPIOC
#define FMC_WE_GPIO_CLK()        __GPIOC_CLK_ENABLE()
#define FMC_WE_GPIO_PIN          GPIO_PIN_0

#define FMC_RAS_GPIO_PORT        GPIOF
#define FMC_RAS_GPIO_CLK()       __GPIOF_CLK_ENABLE()
#define FMC_RAS_GPIO_PIN         GPIO_PIN_11

#define FMC_CAS_GPIO_PORT        GPIOG
#define FMC_CAS_GPIO_CLK()       __GPIOG_CLK_ENABLE()
#define FMC_CAS_GPIO_PIN         GPIO_PIN_15

#define FMC_CLK_GPIO_PORT        GPIOG
#define FMC_CLK_GPIO_CLK()       __GPIOG_CLK_ENABLE()
#define FMC_CLK_GPIO_PIN         GPIO_PIN_8

#define FMC_CKE_GPIO_PORT        GPIOH
#define FMC_CKE_GPIO_CLK()       __GPIOH_CLK_ENABLE()
#define FMC_CKE_GPIO_PIN         GPIO_PIN_7

/* UDQM LDQM */
#define FMC_UDQM_GPIO_PORT        GPIOE
#define FMC_UDQM_GPIO_CLK()       __GPIOE_CLK_ENABLE()
#define FMC_UDQM_GPIO_PIN         GPIO_PIN_1

#define FMC_LDQM_GPIO_PORT        GPIOE
#define FMC_LDQM_GPIO_CLK()       __GPIOE_CLK_ENABLE()
#define FMC_LDQM_GPIO_PIN         GPIO_PIN_0

/* UDQM2 LDQM2 */
#define FMC_UDQM2_GPIO_PORT        GPIOI
#define FMC_UDQM2_GPIO_CLK()       __GPIOI_CLK_ENABLE()
#define FMC_UDQM2_GPIO_PIN         GPIO_PIN_5

#define FMC_LDQM2_GPIO_PORT       GPIOI
#define FMC_LDQM2_GPIO_CLK()      __GPIOI_CLK_ENABLE()
#define FMC_LDQM2_GPIO_PIN        GPIO_PIN_4


void bsp_InitExtSDRAM(void);
uint32_t bsp_TestExtSDRAM1(void);
uint32_t bsp_TestExtSDRAM2(void);
uint8_t SDRAM_Test(void);
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
