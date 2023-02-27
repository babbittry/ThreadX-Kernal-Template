/*
*********************************************************************************************************
*	                                  
*	ģ������ : ThreadX
*	�ļ����� : mian.c
*	��    �� : V1.0
*	˵    �� : ThreadX����ģ��
*              ʵ��Ŀ�ģ�
*                1. ѧϰMDK,IAR,Embedded Studio��ThreadX����ģ�崴����              
*              ʵ�����ݣ�
*                1. �����������¼�������ͨ�����°���K1����ͨ�����ڴ�ӡ�����ջʹ�����
*                    ===============================================================
*                      OS CPU Usage =  1.94%
*                    ===============================================================
*                       Prio StackSize CurStack MaxStack Taskname
*                        2     4092      383      391    App Task Start
*                        3     4092      543      659    App Task Blink
*                        4     4092      391      391    App Task UserIF
*                        5     4092      543      659    App Task COM
*                       30     1020      519      519    App Task STAT
*                       31     1020      143       71    App Task IDLE
*                        0     1020      391      391    System Timer Thread                
*                    �����������ʹ��SecureCRT��V7���������д�������鿴��ӡ��Ϣ��
*                    App Task Start����  ������������������BSP����������
*                    App Task Blink����  ��LED ����˸��
*                    App Task UserIF���� ��������Ϣ����
*                    App Task COM����    �����������ڴ�ӡ��
*                    App Task STAT����   ��ͳ������
*                    App Task IDLE����   ����������
*                    System Timer Thread����ϵͳ��ʱ������
*                2. (1) �����õ�printf������ȫ��ͨ������App_Printfʵ�֡�
*                   (2) App_Printf���������ź����Ļ�������������Դ�������⡣
*              ע�����
*                1. ��ʵ���Ƽ�ʹ�ô������SecureCRT��Ҫ�����ڴ�ӡЧ�������롣�������
*                   V7��������������С�
*                2. ��ؽ��༭��������������TAB����Ϊ4���Ķ����ļ���Ҫ��������ʾ�����롣
*
*	�޸ļ�¼ :
*		�汾��   ����         ����            ˵��
*       V1.0    2020-09-06   Eric2013    1. ST�̼���1.9.0�汾
*                                        2. BSP������V1.2
*                                        3. ThreadX�汾V6.0.1
*                                       
*	Copyright (C), 2020-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"
         


/*
*********************************************************************************************************
*                                 �������ȼ�����ֵԽС���ȼ�Խ��
*********************************************************************************************************
*/
#define  APP_CFG_TASK_START_PRIO                          2u
#define  APP_CFG_TASK_BLINK_PRIO                          3u
#define  APP_CFG_TASK_USER_IF_PRIO                        4u
#define  APP_CFG_TASK_COM_PRIO                            5u
#define  APP_CFG_TASK_STAT_PRIO                           30u
#define  APP_CFG_TASK_IDLE_PRIO                           31u


/*
*********************************************************************************************************
*                                    ����ջ��С����λ�ֽ�
*********************************************************************************************************
*/
#define  APP_CFG_TASK_START_STK_SIZE                    4096u
#define  APP_CFG_TASK_BLINK_STK_SIZE                    512u
#define  APP_CFG_TASK_COM_STK_SIZE                      1024u
#define  APP_CFG_TASK_USER_IF_STK_SIZE                  1024u
#define  APP_CFG_TASK_IDLE_STK_SIZE                  	1024u
#define  APP_CFG_TASK_STAT_STK_SIZE                  	1024u

/*
*********************************************************************************************************
*                                       ��̬ȫ�ֱ���
*********************************************************************************************************
*/                                                        
static  TX_THREAD   AppTaskStartTCB;
static  uint64_t    AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE/8];

static  TX_THREAD   AppTaskBlinkTCB;
static  uint64_t    AppTaskBlinkStk[APP_CFG_TASK_BLINK_STK_SIZE/8];

static  TX_THREAD   AppTaskCOMTCB;
static  uint64_t    AppTaskCOMStk[APP_CFG_TASK_COM_STK_SIZE/8];

static  TX_THREAD   AppTaskUserIFTCB;
static  uint64_t    AppTaskUserIFStk[APP_CFG_TASK_USER_IF_STK_SIZE/8];

static  TX_THREAD   AppTaskIdleTCB;
static  uint64_t    AppTaskIdleStk[APP_CFG_TASK_IDLE_STK_SIZE/8];

static  TX_THREAD   AppTaskStatTCB;
static  uint64_t    AppTaskStatStk[APP_CFG_TASK_STAT_STK_SIZE/8];


/*
*********************************************************************************************************
*                                      ��������
*********************************************************************************************************
*/
static  void  AppTaskStart          (ULONG thread_input);
static  void  AppBlinkPro           (ULONG thread_input);
static  void  AppTaskUserIF         (ULONG thread_input);
static  void  AppTaskCOM            (ULONG thread_input);
static  void  AppTaskIDLE           (ULONG thread_input);
static  void  AppTaskStat           (ULONG thread_input);
static  void  App_Printf (const char *fmt, ...);
static  void  AppTaskCreate         (void);
static  void  DispTaskInfo          (void);
static  void  AppObjCreate          (void);
static  void  OSStatInit (void);

/*
*******************************************************************************************************
*                               ����
*******************************************************************************************************
*/
static  TX_MUTEX   AppPrintfSemp;	/* ����printf���� */


/* ͳ������ʹ�� */
__IO uint8_t   OSStatRdy;        /* ͳ�����������־ */
__IO uint32_t  OSIdleCtr;        /* ����������� */
__IO float     OSCPUUsage;       /* CPU�ٷֱ� */
uint32_t       OSIdleCtrMax;     /* 1�������Ŀ��м��� */
uint32_t       OSIdleCtrRun;     /* 1���ڿ�������ǰ���� */

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int main(void)
{
     /* HAL�⣬MPU��Cache��ʱ�ӵ�ϵͳ��ʼ�� */
    System_Init();

    /* �ں˿���ǰ�ر�HAL��ʱ���׼ */
    HAL_SuspendTick();
    
    /* ����ThreadX�ں� */
    tx_kernel_enter();

    while(1);
}

/*
*********************************************************************************************************
*	�� �� ��: tx_application_define
*	����˵��: ThreadXר�õ����񴴽���ͨ�������������
*	��    ��: first_unused_memory  δʹ�õĵ�ַ�ռ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void  tx_application_define(void *first_unused_memory)
{
    /*
       ���ʵ������CPU������ͳ�ƵĻ����˺���������ʵ����������ͳ������Ϳ����������������ں���
       AppTaskCreate���洴����
    */
    /**************������������*********************/
    tx_thread_create(&AppTaskStartTCB,                  /* ������ƿ��ַ */  
                       "App Task Start",                /* ������ */
                       AppTaskStart,                    /* ������������ַ */
                       0,                               /* ���ݸ�����Ĳ��� */
                       &AppTaskStartStk[0],             /* ��ջ����ַ */
                       APP_CFG_TASK_START_STK_SIZE,     /* ��ջ�ռ��С */  
                       APP_CFG_TASK_START_PRIO,         /* �������ȼ�*/
                       APP_CFG_TASK_START_PRIO,         /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,                /* ������ʱ��Ƭ */
                       TX_AUTO_START);                  /* �������������� */
          
    /**************����ͳ������*********************/
    tx_thread_create(&AppTaskStatTCB,                   /* ������ƿ��ַ */   
                       "App Task STAT",                 /* ������ */
                       AppTaskStat,                     /* ������������ַ */
                       0,                               /* ���ݸ�����Ĳ��� */
                       &AppTaskStatStk[0],              /* ��ջ����ַ */
                       APP_CFG_TASK_STAT_STK_SIZE,      /* ��ջ�ռ��С */  
                       APP_CFG_TASK_STAT_PRIO,          /* �������ȼ�*/
                       APP_CFG_TASK_STAT_PRIO,          /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,                /* ������ʱ��Ƭ */
                       TX_AUTO_START);                  /* �������������� */
                       
                   
    /**************������������*********************/
    tx_thread_create(&AppTaskIdleTCB,                   /* ������ƿ��ַ */    
                       "App Task IDLE",                 /* ������ */
                       AppTaskIDLE,                     /* ������������ַ */
                       0,                               /* ���ݸ�����Ĳ��� */
                       &AppTaskIdleStk[0],              /* ��ջ����ַ */
                       APP_CFG_TASK_IDLE_STK_SIZE,      /* ��ջ�ռ��С */  
                       APP_CFG_TASK_IDLE_PRIO,          /* �������ȼ�*/
                       APP_CFG_TASK_IDLE_PRIO,          /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,                /* ������ʱ��Ƭ */
                       TX_AUTO_START);                  /* �������������� */
               
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskStart
*	����˵��: ��������
*	��    ��: thread_input ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
    �� �� ��: 2
*********************************************************************************************************
*/
static  void  AppTaskStart (ULONG thread_input)
{
    (void)thread_input;

    /* ����ִ������ͳ�� */
    OSStatInit();

    /* �ں˿����󣬻ָ�HAL���ʱ���׼ */
    HAL_ResumeTick();
    
    /* �����ʼ�� */
    bsp_Init();
    
    /* �������� */
    AppTaskCreate(); 

    /* ���������ͨ�Ż��� */
    AppObjCreate();	

    while (1)
    {  
        /* ��Ҫ�����Դ���ĳ��򣬶�Ӧ������̵��õ�SysTick_ISR */
        bsp_ProPer1ms();
        tx_thread_sleep(1);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskBlink
*	����˵��: LED ����˸
*	��    ��: thread_input ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
    �� �� ��: 3
*********************************************************************************************************
*/
static void AppTaskBlink(ULONG thread_input)
{
    (void)thread_input;
          
    while(1)
    {
        LED2_ON;
        LED3_ON;
        tx_thread_sleep(5);
        LED2_OFF;
        LED3_OFF;
        tx_thread_sleep(995);
    }   
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskUserIF
*	����˵��: ������Ϣ����
*	��    ��: thread_input ����������ʱ���ݵ��β�
*	�� �� ֵ: ��
    �� �� ��: 4
*********************************************************************************************************
*/
static void AppTaskUserIF(ULONG thread_input)
{
    uint8_t ucKeyCode;	/* �������� */
    
    (void)thread_input;
          
    while(1)
    {        
        ucKeyCode = bsp_GetKey();
        
        if (ucKeyCode != KEY_NONE)
        {
            switch (ucKeyCode)
            {
                case KEY_DOWN_K1:			  /* K1������ӡ����ִ����� */
                     DispTaskInfo();
                    break;
                
                default:                     /* �����ļ�ֵ������ */
                    break;
            }
        }

        tx_thread_sleep(20);
    }
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCom
*	����˵��: ���������ڴ�ӡ
*	��    ��: thread_input ����������ʱ���ݵ��β�
*	�� �� ֵ: ��
    �� �� ��: 5
*********************************************************************************************************
*/
static void AppTaskCOM(ULONG thread_input)
{	
    double f_c = 1.1;
    double f_d = 2.2345;
    ULONG currentTime;
    (void)thread_input;
    
    while(1)
    {
        f_c += 0.00000000001;
        f_d -= 0.00000000002;;
        App_Printf("AppTaskCom: f_a = %.11f, f_b = %.11f\r\n", f_c, f_d);
        currentTime = tx_time_get();
        App_Printf("current time: %d\r\n", currentTime);
        tx_thread_sleep(2000);
    } 			  	 	       											   
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskStatistic
*	����˵��: ͳ����������ʵ��CPU�����ʵ�ͳ�ơ�Ϊ�˲��Ը���׼ȷ�����Կ���ע�͵��õ�ȫ���жϿ���
*	��    ��: thread_input ����������ʱ���ݵ��β� 
*	�� �� ֵ: ��
*   �� �� ��: 30
*********************************************************************************************************
*/
void  OSStatInit (void)
{
    OSStatRdy = FALSE;
    
    tx_thread_sleep(2u);        /* ʱ��ͬ�� */
    
    //__disable_irq();
    OSIdleCtr    = 0uL;         /* ����м��� */
    //__enable_irq();
    
    tx_thread_sleep(100);       /* ͳ��100ms�ڣ������м��� */
    
       //__disable_irq();
    OSIdleCtrMax = OSIdleCtr;   /* ���������м��� */
    OSStatRdy    = TRUE;
    //__enable_irq();
}

static void AppTaskStat(ULONG thread_input)
{
    (void)thread_input;

    while (OSStatRdy == FALSE) 
    {
        tx_thread_sleep(200);     /* �ȴ�ͳ��������� */
    }

    OSIdleCtrMax /= 100uL;
    if (OSIdleCtrMax == 0uL) 
    {
        OSCPUUsage = 0u;
    }
    
    //__disable_irq();
    OSIdleCtr = OSIdleCtrMax * 100uL;  /* ���ó�ʼCPU������ 0% */
    //__enable_irq();
    
    for (;;) 
    {
       // __disable_irq();
        OSIdleCtrRun = OSIdleCtr;    /* ���100ms�ڿ��м��� */
        OSIdleCtr    = 0uL;          /* ��λ���м��� */
       //	__enable_irq();            /* ����100ms�ڵ�CPU������ */
        OSCPUUsage   = (100uL - (float)OSIdleCtrRun / OSIdleCtrMax);
        tx_thread_sleep(100);        /* ÿ100msͳ��һ�� */
    }
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskIDLE
*	����˵��: ��������
*	��    ��: thread_input ����������ʱ���ݵ��β�
*	�� �� ֵ: ��
    �� �� ��: 31
*********************************************************************************************************
*/
static void AppTaskIDLE(ULONG thread_input)
{	
  TX_INTERRUPT_SAVE_AREA

  (void)thread_input;
    
  while(1)
  {
       TX_DISABLE
       OSIdleCtr++;
       TX_RESTORE
  }			  	 	       											   
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
    /**************����Blink����*********************/
    tx_thread_create(&AppTaskBlinkTCB,               /* ������ƿ��ַ */    
                       "App Task Blink",                 /* ������ */
                       AppTaskBlink,                  /* ������������ַ */
                       0,                             /* ���ݸ�����Ĳ��� */
                       &AppTaskBlinkStk[0],            /* ��ջ����ַ */
                       APP_CFG_TASK_BLINK_STK_SIZE,    /* ��ջ�ռ��С */  
                       APP_CFG_TASK_BLINK_PRIO,        /* �������ȼ�*/
                       APP_CFG_TASK_BLINK_PRIO,        /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,               /* ������ʱ��Ƭ */
                       TX_AUTO_START);                /* �������������� */
   

    /**************����USER IF����*********************/
    tx_thread_create(&AppTaskUserIFTCB,               /* ������ƿ��ַ */      
                       "App Task UserIF",              /* ������ */
                       AppTaskUserIF,                  /* ������������ַ */
                       0,                              /* ���ݸ�����Ĳ��� */
                       &AppTaskUserIFStk[0],            /* ��ջ����ַ */
                       APP_CFG_TASK_USER_IF_STK_SIZE,  /* ��ջ�ռ��С */  
                       APP_CFG_TASK_USER_IF_PRIO,      /* �������ȼ�*/
                       APP_CFG_TASK_USER_IF_PRIO,      /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,               /* ������ʱ��Ƭ */
                       TX_AUTO_START);                 /* �������������� */

    /**************����COM����*********************/
    tx_thread_create(&AppTaskCOMTCB,               /* ������ƿ��ַ */    
                       "App Task COM",              /* ������ */
                       AppTaskCOM,                  /* ������������ַ */
                       0,                           /* ���ݸ�����Ĳ��� */
                       &AppTaskCOMStk[0],            /* ��ջ����ַ */
                       APP_CFG_TASK_COM_STK_SIZE,    /* ��ջ�ռ��С */  
                       APP_CFG_TASK_COM_PRIO,        /* �������ȼ�*/
                       APP_CFG_TASK_COM_PRIO,        /* ������ռ��ֵ */
                       TX_NO_TIME_SLICE,             /* ������ʱ��Ƭ */
                       TX_AUTO_START);               /* �������������� */
}

/*
*********************************************************************************************************
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨѶ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
     /* ���������ź��� */
    tx_mutex_create(&AppPrintfSemp, "AppPrintfSemp", TX_NO_INHERIT);
}

/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  App_Printf(const char *fmt, ...)
{
    char  buf_str[200 + 1]; /* �ر�ע�⣬���printf�ı����϶࣬ע��˾ֲ������Ĵ�С�Ƿ��� */
    va_list   v_args;


    va_start(v_args, fmt);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) fmt,
                                  v_args);
    va_end(v_args);

    /* ������� */
    tx_mutex_get(&AppPrintfSemp, TX_WAIT_FOREVER);

    printf("%s", buf_str);

    tx_mutex_put(&AppPrintfSemp);
}

/*
*********************************************************************************************************
*	�� �� ��: DispTaskInfo
*	����˵��: ��uCOS-III������Ϣͨ�����ڴ�ӡ����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispTaskInfo(void)
{
    TX_THREAD      *p_tcb;	        /* ����һ��������ƿ�ָ�� */

    p_tcb = &AppTaskStartTCB;
    
    /* ��ӡ���� */
    App_Printf("===============================================================\r\n");
    App_Printf("OS CPU Usage = %5.2f%%\r\n", OSCPUUsage);
    App_Printf("===============================================================\r\n");
    App_Printf(" �������ȼ� ����ջ��С ��ǰʹ��ջ  ���ջʹ��   ������\r\n");
    App_Printf("   Prio     StackSize   CurStack    MaxStack   Taskname\r\n");

    /* ����������ƿ���?TCB list)����ӡ���е���������ȼ�����?*/
    while (p_tcb != (TX_THREAD *)0) 
    {
        
        App_Printf("   %2d        %5d      %5d       %5d      %s\r\n", 
                    p_tcb->tx_thread_priority,
                    p_tcb->tx_thread_stack_size,
                    (int)p_tcb->tx_thread_stack_end - (int)p_tcb->tx_thread_stack_ptr,
                    (int)p_tcb->tx_thread_stack_end - (int)p_tcb->tx_thread_stack_highest_ptr,
                    p_tcb->tx_thread_name);


        p_tcb = p_tcb->tx_thread_created_next;

        if(p_tcb == &AppTaskStartTCB) break;
    }
    App_Printf("===============================================================\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
