/*
*********************************************************************************************************
*
*	模块名称 : 外部SDRAM驱动模块
*	文件名称 : bsp_fmc_sdram.c
*	版    本 : V2.4
*	说    明 : 安富莱STM32-V7开发板标配的SDRAM型号IS42S32800G-6BLI, 32位带宽, 容量32MB, 6ns速度(166MHz)
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-05-04 armfly  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp_fmc_sdram.h"

/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_8  */
/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_16 */
#define SDRAM_MEMORY_WIDTH               FMC_SDRAM_MEM_BUS_WIDTH_32

#define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2
/* #define SDCLOCK_PERIOD                FMC_SDRAM_CLOCK_PERIOD_3 */

#define SDRAM_TIMEOUT                    ((uint32_t)0xFFFF)
#define REFRESH_COUNT                    ((uint32_t)824)    /* SDRAM自刷新计数 */

/* SDRAM的参数配置 */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

static void SDRAM_GPIOConfig(void);
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);


/*
*********************************************************************************************************
*	函 数 名: bsp_InitExtSDRAM
*	功能说明: 配置连接外部SDRAM的GPIO和FMC
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitExtSDRAM(void)
{
    SDRAM_HandleTypeDef      hsdram             = {0};
    FMC_SDRAM_TimingTypeDef  SDRAM_Timing       = {0};
    FMC_SDRAM_CommandTypeDef command            = {0};
    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit  = {0};

    /* FMC SDRAM所涉及到GPIO配置 */
    SDRAM_GPIOConfig();

    /* 配置SDRAM时钟源 */
    RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FMC;
    RCC_PeriphClkInit.PLL2.PLL2M = 5;
    RCC_PeriphClkInit.PLL2.PLL2N = 144;
    RCC_PeriphClkInit.PLL2.PLL2P = 2;
    RCC_PeriphClkInit.PLL2.PLL2Q = 2;
    RCC_PeriphClkInit.PLL2.PLL2R = 3;
    RCC_PeriphClkInit.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
    RCC_PeriphClkInit.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    RCC_PeriphClkInit.PLL2.PLL2FRACN = 0;
    RCC_PeriphClkInit.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit) != HAL_OK)
    {
        while (1)
            ;
    }

    /* 使能 FMC 时钟 */
    __FMC_CLK_ENABLE();

    /* SDRAM配置 */
    hsdram.Instance = FMC_SDRAM_DEVICE;

    /*
       FMC使用的HCLK3时钟，240MHz，用于SDRAM的话，至少2分频，也就是120MHz，即1个SDRAM时钟周期是8.3ns
       下面参数单位均为8.3ns。
    */
    SDRAM_Timing.LoadToActiveDelay    = 2; /* 16.6ns, TMRD定义加载模式寄存器的命令与激活命令或刷新命令之间的延迟 */
    SDRAM_Timing.ExitSelfRefreshDelay = 8; /* 66.4ns, TXSR定义从发出自刷新命令到发出激活命令之间的延迟 */
    SDRAM_Timing.SelfRefreshTime      = 5; /* 41.5ns, TRAS定义最短的自刷新周期 */
    SDRAM_Timing.RowCycleDelay        = 8; /* 66.4ns, TRC定义刷新命令和激活命令之间的延迟 */
    SDRAM_Timing.WriteRecoveryTime    = 2; /* 16.6ns, TWR定义在写命令和预充电命令之间的延迟 */
    SDRAM_Timing.RPDelay              = 2; /* 16.6ns, TRP定义预充电命令与其它命令之间的延迟 */
    SDRAM_Timing.RCDDelay             = 2; /* 16.6ns, TRCD定义激活命令与读/写命令之间的延迟 */

    hsdram.Init.SDBank             = FMC_SDRAM_BANK2;               /* 硬件设计上用的BANK2 */
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;   /* 9列 */
    hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;     /* 13行 */
    hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;	/* 32位带宽 */
    hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;  /* SDRAM有4个BANK */
    hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;       /* 列地址选通信延时：CAS Latency可以设置Latency1，2和3，实际测试Latency3稳定 */
    hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE; /* 禁止写保护 */
    hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;                /* FMC时钟240MHz，2分频后给SDRAM，即120MHz */
    hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;       /* 使能读突发 */
    hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;       /* 此位定CAS延时后延后多少个SDRAM时钟周期读取数据，实际测此位可以设置无需延迟 */

    /* 配置SDRAM控制器基本参数 */
    if(HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler(__FILE__, __LINE__);
    }

    /* 完成SDRAM序列初始化 */
    SDRAM_Initialization_Sequence(&hsdram, &command);
}

/*
*********************************************************************************************************
*	函 数 名: SDRAM_GPIOConfig
*	功能说明: 配置连接外部SDRAM的GPIO
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void SDRAM_GPIOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能SDRAM相关的IO时钟 */
    /*地址信号线*/
    FMC_A0_GPIO_CLK();FMC_A1_GPIO_CLK(); FMC_A2_GPIO_CLK();
    FMC_A3_GPIO_CLK();FMC_A4_GPIO_CLK(); FMC_A5_GPIO_CLK();
    FMC_A6_GPIO_CLK();FMC_A7_GPIO_CLK(); FMC_A8_GPIO_CLK();
    FMC_A9_GPIO_CLK();FMC_A10_GPIO_CLK();FMC_A11_GPIO_CLK();
    FMC_A12_GPIO_CLK();
    /*数据信号线*/
    FMC_D0_GPIO_CLK(); FMC_D1_GPIO_CLK() ; FMC_D2_GPIO_CLK() ;
    FMC_D3_GPIO_CLK(); FMC_D4_GPIO_CLK() ; FMC_D5_GPIO_CLK() ;
    FMC_D6_GPIO_CLK(); FMC_D7_GPIO_CLK() ; FMC_D8_GPIO_CLK() ;
    FMC_D9_GPIO_CLK(); FMC_D10_GPIO_CLK(); FMC_D11_GPIO_CLK();
    FMC_D12_GPIO_CLK();FMC_D13_GPIO_CLK(); FMC_D14_GPIO_CLK();
    FMC_D15_GPIO_CLK();FMC_D16_GPIO_CLK(); FMC_D17_GPIO_CLK();
    FMC_D18_GPIO_CLK();FMC_D19_GPIO_CLK(); FMC_D20_GPIO_CLK();
    FMC_D21_GPIO_CLK();FMC_D22_GPIO_CLK(); FMC_D23_GPIO_CLK();
    FMC_D24_GPIO_CLK();FMC_D25_GPIO_CLK(); FMC_D26_GPIO_CLK();
    FMC_D27_GPIO_CLK();FMC_D28_GPIO_CLK(); FMC_D29_GPIO_CLK();
    FMC_D30_GPIO_CLK();FMC_D31_GPIO_CLK();
    /*控制信号线*/
    FMC_CS_GPIO_CLK() ; FMC_BA0_GPIO_CLK(); FMC_BA1_GPIO_CLK() ;
    FMC_WE_GPIO_CLK() ; FMC_RAS_GPIO_CLK(); FMC_CAS_GPIO_CLK();
    FMC_CLK_GPIO_CLK(); FMC_CKE_GPIO_CLK(); FMC_UDQM_GPIO_CLK();
    FMC_LDQM_GPIO_CLK();FMC_UDQM2_GPIO_CLK();FMC_LDQM2_GPIO_CLK();

    /*-- SDRAM IO 配置 -----------------------------------------------------*/
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;//配置为复用功能
    GPIO_InitStructure.Pull      = GPIO_PULLUP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = GPIO_AF12_FMC;

    /*地址信号线 针对引脚配置*/
    GPIO_InitStructure.Pin = FMC_A0_GPIO_PIN;
    HAL_GPIO_Init(FMC_A0_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A1_GPIO_PIN;
    HAL_GPIO_Init(FMC_A1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A2_GPIO_PIN;
    HAL_GPIO_Init(FMC_A2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A3_GPIO_PIN;
    HAL_GPIO_Init(FMC_A3_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A4_GPIO_PIN;
    HAL_GPIO_Init(FMC_A4_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A5_GPIO_PIN;
    HAL_GPIO_Init(FMC_A5_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A6_GPIO_PIN;
    HAL_GPIO_Init(FMC_A6_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A7_GPIO_PIN;
    HAL_GPIO_Init(FMC_A7_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A8_GPIO_PIN;
    HAL_GPIO_Init(FMC_A8_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A9_GPIO_PIN;
    HAL_GPIO_Init(FMC_A9_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A10_GPIO_PIN;
    HAL_GPIO_Init(FMC_A10_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A11_GPIO_PIN;
    HAL_GPIO_Init(FMC_A11_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_A12_GPIO_PIN;
    HAL_GPIO_Init(FMC_A12_GPIO_PORT, &GPIO_InitStructure);

    /*数据信号线 针对引脚配置*/
    GPIO_InitStructure.Pin = FMC_D0_GPIO_PIN;
    HAL_GPIO_Init(FMC_D0_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D1_GPIO_PIN;
    HAL_GPIO_Init(FMC_D1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D2_GPIO_PIN;
    HAL_GPIO_Init(FMC_D2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D3_GPIO_PIN;
    HAL_GPIO_Init(FMC_D3_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D4_GPIO_PIN;
    HAL_GPIO_Init(FMC_D4_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D5_GPIO_PIN;
    HAL_GPIO_Init(FMC_D5_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D6_GPIO_PIN;
    HAL_GPIO_Init(FMC_D6_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D7_GPIO_PIN;
    HAL_GPIO_Init(FMC_D7_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D8_GPIO_PIN;
    HAL_GPIO_Init(FMC_D8_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D9_GPIO_PIN;
    HAL_GPIO_Init(FMC_D9_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D10_GPIO_PIN;
    HAL_GPIO_Init(FMC_D10_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D11_GPIO_PIN;
    HAL_GPIO_Init(FMC_D11_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D12_GPIO_PIN;
    HAL_GPIO_Init(FMC_D12_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D13_GPIO_PIN;
    HAL_GPIO_Init(FMC_D13_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D14_GPIO_PIN;
    HAL_GPIO_Init(FMC_D14_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D15_GPIO_PIN;
    HAL_GPIO_Init(FMC_D15_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D16_GPIO_PIN;
    HAL_GPIO_Init(FMC_D16_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D17_GPIO_PIN;
    HAL_GPIO_Init(FMC_D17_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D18_GPIO_PIN;
    HAL_GPIO_Init(FMC_D18_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D19_GPIO_PIN;
    HAL_GPIO_Init(FMC_D19_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D20_GPIO_PIN;
    HAL_GPIO_Init(FMC_D20_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D21_GPIO_PIN;
    HAL_GPIO_Init(FMC_D21_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D22_GPIO_PIN;
    HAL_GPIO_Init(FMC_D22_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D23_GPIO_PIN;
    HAL_GPIO_Init(FMC_D23_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D24_GPIO_PIN;
    HAL_GPIO_Init(FMC_D24_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D25_GPIO_PIN;
    HAL_GPIO_Init(FMC_D25_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D26_GPIO_PIN;
    HAL_GPIO_Init(FMC_D26_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D27_GPIO_PIN;
    HAL_GPIO_Init(FMC_D27_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D28_GPIO_PIN;
    HAL_GPIO_Init(FMC_D28_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D29_GPIO_PIN;
    HAL_GPIO_Init(FMC_D29_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D30_GPIO_PIN;
    HAL_GPIO_Init(FMC_D30_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_D31_GPIO_PIN;
    HAL_GPIO_Init(FMC_D31_GPIO_PORT, &GPIO_InitStructure);
    /*控制信号线*/
    GPIO_InitStructure.Pin = FMC_CS_GPIO_PIN;
    HAL_GPIO_Init(FMC_CS_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_BA0_GPIO_PIN;
    HAL_GPIO_Init(FMC_BA0_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_BA1_GPIO_PIN;
    HAL_GPIO_Init(FMC_BA1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_WE_GPIO_PIN;
    HAL_GPIO_Init(FMC_WE_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_RAS_GPIO_PIN;
    HAL_GPIO_Init(FMC_RAS_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_CAS_GPIO_PIN;
    HAL_GPIO_Init(FMC_CAS_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_CLK_GPIO_PIN;
    HAL_GPIO_Init(FMC_CLK_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_CKE_GPIO_PIN;
    HAL_GPIO_Init(FMC_CKE_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_UDQM_GPIO_PIN;
    HAL_GPIO_Init(FMC_UDQM_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_LDQM_GPIO_PIN;
    HAL_GPIO_Init(FMC_LDQM_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_UDQM2_GPIO_PIN;
    HAL_GPIO_Init(FMC_UDQM2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = FMC_LDQM2_GPIO_PIN;
    HAL_GPIO_Init(FMC_LDQM2_GPIO_PORT, &GPIO_InitStructure);
}


/*
*********************************************************************************************************
*	函 数 名: SDRAM初始化序列
*	功能说明: 完成SDRAM序列初始化
*	形    参: hsdram: SDRAM句柄
*			  Command: 命令结构体指针
*	返 回 值: None
*********************************************************************************************************
*/
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
    __IO uint32_t tmpmrd =0;

    /*##-1- 时钟使能命令 ##################################################*/
    Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* 发送命令 */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /*##-2- 插入延迟，至少100us ##################################################*/
    HAL_Delay(1);

    /*##-3- 整个SDRAM预充电命令，PALL(precharge all) #############################*/
    Command->CommandMode = FMC_SDRAM_CMD_PALL;
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* 发送命令 */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /*##-4- 自动刷新命令 #######################################################*/
    Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK;
    Command->AutoRefreshNumber = 8;
    Command->ModeRegisterDefinition = 0;

    /* 发送命令 */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /*##-5- 配置SDRAM模式寄存器 ###############################################*/
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = tmpmrd;

    /* 发送命令 */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /*##-6- 设置自刷新率 ####################################################*/
    /*
        SDRAM refresh period / Number of rows）*SDRAM时钟速度 – 20
      = 64ms / 8192 * 100MHz - 20
      = 823.75 取值824
    */
    HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_TestExtSDRAM
*	功能说明: 扫描测试外部SDRAM的全部单元。
*	形    参: 无
*	返 回 值: 0 表示测试通过； 大于0表示错误单元的个数。
*********************************************************************************************************
*/
uint32_t bsp_TestExtSDRAM1(void)
{
    uint32_t i;
    uint32_t *pSRAM;
    uint8_t *pBytes;
    uint32_t err;
    const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

    /* 写SRAM */
    pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        *pSRAM++ = i;
    }

    /* 读SRAM */
    err = 0;
    pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        if (*pSRAM++ != i)
        {
            err++;
        }
    }

    if (err >  0)
    {
        return  (4 * err);
    }

    /* 对SRAM 的数据求反并写入 */
    pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        *pSRAM = ~*pSRAM;
        pSRAM++;
    }

    /* 再次比较SDRAM的数据 */
    err = 0;
    pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        if (*pSRAM++ != (~i))
        {
            err++;
        }
    }

    if (err >  0)
    {
        return (4 * err);
    }

    /* 测试按字节方式访问, 目的是验证 FSMC_NBL0 、 FSMC_NBL1 口线 */
    pBytes = (uint8_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < sizeof(ByteBuf); i++)
    {
        *pBytes++ = ByteBuf[i];
    }

    /* 比较SDRAM的数据 */
    err = 0;
    pBytes = (uint8_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < sizeof(ByteBuf); i++)
    {
        if (*pBytes++ != ByteBuf[i])
        {
            err++;
        }
    }
    if (err >  0)
    {
        return err;
    }
    return 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_TestExtSDRAM2
*	功能说明: 扫描测试外部SDRAM，不扫描前面4M字节的显存。
*	形    参: 无
*	返 回 值: 0 表示测试通过； 大于0表示错误单元的个数。
*********************************************************************************************************
*/
uint32_t bsp_TestExtSDRAM2(void)
{
    uint32_t i;
    uint32_t *pSRAM;
    uint8_t *pBytes;
    uint32_t err;
    const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

    /* 写SRAM */
    pSRAM = (uint32_t *)SDRAM_APP_BUF;
    for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
    {
        *pSRAM++ = i;
    }

    /* 读SRAM */
    err = 0;
    pSRAM = (uint32_t *)SDRAM_APP_BUF;
    for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
    {
        if (*pSRAM++ != i)
        {
            err++;
        }
    }

    if (err >  0)
    {
        return  (4 * err);
    }

#if 0
    /* 对SRAM 的数据求反并写入 */
    pSRAM = (uint32_t *)SDRAM_APP_BUF;
    for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
    {
        *pSRAM = ~*pSRAM;
        pSRAM++;
    }

    /* 再次比较SDRAM的数据 */
    err = 0;
    pSRAM = (uint32_t *)SDRAM_APP_BUF;
    for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
    {
        if (*pSRAM++ != (~i))
        {
            err++;
        }
    }

    if (err >  0)
    {
        return (4 * err);
    }
#endif

    /* 测试按字节方式访问, 目的是验证 FSMC_NBL0 、 FSMC_NBL1 口线 */
    pBytes = (uint8_t *)SDRAM_APP_BUF;
    for (i = 0; i < sizeof(ByteBuf); i++)
    {
        *pBytes++ = ByteBuf[i];
    }

    /* 比较SDRAM的数据 */
    err = 0;
    pBytes = (uint8_t *)SDRAM_APP_BUF;
    for (i = 0; i < sizeof(ByteBuf); i++)
    {
        if (*pBytes++ != ByteBuf[i])
        {
            err++;
        }
    }
    if (err >  0)
    {
        return err;
    }
    return 0;
}


/**
  * @brief  测试SDRAM是否正常
  * @param  None
  * @retval 正常返回1，异常返回0
  */
uint8_t SDRAM_Test(void)
{
  /*写入数据计数器*/
  uint32_t counter=0;

  /* 8位的数据 */
  uint8_t ubWritedata_8b = 0, ubReaddata_8b = 0;

  /* 16位的数据 */
  uint16_t uhWritedata_16b = 0, uhReaddata_16b = 0;

    /* 32位的数据 */
  uint32_t uhWritedata_32b = 0, uhReaddata_32b = 0;

  SDRAM_INFO("正在检测SDRAM，以8位、16、32位的方式读写sdram...");


  /*按8位格式读写数据，并校验*/

  /* 把SDRAM数据全部重置为0 ，SDRAM_SIZE是以8位为单位的 */
  for (counter = 0x00; counter < EXT_SDRAM_SIZE; counter++)
  {
    *(__IO uint8_t*) (EXT_SDRAM_ADDR + counter) = (uint8_t)0x0;
  }

  /* 向整个SDRAM写入数据  8位 */
  for (counter = 0; counter < EXT_SDRAM_SIZE; counter++)
  {
    *(__IO uint8_t*) (EXT_SDRAM_ADDR + counter) = (uint8_t)(ubWritedata_8b + counter);
  }

  /* 读取 SDRAM 数据并检测*/
  for(counter = 0; counter<EXT_SDRAM_SIZE;counter++ )
  {
    ubReaddata_8b = *(__IO uint8_t*)(EXT_SDRAM_ADDR + counter);  //从该地址读出数据

    if(ubReaddata_8b != (uint8_t)(ubWritedata_8b + counter))      //检测数据，若不相等，跳出函数,返回检测失败结果。
    {
      SDRAM_ERROR("8位数据读写错误！出错位置：%d",counter);
      return 0;
    }
  }


  /*按16位格式读写数据，并检测*/

  /* 把SDRAM数据全部重置为0 */
  for (counter = 0x00; counter < EXT_SDRAM_SIZE/2; counter++)
  {
    *(__IO uint16_t*) (EXT_SDRAM_ADDR + 2*counter) = (uint16_t)0x00;
  }

  /* 向整个SDRAM写入数据  16位 */
  for (counter = 0; counter < EXT_SDRAM_SIZE/2; counter++)
  {
    *(__IO uint16_t*) (EXT_SDRAM_ADDR + 2*counter) = (uint16_t)(uhWritedata_16b + counter);
  }

    /* 读取 SDRAM 数据并检测*/
  for(counter = 0; counter<EXT_SDRAM_SIZE/2;counter++ )
  {
    uhReaddata_16b = *(__IO uint16_t*)(EXT_SDRAM_ADDR + 2*counter);  //从该地址读出数据

    if(uhReaddata_16b != (uint16_t)(uhWritedata_16b + counter))      //检测数据，若不相等，跳出函数,返回检测失败结果。
    {
      SDRAM_ERROR("16位数据读写错误！出错位置：%d",counter);

      return 0;
    }
  }


     /*按32位格式读写数据，并检测*/

  /* 把SDRAM数据全部重置为0 */
  for (counter = 0x00; counter < EXT_SDRAM_SIZE/4; counter++)
  {
    *(__IO uint32_t*) (EXT_SDRAM_ADDR + 4*counter) = (uint32_t)0x00;
  }

  /* 向整个SDRAM写入数据  32位 */
  for (counter = 0; counter < EXT_SDRAM_SIZE/4; counter++)
  {
    *(__IO uint32_t*) (EXT_SDRAM_ADDR + 4*counter) = (uint32_t)(uhWritedata_32b + counter);
  }

    /* 读取 SDRAM 数据并检测*/
  for(counter = 0; counter<EXT_SDRAM_SIZE/4;counter++ )
  {
    uhReaddata_32b = *(__IO uint32_t*)(EXT_SDRAM_ADDR + 4*counter);  //从该地址读出数据

    if(uhReaddata_32b != (uint32_t)(uhWritedata_32b + counter))      //检测数据，若不相等，跳出函数,返回检测失败结果。
    {
      SDRAM_ERROR("32位数据读写错误！出错位置：%d",counter);

      return 0;
    }
  }

  SDRAM_INFO("SDRAM读写测试正常！");
  /*检测正常，return 1 */
  return 1;
}


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
