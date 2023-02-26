/*
*********************************************************************************************************
*
*	ģ������ : �ⲿSDRAM����ģ��
*	�ļ����� : bsp_fmc_sdram.c
*	��    �� : V2.4
*	˵    �� : ������STM32-V7����������SDRAM�ͺ�IS42S32800G-6BLI, 32λ����, ����32MB, 6ns�ٶ�(166MHz)
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2018-05-04 armfly  ��ʽ����
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
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
#define REFRESH_COUNT                    ((uint32_t)824)    /* SDRAM��ˢ�¼��� */

/* SDRAM�Ĳ������� */
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
*	�� �� ��: bsp_InitExtSDRAM
*	����˵��: ���������ⲿSDRAM��GPIO��FMC
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitExtSDRAM(void)
{
    SDRAM_HandleTypeDef      hsdram             = {0};
    FMC_SDRAM_TimingTypeDef  SDRAM_Timing       = {0};
    FMC_SDRAM_CommandTypeDef command            = {0};
    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit  = {0};

    /* FMC SDRAM���漰��GPIO���� */
    SDRAM_GPIOConfig();

    /* ����SDRAMʱ��Դ */
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

    /* ʹ�� FMC ʱ�� */
    __FMC_CLK_ENABLE();

    /* SDRAM���� */
    hsdram.Instance = FMC_SDRAM_DEVICE;

    /*
       FMCʹ�õ�HCLK3ʱ�ӣ�240MHz������SDRAM�Ļ�������2��Ƶ��Ҳ����120MHz����1��SDRAMʱ��������8.3ns
       ���������λ��Ϊ8.3ns��
    */
    SDRAM_Timing.LoadToActiveDelay    = 2; /* 16.6ns, TMRD�������ģʽ�Ĵ����������뼤�������ˢ������֮����ӳ� */
    SDRAM_Timing.ExitSelfRefreshDelay = 8; /* 66.4ns, TXSR����ӷ�����ˢ�����������������֮����ӳ� */
    SDRAM_Timing.SelfRefreshTime      = 5; /* 41.5ns, TRAS������̵���ˢ������ */
    SDRAM_Timing.RowCycleDelay        = 8; /* 66.4ns, TRC����ˢ������ͼ�������֮����ӳ� */
    SDRAM_Timing.WriteRecoveryTime    = 2; /* 16.6ns, TWR������д�����Ԥ�������֮����ӳ� */
    SDRAM_Timing.RPDelay              = 2; /* 16.6ns, TRP����Ԥ�����������������֮����ӳ� */
    SDRAM_Timing.RCDDelay             = 2; /* 16.6ns, TRCD���弤���������/д����֮����ӳ� */

    hsdram.Init.SDBank             = FMC_SDRAM_BANK2;               /* Ӳ��������õ�BANK2 */
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;   /* 9�� */
    hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;     /* 13�� */
    hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;	/* 32λ���� */
    hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;  /* SDRAM��4��BANK */
    hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;       /* �е�ַѡͨ����ʱ��CAS Latency��������Latency1��2��3��ʵ�ʲ���Latency3�ȶ� */
    hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE; /* ��ֹд���� */
    hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;                /* FMCʱ��240MHz��2��Ƶ���SDRAM����120MHz */
    hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;       /* ʹ�ܶ�ͻ�� */
    hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;       /* ��λ��CAS��ʱ���Ӻ���ٸ�SDRAMʱ�����ڶ�ȡ���ݣ�ʵ�ʲ��λ�������������ӳ� */

    /* ����SDRAM�������������� */
    if(HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler(__FILE__, __LINE__);
    }

    /* ���SDRAM���г�ʼ�� */
    SDRAM_Initialization_Sequence(&hsdram, &command);
}

/*
*********************************************************************************************************
*	�� �� ��: SDRAM_GPIOConfig
*	����˵��: ���������ⲿSDRAM��GPIO
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SDRAM_GPIOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* ʹ��SDRAM��ص�IOʱ�� */
    /*��ַ�ź���*/
    FMC_A0_GPIO_CLK();FMC_A1_GPIO_CLK(); FMC_A2_GPIO_CLK();
    FMC_A3_GPIO_CLK();FMC_A4_GPIO_CLK(); FMC_A5_GPIO_CLK();
    FMC_A6_GPIO_CLK();FMC_A7_GPIO_CLK(); FMC_A8_GPIO_CLK();
    FMC_A9_GPIO_CLK();FMC_A10_GPIO_CLK();FMC_A11_GPIO_CLK();
    FMC_A12_GPIO_CLK();
    /*�����ź���*/
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
    /*�����ź���*/
    FMC_CS_GPIO_CLK() ; FMC_BA0_GPIO_CLK(); FMC_BA1_GPIO_CLK() ;
    FMC_WE_GPIO_CLK() ; FMC_RAS_GPIO_CLK(); FMC_CAS_GPIO_CLK();
    FMC_CLK_GPIO_CLK(); FMC_CKE_GPIO_CLK(); FMC_UDQM_GPIO_CLK();
    FMC_LDQM_GPIO_CLK();FMC_UDQM2_GPIO_CLK();FMC_LDQM2_GPIO_CLK();

    /*-- SDRAM IO ���� -----------------------------------------------------*/
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;//����Ϊ���ù���
    GPIO_InitStructure.Pull      = GPIO_PULLUP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = GPIO_AF12_FMC;

    /*��ַ�ź��� �����������*/
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

    /*�����ź��� �����������*/
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
    /*�����ź���*/
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
*	�� �� ��: SDRAM��ʼ������
*	����˵��: ���SDRAM���г�ʼ��
*	��    ��: hsdram: SDRAM���
*			  Command: ����ṹ��ָ��
*	�� �� ֵ: None
*********************************************************************************************************
*/
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
    __IO uint32_t tmpmrd =0;

    /*##-1- ʱ��ʹ������ ##################################################*/
    Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* �������� */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /*##-2- �����ӳ٣�����100us ##################################################*/
    HAL_Delay(1);

    /*##-3- ����SDRAMԤ������PALL(precharge all) #############################*/
    Command->CommandMode = FMC_SDRAM_CMD_PALL;
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* �������� */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /*##-4- �Զ�ˢ������ #######################################################*/
    Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK;
    Command->AutoRefreshNumber = 8;
    Command->ModeRegisterDefinition = 0;

    /* �������� */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /*##-5- ����SDRAMģʽ�Ĵ��� ###############################################*/
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = tmpmrd;

    /* �������� */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /*##-6- ������ˢ���� ####################################################*/
    /*
        SDRAM refresh period / Number of rows��*SDRAMʱ���ٶ� �C 20
      = 64ms / 8192 * 100MHz - 20
      = 823.75 ȡֵ824
    */
    HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_TestExtSDRAM
*	����˵��: ɨ������ⲿSDRAM��ȫ����Ԫ��
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ����ͨ���� ����0��ʾ����Ԫ�ĸ�����
*********************************************************************************************************
*/
uint32_t bsp_TestExtSDRAM1(void)
{
    uint32_t i;
    uint32_t *pSRAM;
    uint8_t *pBytes;
    uint32_t err;
    const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

    /* дSRAM */
    pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        *pSRAM++ = i;
    }

    /* ��SRAM */
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

    /* ��SRAM �������󷴲�д�� */
    pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        *pSRAM = ~*pSRAM;
        pSRAM++;
    }

    /* �ٴαȽ�SDRAM������ */
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

    /* ���԰��ֽڷ�ʽ����, Ŀ������֤ FSMC_NBL0 �� FSMC_NBL1 ���� */
    pBytes = (uint8_t *)EXT_SDRAM_ADDR;
    for (i = 0; i < sizeof(ByteBuf); i++)
    {
        *pBytes++ = ByteBuf[i];
    }

    /* �Ƚ�SDRAM������ */
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
*	�� �� ��: bsp_TestExtSDRAM2
*	����˵��: ɨ������ⲿSDRAM����ɨ��ǰ��4M�ֽڵ��Դ档
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ����ͨ���� ����0��ʾ����Ԫ�ĸ�����
*********************************************************************************************************
*/
uint32_t bsp_TestExtSDRAM2(void)
{
    uint32_t i;
    uint32_t *pSRAM;
    uint8_t *pBytes;
    uint32_t err;
    const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

    /* дSRAM */
    pSRAM = (uint32_t *)SDRAM_APP_BUF;
    for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
    {
        *pSRAM++ = i;
    }

    /* ��SRAM */
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
    /* ��SRAM �������󷴲�д�� */
    pSRAM = (uint32_t *)SDRAM_APP_BUF;
    for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
    {
        *pSRAM = ~*pSRAM;
        pSRAM++;
    }

    /* �ٴαȽ�SDRAM������ */
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

    /* ���԰��ֽڷ�ʽ����, Ŀ������֤ FSMC_NBL0 �� FSMC_NBL1 ���� */
    pBytes = (uint8_t *)SDRAM_APP_BUF;
    for (i = 0; i < sizeof(ByteBuf); i++)
    {
        *pBytes++ = ByteBuf[i];
    }

    /* �Ƚ�SDRAM������ */
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
  * @brief  ����SDRAM�Ƿ�����
  * @param  None
  * @retval ��������1���쳣����0
  */
uint8_t SDRAM_Test(void)
{
  /*д�����ݼ�����*/
  uint32_t counter=0;

  /* 8λ������ */
  uint8_t ubWritedata_8b = 0, ubReaddata_8b = 0;

  /* 16λ������ */
  uint16_t uhWritedata_16b = 0, uhReaddata_16b = 0;

    /* 32λ������ */
  uint32_t uhWritedata_32b = 0, uhReaddata_32b = 0;

  SDRAM_INFO("���ڼ��SDRAM����8λ��16��32λ�ķ�ʽ��дsdram...");


  /*��8λ��ʽ��д���ݣ���У��*/

  /* ��SDRAM����ȫ������Ϊ0 ��SDRAM_SIZE����8λΪ��λ�� */
  for (counter = 0x00; counter < EXT_SDRAM_SIZE; counter++)
  {
    *(__IO uint8_t*) (EXT_SDRAM_ADDR + counter) = (uint8_t)0x0;
  }

  /* ������SDRAMд������  8λ */
  for (counter = 0; counter < EXT_SDRAM_SIZE; counter++)
  {
    *(__IO uint8_t*) (EXT_SDRAM_ADDR + counter) = (uint8_t)(ubWritedata_8b + counter);
  }

  /* ��ȡ SDRAM ���ݲ����*/
  for(counter = 0; counter<EXT_SDRAM_SIZE;counter++ )
  {
    ubReaddata_8b = *(__IO uint8_t*)(EXT_SDRAM_ADDR + counter);  //�Ӹõ�ַ��������

    if(ubReaddata_8b != (uint8_t)(ubWritedata_8b + counter))      //������ݣ�������ȣ���������,���ؼ��ʧ�ܽ����
    {
      SDRAM_ERROR("8λ���ݶ�д���󣡳���λ�ã�%d",counter);
      return 0;
    }
  }


  /*��16λ��ʽ��д���ݣ������*/

  /* ��SDRAM����ȫ������Ϊ0 */
  for (counter = 0x00; counter < EXT_SDRAM_SIZE/2; counter++)
  {
    *(__IO uint16_t*) (EXT_SDRAM_ADDR + 2*counter) = (uint16_t)0x00;
  }

  /* ������SDRAMд������  16λ */
  for (counter = 0; counter < EXT_SDRAM_SIZE/2; counter++)
  {
    *(__IO uint16_t*) (EXT_SDRAM_ADDR + 2*counter) = (uint16_t)(uhWritedata_16b + counter);
  }

    /* ��ȡ SDRAM ���ݲ����*/
  for(counter = 0; counter<EXT_SDRAM_SIZE/2;counter++ )
  {
    uhReaddata_16b = *(__IO uint16_t*)(EXT_SDRAM_ADDR + 2*counter);  //�Ӹõ�ַ��������

    if(uhReaddata_16b != (uint16_t)(uhWritedata_16b + counter))      //������ݣ�������ȣ���������,���ؼ��ʧ�ܽ����
    {
      SDRAM_ERROR("16λ���ݶ�д���󣡳���λ�ã�%d",counter);

      return 0;
    }
  }


     /*��32λ��ʽ��д���ݣ������*/

  /* ��SDRAM����ȫ������Ϊ0 */
  for (counter = 0x00; counter < EXT_SDRAM_SIZE/4; counter++)
  {
    *(__IO uint32_t*) (EXT_SDRAM_ADDR + 4*counter) = (uint32_t)0x00;
  }

  /* ������SDRAMд������  32λ */
  for (counter = 0; counter < EXT_SDRAM_SIZE/4; counter++)
  {
    *(__IO uint32_t*) (EXT_SDRAM_ADDR + 4*counter) = (uint32_t)(uhWritedata_32b + counter);
  }

    /* ��ȡ SDRAM ���ݲ����*/
  for(counter = 0; counter<EXT_SDRAM_SIZE/4;counter++ )
  {
    uhReaddata_32b = *(__IO uint32_t*)(EXT_SDRAM_ADDR + 4*counter);  //�Ӹõ�ַ��������

    if(uhReaddata_32b != (uint32_t)(uhWritedata_32b + counter))      //������ݣ�������ȣ���������,���ؼ��ʧ�ܽ����
    {
      SDRAM_ERROR("32λ���ݶ�д���󣡳���λ�ã�%d",counter);

      return 0;
    }
  }

  SDRAM_INFO("SDRAM��д����������");
  /*���������return 1 */
  return 1;
}


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
