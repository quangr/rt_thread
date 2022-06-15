/*
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-10-30     CDT          first version
 */
 


#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "hc32_ddl.h"
#include "drv_usart.h"

#define  LED0_PORT        (PortC)
#define  LED0_PIN         (Pin13)

#define  LED0_ON()    (PORT_SetBits(LED0_PORT, LED0_PIN))
#define  LED0_OFF()    (PORT_ResetBits(LED0_PORT, LED0_PIN))

/**
 * @addtogroup HC32
 */

/*@{*/

void BSP_CLK_Init(void)
{
    stc_clk_sysclk_cfg_t    stcSysClkCfg;
    stc_clk_xtal_cfg_t      stcXtalCfg;
    stc_clk_mpll_cfg_t      stcMpllCfg;
    stc_sram_config_t       stcSramConfig;

    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);
    MEM_ZERO_STRUCT(stcSramConfig);

    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv  = ClkSysclkDiv1;
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;
    CLK_SysClkConfig(&stcSysClkCfg);

    /* Config Xtal and Enable Xtal */
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    stcXtalCfg.enFastStartup = Enable;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* sram init include read/write wait cycle setting */
    stcSramConfig.u8SramIdx = Sram12Idx | Sram3Idx | SramHsIdx | SramRetIdx;
    stcSramConfig.enSramRC = SramCycle2;
    stcSramConfig.enSramWC = SramCycle2;
    SRAM_Init(&stcSramConfig);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_5);
    EFM_Lock();

    /* MPLL config (XTAL / pllmDiv * plln / PllpDiv = 200M). */
    stcMpllCfg.pllmDiv = 1ul;
    stcMpllCfg.plln    = 50ul;
    stcMpllCfg.PllpDiv = 2ul;
    stcMpllCfg.PllqDiv = 2ul;
    stcMpllCfg.PllrDiv = 2ul;
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_MpllConfig(&stcMpllCfg);

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);
    /* Wait MPLL ready. */
    while(Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
        ;
    }
    /* Switch driver ability */
    PWC_HS2HP();
    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);
}


void rt_hw_board_clock_init(void)
{
	BSP_CLK_Init();
	SysTick_Init(RT_TICK_PER_SECOND);
}


/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * This function will initialize HC32 board.
 */
void rt_hw_board_init()
{
    /* Unlock the protected registers. */
    //Peripheral_WE();
    stc_port_init_t stcPortInit;
    //      stc_port_init_t stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    M4_PORT->PCRC14&=0x7fff;
    M4_PORT->PCRC15&=0x7fff;
				PORT_SetFunc(PortC, Pin14, Func_Gpio, Disable);
				PORT_SetFunc(PortC, Pin15, Func_Gpio, Disable);

    /* LED0 Port/Pin initialization */
    PORT_Init(PortC, Pin13, &stcPortInit);
    PORT_Init(PortC, Pin14, &stcPortInit);
    PORT_Init(PortC, Pin15, &stcPortInit);
    PORT_Init(PortA, Pin12, &stcPortInit);
    //     PORT_SetBits(PortA, Pin12);
    // //TODO PC11 is broken
    //     PORT_SetBits(PortC, Pin13);
    //     PORT_SetBits(PortC, Pin14);
    //     PORT_SetBits(PortC, Pin15);
    /* Configure the System clock */
    rt_hw_board_clock_init();

  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
  #endif

    /* Configure the SysTick */
    //SysTick_Configuration();

#ifdef RT_USING_HEAP
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
}

void rt_hw_us_delay(rt_uint32_t us)
{
    uint32_t start, now, delta, reload, us_tick;
    start = SysTick->VAL;
    reload = SysTick->LOAD;
    us_tick = SystemCoreClock / 1000000UL;

    do{
        now = SysTick->VAL;
        delta = start > now ?  start - now : reload + start - now;
    }
    while(delta < us_tick * us);}
/*@}*/
