/*
*********************************************************************************************************
*
*	模块名称 : ThreadX
*	文件名称 : mian.c
*	版    本 : V1.0
*	说    明 : ThreadX程序模板
*              实验目的：
*                1. 学习MDK,IAR,Embedded Studio的ThreadX程序模板创建。
*              实验内容：
*                1. 共创建了如下几个任务，通过按下按键K1可以通过串口打印任务堆栈使用情况
*                    ===============================================================
*                    CPU利用率 = 0.29%
*                    CPU利用率 = 1.038981031s
*                    任务执行时间 = 475.218665115s
*                    空闲执行时间 = 0.369709350s
*                    系统总执行时间 = 476.627355496s
*                    ===============================================================
*                     任务优先级   任务栈大小   当前使用栈    最大栈使用   任务名
*                       Prio     StackSize   CurStack    MaxStack   Taskname
*                        2         4092        671         671      App Task Start
*                       31         1020        111         111      App Task IDLE
*                        3          508        111         111      App Task Blink
*                        4         1020        255         635      App Task UserIF
*                        5         1020        255         571      App Task COM
*                        0         1020        159         159      System Timer Thread
*                    ===============================================================
*                    串口软件建议使用SecureCRT（V7光盘里面有此软件）查看打印信息。
*                    App Task Start任务  ：启动任务，这里用作BSP驱动包处理。
*                    App Task Blink任务  ：LED 灯闪烁。
*                    App Task UserIF任务 ：按键消息处理。
*                    App Task COM任务    ：浮点数串口打印。
*                    App Task IDLE任务   ：空闲任务
*                    System Timer Thread任务：系统定时器任务
*                2. (1) 凡是用到printf函数的全部通过函数App_Printf实现。
*                   (2) App_Printf函数做了信号量的互斥操作，解决资源共享问题。
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT，要不串口打印效果不整齐。此软件在
*                   V7开发板光盘里面有。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者            说明
*       V1.0    2020-09-06   Eric2013    1. ST固件库1.9.0版本
*                                        2. BSP驱动包V1.2
*                                        3. ThreadX版本V6.0.1
*       V1.1    2023-03-06   ZXY         1. ST固件库1.11.0版本
*                                        2. BSP驱动包V1.x
*                                        3. ThreadX版本V6.2.0
*       V1.2    2023-03-12   ZXY         1. ThreadX版本V6.2.1
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"



/*
*********************************************************************************************************
*                                 任务优先级，数值越小优先级越高
*********************************************************************************************************
*/
#define  APP_CFG_TASK_START_PRIO                          2u
#define  APP_CFG_TASK_BLINK_PRIO                          3u
#define  APP_CFG_TASK_USER_IF_PRIO                        4u
#define  APP_CFG_TASK_COM_PRIO                            5u
#define  APP_CFG_TASK_IDLE_PRIO                           31u


/*
*********************************************************************************************************
*                                    任务栈大小，单位字节
*********************************************************************************************************
*/
#define  APP_CFG_TASK_START_STK_SIZE                    4096u
#define  APP_CFG_TASK_BLINK_STK_SIZE                    512u
#define  APP_CFG_TASK_COM_STK_SIZE                      1024u
#define  APP_CFG_TASK_USER_IF_STK_SIZE                  1024u
#define  APP_CFG_TASK_IDLE_STK_SIZE                     1024u

/*
*********************************************************************************************************
*                                       静态全局变量
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


/*
*********************************************************************************************************
*                                      函数声明
*********************************************************************************************************
*/
static  void  AppTaskStart          (ULONG thread_input);
static  void  AppTaskBlink          (ULONG thread_input);
static  void  AppTaskUserIF         (ULONG thread_input);
static  void  AppTaskCOM            (ULONG thread_input);
static  void  AppTaskIDLE           (ULONG thread_input);
static  void  App_Printf            (const char *fmt, ...);
static  void  AppTaskCreate         (void);
static  void  DispTaskInfo          (void);
static  void  AppObjCreate          (void);

/*
*******************************************************************************************************
*                               变量
*******************************************************************************************************
*/
static  TX_MUTEX   AppPrintfSemp;	/* 用于printf互斥 */


/* 统计任务使用 */
__IO double    OSCPUUsage;     /* CPU 百分比 */

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
     /* HAL库，MPU，Cache，时钟等系统初始化 */
    System_Init();

    /* 内核开启前关闭HAL的时间基准 */
    HAL_SuspendTick();

    /* 进入ThreadX内核 */
    tx_kernel_enter();

    while(1);
}

/*
*********************************************************************************************************
*	函 数 名: tx_application_define
*	功能说明: ThreadX专用的任务创建，通信组件创建函数
*	形    参: first_unused_memory  未使用的地址空间
*	返 回 值: 无
*********************************************************************************************************
*/
void  tx_application_define(void *first_unused_memory)
{
    /*
       如果实现任务CPU利用率统计的话，此函数仅用于实现启动任务，统计任务和空闲任务，其它任务在函数
       AppTaskCreate里面创建。
    */
    /**************创建启动任务*********************/
    tx_thread_create(&AppTaskStartTCB,                  /* 任务控制块地址 */
                       "App Task Start",                /* 任务名 */
                       AppTaskStart,                    /* 启动任务函数地址 */
                       0,                               /* 传递给任务的参数 */
                       &AppTaskStartStk[0],             /* 堆栈基地址 */
                       APP_CFG_TASK_START_STK_SIZE,     /* 堆栈空间大小 */
                       APP_CFG_TASK_START_PRIO,         /* 任务优先级*/
                       APP_CFG_TASK_START_PRIO,         /* 任务抢占阀值 */
                       TX_NO_TIME_SLICE,                /* 不开启时间片 */
                       TX_AUTO_START);                  /* 创建后立即启动 */

    /**************创建空闲任务*********************/
    tx_thread_create(&AppTaskIdleTCB,                   /* 任务控制块地址 */
                       "App Task IDLE",                 /* 任务名 */
                       AppTaskIDLE,                     /* 启动任务函数地址 */
                       0,                               /* 传递给任务的参数 */
                       &AppTaskIdleStk[0],              /* 堆栈基地址 */
                       APP_CFG_TASK_IDLE_STK_SIZE,      /* 堆栈空间大小 */
                       APP_CFG_TASK_IDLE_PRIO,          /* 任务优先级*/
                       APP_CFG_TASK_IDLE_PRIO,          /* 任务抢占阀值 */
                       TX_NO_TIME_SLICE,                /* 不开启时间片 */
                       TX_AUTO_START);                  /* 创建后立即启动 */

}

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 启动任务。
*	形    参: thread_input 是在创建该任务时传递的形参
*	返 回 值: 无
    优 先 级: 2
*********************************************************************************************************
*/
static  void  AppTaskStart (ULONG thread_input)
{
    /* 
     * TolTime: 总的时间
     * IdleTime： 总的空闲时间
     * deltaTolTime：200ms 内的总时间
     * deltaIdleTime： 200ms 内的空闲时间 
     */
    EXECUTION_TIME TolTime, IdleTime, deltaTolTime, deltaIdleTime;
    uint32_t uiCount = 0;
    (void)thread_input;

    /* 内核开启后，恢复HAL里的时间基准 */
    HAL_ResumeTick();

    /* 外设初始化 */
    bsp_Init();

    /* 创建任务 */
    AppTaskCreate();

    /* 创建任务间通信机制 */
    AppObjCreate();

    /* 计算CPU利用率 */
    IdleTime = _tx_execution_idle_time_total;
    TolTime = _tx_execution_thread_time_total + _tx_execution_isr_time_total + _tx_execution_idle_time_total;

    while (1)
    {
        /* 需要周期性处理的程序，对应裸机工程调用的SysTick_ISR */
        bsp_ProPer1ms();

        /* CPU利用率统计 */
        uiCount++;
        if(uiCount == 200)
        {
            uiCount = 0;
            deltaIdleTime = _tx_execution_idle_time_total - IdleTime;
            deltaTolTime = _tx_execution_thread_time_total + _tx_execution_isr_time_total + _tx_execution_idle_time_total - TolTime;
            OSCPUUsage = (double)deltaIdleTime / deltaTolTime;
            OSCPUUsage = 100- OSCPUUsage * 100;
            IdleTime = _tx_execution_idle_time_total;
            TolTime = _tx_execution_thread_time_total + _tx_execution_isr_time_total + _tx_execution_idle_time_total;
        }
        tx_thread_sleep(1);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskBlink
*	功能说明: LED 灯闪烁
*	形    参: thread_input 是在创建该任务时传递的形参
*	返 回 值: 无
    优 先 级: 3
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
*	函 数 名: AppTaskUserIF
*	功能说明: 按键消息处理
*	形    参: thread_input 创建该任务时传递的形参
*	返 回 值: 无
    优 先 级: 4
*********************************************************************************************************
*/
static void AppTaskUserIF(ULONG thread_input)
{
    uint8_t ucKeyCode;	/* 按键代码 */

    (void)thread_input;

    while(1)
    {
        ucKeyCode = bsp_GetKey();

        if (ucKeyCode != KEY_NONE)
        {
            switch (ucKeyCode)
            {
                case KEY_DOWN_K1:			  /* K1键按打印任务执行情况 */
                     DispTaskInfo();
                    break;

                default:                     /* 其他的键值不处理 */
                    break;
            }
        }

        tx_thread_sleep(20);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCom
*	功能说明: 浮点数串口打印
*	形    参: thread_input 创建该任务时传递的形参
*	返 回 值: 无
    优 先 级: 5
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
*	函 数 名: AppTaskIDLE
*	功能说明: 空闲任务
*	形    参: thread_input 创建该任务时传递的形参
*	返 回 值: 无
    优 先 级: 31
*********************************************************************************************************
*/
static void AppTaskIDLE(ULONG thread_input)
{
  TX_INTERRUPT_SAVE_AREA

  (void)thread_input;

  while(1)
  {
        TX_DISABLE
        TX_RESTORE
        tx_thread_sleep(2000);
  }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
    /**************创建Blink任务*********************/
    tx_thread_create(&AppTaskBlinkTCB,                  /* 任务控制块地址 */
                       "App Task Blink",                /* 任务名 */
                       AppTaskBlink,                    /* 启动任务函数地址 */
                       0,                               /* 传递给任务的参数 */
                       &AppTaskBlinkStk[0],             /* 堆栈基地址 */
                       APP_CFG_TASK_BLINK_STK_SIZE,     /* 堆栈空间大小 */
                       APP_CFG_TASK_BLINK_PRIO,         /* 任务优先级*/
                       APP_CFG_TASK_BLINK_PRIO,         /* 任务抢占阀值 */
                       TX_NO_TIME_SLICE,                /* 不开启时间片 */
                       TX_AUTO_START);                  /* 创建后立即启动 */


    /**************创建USER IF任务*********************/
    tx_thread_create(&AppTaskUserIFTCB,                 /* 任务控制块地址 */
                       "App Task UserIF",               /* 任务名 */
                       AppTaskUserIF,                   /* 启动任务函数地址 */
                       0,                               /* 传递给任务的参数 */
                       &AppTaskUserIFStk[0],            /* 堆栈基地址 */
                       APP_CFG_TASK_USER_IF_STK_SIZE,   /* 堆栈空间大小 */
                       APP_CFG_TASK_USER_IF_PRIO,       /* 任务优先级*/
                       APP_CFG_TASK_USER_IF_PRIO,       /* 任务抢占阀值 */
                       TX_NO_TIME_SLICE,                /* 不开启时间片 */
                       TX_AUTO_START);                  /* 创建后立即启动 */

    /**************创建COM任务*********************/
    tx_thread_create(&AppTaskCOMTCB,                    /* 任务控制块地址 */
                       "App Task COM",                  /* 任务名 */
                       AppTaskCOM,                      /* 启动任务函数地址 */
                       0,                               /* 传递给任务的参数 */
                       &AppTaskCOMStk[0],               /* 堆栈基地址 */
                       APP_CFG_TASK_COM_STK_SIZE,       /* 堆栈空间大小 */
                       APP_CFG_TASK_COM_PRIO,           /* 任务优先级*/
                       APP_CFG_TASK_COM_PRIO,           /* 任务抢占阀值 */
                       TX_NO_TIME_SLICE,                /* 不开启时间片 */
                       TX_AUTO_START);                  /* 创建后立即启动 */
}

/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通讯
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
     /* 创建互斥信号量 */
    tx_mutex_create(&AppPrintfSemp, "AppPrintfSemp", TX_NO_INHERIT);
}

/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  App_Printf(const char *fmt, ...)
{
    char  buf_str[200 + 1]; /* 特别注意，如果printf的变量较多，注意此局部变量的大小是否够用 */
    va_list   v_args;


    va_start(v_args, fmt);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) fmt,
                                  v_args);
    va_end(v_args);

    /* 互斥操作 */
    tx_mutex_get(&AppPrintfSemp, TX_WAIT_FOREVER);

    printf("%s", buf_str);

    tx_mutex_put(&AppPrintfSemp);
}

/*
*********************************************************************************************************
*	函 数 名: DispTaskInfo
*	功能说明: 将 ThreadX 任务信息通过串口打印出来
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispTaskInfo(void)
{
    TX_THREAD      *p_tcb;	        /* 定义一个任务控制块指针 */

    p_tcb = &AppTaskStartTCB;

    /* 打印标题 */
    App_Printf("===============================================================\r\n");
    App_Printf("CPU利用率 = %5.2f%%\r\n", OSCPUUsage);
    App_Printf("任务执行时间 = %.9fs\r\n", (double)_tx_execution_thread_time_total/SystemCoreClock);
    App_Printf("空闲执行时间 = %.9fs\r\n", (double)_tx_execution_idle_time_total/SystemCoreClock);
    App_Printf("中断执行时间 = %.9fs\r\n", (double)_tx_execution_isr_time_total/SystemCoreClock);
    App_Printf("系统总执行时间 = %.9fs\r\n", (double)(_tx_execution_thread_time_total + \
                                                       _tx_execution_idle_time_total +  \
                                                       _tx_execution_isr_time_total)/SystemCoreClock);

    App_Printf("===============================================================\r\n");
    App_Printf(" 任务优先级 任务栈大小 当前使用栈  最大栈使用   任务名\r\n");
    App_Printf("   Prio     StackSize   CurStack    MaxStack   Taskname\r\n");

    /* 遍历任务控制块列表（TCB list)，打印所有的任务的优先级和名称 */
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

