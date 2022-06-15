/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file main.c
 **
 ** \brief ADC sample
 **
 **   - 2021-04-16  CDT First version for Device Driver Library of
 **     ADC
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <board_config.h>
#include <rtthread.h>
#include "arm_math.h"
#include <rtdevice.h>
#include "renode.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/*
 * If you remap the mapping between the channel and the pin with the function
 * ADC_ChannelRemap, define ADC_CH_REMAP as non-zero, otherwise define as 0.
 */

#define ADC_CH_REMAP (0u)

/* ADC clock selection definition. */
#define ADC_CLK_PCLK (1u)
#define ADC_CLK_MPLLQ (2u)
#define ADC_CLK_UPLLR (3u)

/* Select UPLLR as ADC clock. */
#define ADC_CLK (ADC_CLK_UPLLR)

/* ADC1 channel definition for this example. */
#define ADC1_SA_NORMAL_CHANNEL (ADC1_CH0 | ADC1_CH1 | ADC1_CH4 | ADC1_CH5 | ADC1_CH6 | ADC1_CH7 | ADC1_CH10 | ADC1_CH11 | ADC1_CH12 | ADC1_CH13)
#define ADC1_SA_CHANNEL (ADC1_SA_NORMAL_CHANNEL)
#define ADC1_SA_CHANNEL_COUNT (10u)

#define ADC1_CHANNEL (ADC1_SA_CHANNEL)
#define ADC1_CHANNEL_COUNT (ADC1_SA_CHANNEL_COUNT)

#define Ia 6
#define Iah 7
#define Ib 5  //4
#define Ibh 4 //5
#define Ic 0
#define Ich 1

#define Ua 12
#define Ub 11
#define Uc 10
#define Izh 13

#ifndef oursiden

#define icenter_00 2076
#define icenter_01 2076
#define icenter_10 2076
#define icenter_11 2076
#define icenter_20 2076
#define icenter_21 2076
#define icenter_30 2076
#define icenter_31 2076

#define ucenter_0 2076
#define ucenter_1 2076
#define ucenter_2 2076

#define iratio_00 25414
#define iratio_01 2490
#define iratio_10 25414
#define iratio_11 2490
#define iratio_20 25414
#define iratio_21 2490
#define iratio_30 25414
#define iratio_31 2490

#define uratio_0 406414
#define uratio_1 406414
#define uratio_2 406414

#define pratio_00 481
#define pratio_01 47
#define pratio_10 481
#define pratio_11 47
#define pratio_20 481
#define pratio_21 47


#endif


#define _GETICENTER(n, m) icenter_##n##m
#define GETICENTER(n, m) _GETICENTER(n, m)

#define _GETIRATIO(n, m) iratio_##n##m
#define GETIRATIO(n, m) _GETIRATIO(n, m)

#define _GETPRATIO(n, m) pratio_##n##m
#define GETPRATIO(n, m) _GETPRATIO(n, m)

#define _GETUCENTER(n) ucenter_##n
#define GETUCENTER(n) _GETUCENTER(n)

#define _GETURATIO(n) uratio_##n
#define GETURATIO(n) _GETURATIO(n)
#define highsign_0 useiah
#define highsign_1 useibh
#define highsign_2 useich

#define _GETHIGHSIGN(n) highsign_##n
#define GETHIGHSIGN(n) _GETHIGHSIGN(n)


#define CALVAL(i, ch, chv, h)                               \
    get_vol(i,ch, &so[i], chv, &so[i + 4]);                   \
    GETHIGHSIGN(i)=h;                                             \
    ivalue[i * 16 + (m % 16)] = so[i] / GETIRATIO(i, h);    \
    uvalue[i * 16 + (m % 16)] = (so[i + 4]) / GETURATIO(i); \
    arm_offset_q15(k[ch], -GETICENTER(i, h), ttvv[0], 16);  \
    arm_offset_q15(k[chv], -GETUCENTER(i), ttvv[1], 16);    \
    arm_dot_prod_q15(ttvv[0], ttvv[1], 16, &p_result);      \
    pvalue[i * 16 + (m % 16)] = (int32_t)p_result / GETPRATIO(i, h);\
    tt[2 + i * 2]=tt[2 + i * 2]/ GETIRATIO(i, h);\
    tt[2 + i * 2+1]=tt[2 + i * 2+1]/ GETIRATIO(i, h);\

/* ADC1 channel sampling time.      ADC1_CH0  ADC1_CH1 */
#define ADC1_SA_CHANNEL_SAMPLE_TIME                                \
    {                                                              \
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 \
    }

/* ADC1 channel sampling time.      ADC1_CH4   ADC1_CH5   ADC1_CH6 */
#define ADC1_SB_CHANNEL_SAMPLE_TIME \
    {                               \
        0x50, 0x60, 0x45            \
    }

/* ADC2 channel definition for this example. */
#define ADC2_SA_NORMAL_CHANNEL (ADC2_CH0)
#define ADC2_SA_AVG_CHANNEL (ADC2_CH5)
#define ADC2_SA_CHANNEL (ADC2_SA_NORMAL_CHANNEL | ADC2_SA_AVG_CHANNEL)
#define ADC2_SA_CHANNEL_COUNT (2u)

#define ADC2_SB_NORMAL_CHANNEL (ADC2_CH2 | ADC2_CH3)
#define ADC2_SB_CHANNEL (ADC2_SB_NORMAL_CHANNEL)
#define ADC2_SB_CHANNEL_COUNT (2u)

#define ADC2_AVG_CHANNEL (ADC2_SA_AVG_CHANNEL)
#define ADC2_CHANNEL (ADC2_SA_CHANNEL | ADC2_SB_CHANNEL)

/* ADC2 channel sampling time.     ADC2_CH0  ADC2_CH5 */
#define ADC2_SA_CHANNEL_SAMPLE_TIME \
    {                               \
        0x60, 0x50                  \
    }

/* ADC2 channel sampling time.     ADC2_CH2  ADC2_CH3 */
#define ADC2_SB_CHANNEL_SAMPLE_TIME \
    {                               \
        0x60, 0x50                  \
    }

/* Timer definition for this example. */
#define TMR_UNIT (M4_TMR02)

/* DMA definition for ADC1. */
#define ADC1_SA_DMA_UNIT (M4_DMA1)
#define ADC1_SA_DMA_CH (DmaCh1)
#define ADC1_SA_DMA_PWC (PWC_FCG0_PERIPH_DMA1)
#define ADC1_SA_DMA_TRGSRC (EVT_ADC1_EOCA)
#define ADC1_SA_DMA_IRQ_NUM (INT_DMA1_BTC1)
#define ADC1_SA_DMA_INT_CB (Dma1Btc0_IrqHandler)

#define ADC1_SB_DMA_UNIT (M4_DMA1)
#define ADC1_SB_DMA_CH (DmaCh3)
#define ADC1_SB_DMA_PWC (PWC_FCG0_PERIPH_DMA1)
#define ADC1_SB_DMA_TRGSRC (EVT_ADC1_EOCB)
#define ADC1_SB_DMA_IRQ_NUM (INT_DMA1_BTC1)
#define ADC1_SB_DMA_INT_CB (Dma1Btc1_IrqHandler)

/* DMA definition for ADC2. */
#define ADC2_SA_DMA_UNIT (M4_DMA1)
#define ADC2_SA_DMA_CH (DmaCh2)
#define ADC2_SA_DMA_PWC (PWC_FCG0_PERIPH_DMA1)
#define ADC2_SA_DMA_TRGSRC (EVT_ADC2_EOCA)
#define ADC2_SA_DMA_IRQ_NUM (INT_DMA1_BTC2)
#define ADC2_SA_DMA_INT_CB (Dma1Btc2_IrqHandler)

#define ADC2_SB_DMA_UNIT (M4_DMA1)
#define ADC2_SB_DMA_CH (DmaCh3)
#define ADC2_SB_DMA_PWC (PWC_FCG0_PERIPH_DMA1)
#define ADC2_SB_DMA_TRGSRC (EVT_ADC2_EOCB)
#define ADC2_SB_DMA_IRQ_NUM (INT_DMA1_BTC3)
#define ADC2_SB_DMA_INT_CB (Dma1Btc3_IrqHandler)

/* ADC interrupt flag bit mask definition. */
#define ADC1_SA_DMA_IRQ_BIT (1ul << 0u)
#define ADC1_SB_DMA_IRQ_BIT (1ul << 1u)
#define ADC2_SA_DMA_IRQ_BIT (1ul << 2u)
#define ADC2_SB_DMA_IRQ_BIT (1ul << 3u)

#define ADC_EVENT_READY (1L << 0)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
// q15_t sint[16] = {0,   6392,  12539,  18204,  23170,  27245,  30273,  32138,  32767,  32138,  30273,  27245,  23170,  18204,  12539,   6392};
// q15_t cost[16] = {32767,32138,30274,27246,23170,18205,12540,6393,0,-6393,-12540,-18205,-23170,-27246,-30274,-32138};

q15_t sint[8] = {0, 12539, 23170, 30273, 32767, 30273, 23170, 12539};
q15_t cost[8] = {32767, 30274, 23170, 12540, 0, -12540, -23170, -30274};
volatile extern uint32_t senddata[64];
volatile extern int data_ready;
volatile extern uint16_t control_modifings;
rt_event_t adc_event = RT_NULL;
volatile extern uint32_t cmp[16];
void protect_trigger(uint16_t lineset);
volatile extern int refresh_data;
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
void AdcConfig(void);
static void AdcClockConfig(void);
static void AdcInitConfig(void);
static void AdcChannelConfig(void);
static void AdcTriggerConfig(void);

void DmaConfig(void);
static void DmaInitConfig(void);
static void DmaIrqConfig(void);
static void DmaIrqRegister(stc_irq_regi_conf_t *pstcCfg, uint32_t u32Priority);

void TimerConfig(void);

static void AdcSetChannelPinMode(const M4_ADC_TypeDef *ADCx,
                                 uint32_t u32Channel,
                                 en_pin_mode_t enMode);
static void AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
uint16_t m_au16Adc1SaValue[ADC1_CH_COUNT];
static uint16_t m_au16Adc1SbValue[ADC1_CH_COUNT];
static uint16_t m_au16Adc2SaValue[ADC2_CH_COUNT];
static uint16_t m_au16Adc2SbValue[ADC2_CH_COUNT];

uint32_t m_u32AdcDmaIrqFlag = 0u;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  Main function
 **
 ** \param  None
 **
 ** \retval int32_t return value, if needed
 **
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  ADC configuration, including clock configuration, initial configuration,
 **         channel configuration and trigger source configuration.
 **
 ** \param  None.
 **
 ** \retval None.
 **
 ******************************************************************************/
void AdcConfig(void)
{
    AdcClockConfig();
    AdcInitConfig();
    AdcChannelConfig();
    AdcTriggerConfig();
}

/**
 *******************************************************************************
 ** \brief  ADC clock configuration.
 **
 ** \note   1) ADCLK max frequency is 60MHz.
 **         2) If PCLK2 and PCLK4 are selected as the ADC clock,
 **            the following conditions must be met:
 **            a. ADCLK(PCLK2) max 60MHz;
 **            b. PCLK4 : ADCLK = 1:1, 2:1, 4:1, 8:1, 1:2, 1:4
 **
 ******************************************************************************/
static void AdcClockConfig(void)
{
#if (ADC_CLK == ADC_CLK_PCLK)
    stc_clk_sysclk_cfg_t stcSysclkCfg;

    /* Set bus clock division, depends on the system clock frequency. */
    stcSysclkCfg.enHclkDiv = ClkSysclkDiv1;  // 200MHz
    stcSysclkCfg.enExclkDiv = ClkSysclkDiv2; // 100MHz
    stcSysclkCfg.enPclk0Div = ClkSysclkDiv1; // 200MHz
    stcSysclkCfg.enPclk1Div = ClkSysclkDiv2; // 100MHz
    stcSysclkCfg.enPclk2Div = ClkSysclkDiv4; // 50MHz
    stcSysclkCfg.enPclk3Div = ClkSysclkDiv4; // 50MHz
    stcSysclkCfg.enPclk4Div = ClkSysclkDiv1; // 100MHz.
    CLK_SysClkConfig(&stcSysclkCfg);
    CLK_SetPeriClkSource(ClkPeriSrcPclk);

#elif (ADC_CLK == ADC_CLK_MPLLQ)
    stc_clk_xtal_cfg_t stcXtalCfg;
    stc_clk_mpll_cfg_t stcMpllCfg;

    if (CLKSysSrcMPLL == CLK_GetSysClkSource())
    {
        /*
         * Configure MPLLQ(same as MPLLP and MPLLR) when you
         * configure MPLL as the system clock.
         */
    }
    else
    {
        /* Use XTAL as MPLL source. */
        stcXtalCfg.enFastStartup = Enable;
        stcXtalCfg.enMode = ClkXtalModeOsc;
        stcXtalCfg.enDrv = ClkXtalLowDrv;
        CLK_XtalConfig(&stcXtalCfg);
        CLK_XtalCmd(Enable);

        /* Set MPLL out 240MHz. */
        stcMpllCfg.pllmDiv = 1u;
        /* mpll = 8M / pllmDiv * plln */
        stcMpllCfg.plln = 30u;
        stcMpllCfg.PllpDiv = 16u;
        stcMpllCfg.PllqDiv = 16u;
        stcMpllCfg.PllrDiv = 16u;
        CLK_SetPllSource(ClkPllSrcXTAL);
        CLK_MpllConfig(&stcMpllCfg);
        CLK_MpllCmd(Enable);
    }
    CLK_SetPeriClkSource(ClkPeriSrcMpllp);

#elif (ADC_CLK == ADC_CLK_UPLLR)
    stc_clk_xtal_cfg_t stcXtalCfg;
    stc_clk_upll_cfg_t stcUpllCfg;

    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcUpllCfg);

    /* Use XTAL as UPLL source. */
    stcXtalCfg.enFastStartup = Enable;
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* Set UPLL out 240MHz. */
    stcUpllCfg.pllmDiv = 2u;
    /* upll = 8M(XTAL) / pllmDiv * plln */
    stcUpllCfg.plln = 60u;
    stcUpllCfg.PllpDiv = 16u;
    stcUpllCfg.PllqDiv = 16u;
    stcUpllCfg.PllrDiv = 16u;
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_UpllConfig(&stcUpllCfg);
    CLK_UpllCmd(Enable);
    while (Set != CLK_GetFlagStatus(ClkFlagUPLLRdy))
    {
        ;
    }

    CLK_SetPeriClkSource(ClkPeriSrcUpllr);
#endif
}

/**
 *******************************************************************************
 ** \brief  ADC initial configuration.
 **
 ******************************************************************************/
static void AdcInitConfig(void)
{
    stc_adc_init_t stcAdcInit;

    MEM_ZERO_STRUCT(stcAdcInit);

    stcAdcInit.enResolution = AdcResolution_12Bit;
    stcAdcInit.enDataAlign = AdcDataAlign_Right;
    stcAdcInit.enAutoClear = AdcClren_Enable;
    stcAdcInit.enScanMode = AdcMode_SAOnce;
    /* 1. Enable ADC1. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC1, Enable);
    /* 2. Initialize ADC1. */
    ADC_Init(M4_ADC1, &stcAdcInit);
}

/**
 *******************************************************************************
 ** \brief  ADC channel configuration.
 **
 ******************************************************************************/
static void AdcChannelConfig(void)
{
    stc_adc_ch_cfg_t stcChCfg;
    uint8_t au8Adc1SaSampTime[ADC1_SA_CHANNEL_COUNT] = ADC1_SA_CHANNEL_SAMPLE_TIME;

    MEM_ZERO_STRUCT(stcChCfg);

    /**************************** Add ADC1 channels ****************************/
    /* 1. Set the ADC pin to analog mode. */
    AdcSetChannelPinMode(M4_ADC1, ADC1_CHANNEL, Pin_Mode_Ana);

    stcChCfg.u32Channel = ADC1_SA_CHANNEL;
    stcChCfg.u8Sequence = ADC_SEQ_A;
    stcChCfg.pu8SampTime = au8Adc1SaSampTime;
    /* 2. Add ADC channel. */
    ADC_AddAdcChannel(M4_ADC1, &stcChCfg);
}

/**
 *******************************************************************************
 ** \brief  ADC trigger source configuration.
 **
 ******************************************************************************/
static void AdcTriggerConfig(void)
{
    stc_adc_trg_cfg_t stcTrgCfg;

    MEM_ZERO_STRUCT(stcTrgCfg);

    /*
     * If select an event(@ref en_event_src_t) to trigger ADC,
     * AOS must be enabled first.
     */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

    /* Select EVT_TMR02_GCMA as ADC1 sequence A trigger source. */
    stcTrgCfg.u8Sequence = ADC_SEQ_A;
    stcTrgCfg.enTrgSel = AdcTrgsel_TRGX0;
    stcTrgCfg.enInTrg0 = EVT_TMR02_GCMA;
    ADC_ConfigTriggerSrc(M4_ADC1, &stcTrgCfg);
    ADC_TriggerSrcCmd(M4_ADC1, ADC_SEQ_A, Enable);
}

/**
 *******************************************************************************
 ** \brief  DMA configuration for ADC1 and ADC2, including initial configuration
 **         and interrupt configuration.
 **
 ******************************************************************************/
void DmaConfig(void)
{
    DmaInitConfig();
    DmaIrqConfig();
}

/**
 *******************************************************************************
 ** \brief  DMA initial configuration.
 **
 ******************************************************************************/
static void DmaInitConfig(void)
{
    stc_dma_config_t stcDmaCfg;

    MEM_ZERO_STRUCT(stcDmaCfg);

    stcDmaCfg.u16BlockSize = ADC1_CH_COUNT;
    stcDmaCfg.u16TransferCnt = 0u;
    stcDmaCfg.u32SrcAddr = (uint32_t)(&M4_ADC1->DR0);
    stcDmaCfg.u32DesAddr = (uint32_t)(&m_au16Adc1SaValue[0]);
    stcDmaCfg.u16DesRptSize = ADC1_CH_COUNT;
    stcDmaCfg.u16SrcRptSize = ADC1_CH_COUNT;
    stcDmaCfg.u32DmaLlp = 0u;
    stcDmaCfg.stcSrcNseqCfg.u16Cnt = 0u;
    stcDmaCfg.stcSrcNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDesNseqCfg.u16Cnt = 0u;
    stcDmaCfg.stcDesNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDmaChCfg.enSrcInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enDesInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enSrcRptEn = Enable;
    stcDmaCfg.stcDmaChCfg.enDesRptEn = Enable;
    stcDmaCfg.stcDmaChCfg.enSrcNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enDesNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enTrnWidth = Dma16Bit;
    stcDmaCfg.stcDmaChCfg.enLlpEn = Disable;
    /* Enable DMA interrupt. */
    stcDmaCfg.stcDmaChCfg.enIntEn = Enable;

    PWC_Fcg0PeriphClockCmd(ADC1_SA_DMA_PWC, Enable);
    DMA_InitChannel(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, &stcDmaCfg);
    DMA_Cmd(ADC1_SA_DMA_UNIT, Enable);
    DMA_ClearIrqFlag(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, TrnCpltIrq);
    /* AOS must be enabled to use DMA */
    /* AOS enabled at first. */
    /* If you have enabled AOS before, then the following statement is not needed. */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

    DMA_SetTriggerSrc(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, ADC1_SA_DMA_TRGSRC);
    DMA_ChannelCmd(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, Enable);
}

/**
 *******************************************************************************
 ** \brief  DMA interrupt configuration.
 **
 ** \note   DMA NVIC number: [Int000_IRQn, Int031_IRQn]
 **                          [Int038_IRQn, Int043_IRQn]
 **                          [Int129_IRQn]
 **
 ******************************************************************************/
static void DmaIrqConfig(void)
{
    stc_irq_regi_conf_t stcAdcIrqCfg;

    stcAdcIrqCfg.enIntSrc = ADC1_SA_DMA_IRQ_NUM;
    stcAdcIrqCfg.enIRQn = Int029_IRQn;
    stcAdcIrqCfg.pfnCallback = &ADC1_SA_DMA_INT_CB;
    DmaIrqRegister(&stcAdcIrqCfg, DDL_IRQ_PRIORITY_03);
}

/**
 *******************************************************************************
 ** \brief  DMA IRQ register function.
 **
 ******************************************************************************/
static void DmaIrqRegister(stc_irq_regi_conf_t *pstcCfg, uint32_t u32Priority)
{
    int16_t s16Vnum = pstcCfg->enIRQn;

    if (((s16Vnum >= Int000_IRQn) && (s16Vnum <= Int031_IRQn)) ||
        ((s16Vnum >= Int038_IRQn) && (s16Vnum <= Int043_IRQn)))
    {
        if (Ok != enIrqRegistration(pstcCfg))
        {
            return;
        }
    }
    else if (Int129_IRQn == s16Vnum)
    {
        enShareIrqEnable(pstcCfg->enIntSrc);
    }
    else
    {
        return;
    }
    NVIC_ClearPendingIRQ(pstcCfg->enIRQn);
    NVIC_SetPriority(pstcCfg->enIRQn, u32Priority);
    NVIC_EnableIRQ(pstcCfg->enIRQn);
}

/**
 *******************************************************************************
 ** \brief  Timer configuration, for generating event EVT_TMR02_GCMA every second.
 **
 ** \param  None.
 **
 ** \retval None.
 **
 ******************************************************************************/
void TimerConfig(void)
{
    stc_tim0_base_init_t stcTimerCfg;
    stc_clk_freq_t stcClkTmp;
    uint32_t u32Pclk1;

    MEM_ZERO_STRUCT(stcTimerCfg);
    /* Get PCLK1. */
    CLK_GetClockFreq(&stcClkTmp);
    u32Pclk1 = stcClkTmp.pclk1Freq;

    /* Timer0 peripheral enable. */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable);
    /* Config register for channel A. */
    stcTimerCfg.Tim0_CounterMode = Tim0_Sync;
    stcTimerCfg.Tim0_SyncClockSource = Tim0_Pclk1;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv512;
    /* Tim0_CmpValue's type is uint16_t!!! Be careful!!! */
    stcTimerCfg.Tim0_CmpValue = (uint16_t)(((u32Pclk1 / 512u) / 16) / 50 - 1u);
    TIMER0_BaseInit(TMR_UNIT, Tim0_ChannelA, &stcTimerCfg);

    /* Start timer0. */
    TIMER0_Cmd(TMR_UNIT, Tim0_ChannelA, Enable);
}
void testTimerConfig(void)
{
    stc_tim0_base_init_t stcTimerCfg;
    stc_clk_freq_t stcClkTmp;
    uint32_t u32Pclk1;

    MEM_ZERO_STRUCT(stcTimerCfg);
    /* Get PCLK1. */
    CLK_GetClockFreq(&stcClkTmp);
    u32Pclk1 = stcClkTmp.pclk1Freq;

    /* Timer0 peripheral enable. */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable);
    /* Config register for channel A. */
    stcTimerCfg.Tim0_CounterMode = Tim0_Sync;
    stcTimerCfg.Tim0_SyncClockSource = Tim0_Pclk1;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv1024;

    /* Start timer0. */
}
void testTimerStart(void)
{
    TIMER0_Cmd(TMR_UNIT, Tim0_ChannelB, Enable);
}
void testTimerStop(void)
{
    TIMER0_Cmd(TMR_UNIT, Tim0_ChannelB, Disable);
}
/**
 *******************************************************************************
 ** \brief  Config the pin which is mapping the channel to analog or digit mode.
 **
 ******************************************************************************/
static void AdcSetChannelPinMode(const M4_ADC_TypeDef *ADCx,
                                 uint32_t u32Channel,
                                 en_pin_mode_t enMode)
{
    uint8_t u8ChIndex;
#if (ADC_CH_REMAP)
    uint8_t u8AdcPin;
#else
    uint8_t u8ChOffset = 0u;
#endif

    if (M4_ADC1 == ADCx)
    {
        u32Channel &= ADC1_PIN_MASK_ALL;
    }
    else
    {
        u32Channel &= ADC2_PIN_MASK_ALL;
#if (!ADC_CH_REMAP)
        u8ChOffset = 4u;
#endif
    }

    u8ChIndex = 0u;
    while (0u != u32Channel)
    {
        if (u32Channel & 0x1ul)
        {
#if (ADC_CH_REMAP)
            u8AdcPin = ADC_GetChannelPinNum(ADCx, u8ChIndex);
            AdcSetPinMode(u8AdcPin, enMode);
#else
            AdcSetPinMode((u8ChIndex + u8ChOffset), enMode);
#endif
        }

        u32Channel >>= 1u;
        u8ChIndex++;
    }
}

/**
 *******************************************************************************
 ** \brief  Set an ADC pin as analog input mode or digit mode.
 **
 ******************************************************************************/
static void AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode)
{
    en_port_t enPort = PortA;
    en_pin_t enPin = Pin00;
    bool bFlag = true;
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = enMode;
    stcPortInit.enPullUp = Disable;

    switch (u8AdcPin)
    {
    case ADC1_IN0:
        enPort = PortA;
        enPin = Pin00;
        break;

    case ADC1_IN1:
        enPort = PortA;
        enPin = Pin01;
        break;

    case ADC1_IN2:
        enPort = PortA;
        enPin = Pin02;
        break;

    case ADC1_IN3:
        enPort = PortA;
        enPin = Pin03;
        break;

    case ADC12_IN4:
        enPort = PortA;
        enPin = Pin04;
        break;

    case ADC12_IN5:
        enPort = PortA;
        enPin = Pin05;
        break;

    case ADC12_IN6:
        enPort = PortA;
        enPin = Pin06;
        break;

    case ADC12_IN7:
        enPort = PortA;
        enPin = Pin07;
        break;

    case ADC12_IN8:
        enPort = PortB;
        enPin = Pin00;
        break;

    case ADC12_IN9:
        enPort = PortB;
        enPin = Pin01;
        break;

    case ADC12_IN10:
        enPort = PortC;
        enPin = Pin00;
        break;

    case ADC12_IN11:
        enPort = PortC;
        enPin = Pin01;
        break;

    case ADC1_IN12:
        enPort = PortC;
        enPin = Pin02;
        break;

    case ADC1_IN13:
        enPort = PortC;
        enPin = Pin03;
        break;

    case ADC1_IN14:
        enPort = PortC;
        enPin = Pin04;
        break;

    case ADC1_IN15:
        enPort = PortC;
        enPin = Pin05;
        break;

    default:
        bFlag = false;
        break;
    }

    if (true == bFlag)
    {
        PORT_Init(enPort, enPin, &stcPortInit);
    }
}

/**
 *******************************************************************************
 ** \brief DMA IRQ callbacks.
 **
 ******************************************************************************/
int sample_num = 0;
void Dma1Btc0_IrqHandler(void)
{
    #ifdef RENODE
        rt_event_send(adc_event, ADC_EVENT_READY);
    #else
    sample_num++;
    DMA_ClearIrqFlag(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, BlkTrnCpltIrq);
    if (sample_num == 16)
    {
        rt_event_send(adc_event, ADC_EVENT_READY);
        sample_num = 0;
    }
    #endif

}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
int32_t adc_on(void)
{
    /* Default clock is MRC(8MHz). */

    /* Config ADCs. */
    AdcConfig();

    /* Config DMA. */
    DmaConfig();

    /*
     * Config timer0.
     * Timer0 generates event EVT_TMR02_GCMA every second.
     * This event will trigger ADC sequence A to start conversion.
     */
    TimerConfig();

    /***************** Configuration end, application start **************/

    // while (1u)
    // {
    //     /* Check ADC1 SA. */
    //         if (m_u32AdcDmaIrqFlag & ADC1_SA_DMA_IRQ_BIT)
    //     {
    //         m_u32AdcDmaIrqFlag &= ~ADC1_SA_DMA_IRQ_BIT;
    //         rt_kprintf("get");
    //         // Do something with m_au16Adc1SaValue.
    //     }
    //     rt_kprintf("ds");
    //     rt_thread_mdelay(100);

    // }
    while (1)
    {

        /* ADC1 sequence A will be triggered by timer0 every second. */
        if (m_u32AdcDmaIrqFlag & ADC1_SA_DMA_IRQ_BIT)
        {
            m_u32AdcDmaIrqFlag &= ~ADC1_SA_DMA_IRQ_BIT;
            rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ", m_au16Adc1SaValue[0], m_au16Adc1SaValue[1], m_au16Adc1SaValue[2], m_au16Adc1SaValue[3], m_au16Adc1SaValue[4], m_au16Adc1SaValue[5], m_au16Adc1SaValue[6], m_au16Adc1SaValue[7], m_au16Adc1SaValue[8], m_au16Adc1SaValue[9], m_au16Adc1SaValue[10], m_au16Adc1SaValue[11], m_au16Adc1SaValue[12], m_au16Adc1SaValue[13], m_au16Adc1SaValue[14], m_au16Adc1SaValue[15], m_au16Adc1SaValue[16]);
            rt_thread_mdelay(100);
        }
    }
}
MSH_CMD_EXPORT(adc_on, an adc sample);
int16_t a[10];
int16_t k[17][16], ttvv[2][16];
static stc_dma_llp_descriptor_t llp_desc[16];
uint32_t ivalue[16 * 4] = {0}, uvalue[16 * 3] = {0};
int32_t pvalue[16 * 3] = {0};
float tenergy[3] = {0}, energy[3] = {0};
static void DmatestInitConfig(void)
{
    stc_dma_config_t stcDmaCfg;

    MEM_ZERO_STRUCT(stcDmaCfg);

    stcDmaCfg.u16BlockSize = ADC1_CH_COUNT;
    stcDmaCfg.u16TransferCnt = 1u;
    stcDmaCfg.u32SrcAddr = (uint32_t)(&M4_ADC1->DR0);
    stcDmaCfg.u32DesAddr = k[0];
    stcDmaCfg.u32DmaLlp = 0u;
    stcDmaCfg.stcSrcNseqCfg.u16Cnt = 0u;
    stcDmaCfg.stcSrcNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDmaChCfg.enSrcInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enDesInc = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enSrcRptEn = Disable;
    stcDmaCfg.stcDmaChCfg.enDesRptEn = Disable;
    stcDmaCfg.stcDmaChCfg.enSrcNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enDesNseqEn = Enable;
    stcDmaCfg.stcDmaChCfg.enTrnWidth = Dma16Bit;
    stcDmaCfg.stcDmaChCfg.enLlpEn = Enable;
    stcDmaCfg.stcDesNseqCfg.u32Offset = 16u;
    stcDmaCfg.stcDesNseqCfg.u16Cnt = 1u;

    /* Enable DMA interrupt. */
    stcDmaCfg.stcDmaChCfg.enIntEn = Enable;
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DMA1, Enable);
    DMA_InitChannel(M4_DMA1, ADC1_SA_DMA_CH, &stcDmaCfg);
    stc_dma_llp_init_t llp_init;
    llp_init.u32LlpEn = Enable;
    llp_init.u32LlpRun = LlpWaitNextReq;
    llp_init.u32LlpAddr = (uint32_t)&llp_desc[1];
    DMA_LlpInit(M4_DMA1, ADC1_SA_DMA_CH, &llp_init);
    for (size_t i = 0; i < 16; i++)
    {
        llp_desc[i].SARx = (uint32_t)(&M4_ADC1->DR0);
        llp_desc[i].DARx = &k[0][i];
        llp_desc[i].DTCTLx = (((uint32_t)stcDmaCfg.u16TransferCnt) << DMA_DTCTL_CNT_Pos) | (stcDmaCfg.u16BlockSize << DMA_DTCTL_BLKSIZE_Pos);
        llp_desc[i].LLPx = i == 15 ? (uint32_t)&llp_desc[0] : (uint32_t)&llp_desc[i + 1];
        llp_desc[i].CHxCTL = (stcDmaCfg.stcDmaChCfg.enSrcInc | ((uint32_t)stcDmaCfg.stcDmaChCfg.enDesInc) << (DMA_CHCTL_DINC_Pos) | ((uint32_t)stcDmaCfg.stcDmaChCfg.enTrnWidth) << (DMA_CHCTL_HSIZE_Pos) |
                              ((uint32_t)stcDmaCfg.stcDmaChCfg.enDesNseqEn) << DMA_CHCTL_DNSEQEN_Pos | ((uint32_t)llp_init.u32LlpEn) << DMA_CHCTL_LLPEN_Pos | ((uint32_t)llp_init.u32LlpRun) << DMA_CHCTL_LLPRUN_Pos | ((uint32_t)llp_init.u32LlpEn) << DMA_CHCTL_IE_Pos);
        llp_desc[i].DNSEQCTLx = (stcDmaCfg.stcDesNseqCfg.u32Offset | ((uint32_t)stcDmaCfg.stcDesNseqCfg.u16Cnt) << (DMA_DNSEQCTL_DNSCNT_Pos));
    }

    DMA_Cmd(ADC1_SA_DMA_UNIT, Enable);
    DMA_ClearIrqFlag(M4_DMA1, ADC1_SA_DMA_CH, TrnCpltIrq);
    /* AOS must be enabled to use DMA */
    /* AOS enabled at first. */
    /* If you have enabled AOS before, then the following statement is not needed. */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

    DMA_SetTriggerSrc(M4_DMA1, ADC1_SA_DMA_CH, ADC1_SA_DMA_TRGSRC);
    DMA_ChannelCmd(M4_DMA1, ADC1_SA_DMA_CH, Enable);
}
void DmatestConfig(void)
{
    #ifdef RENODE
    *(uint32_t*)adc=k;
    #else
    DmatestInitConfig();
    #endif
    DmaIrqConfig();
}
q15_t input[32], output[32], moutput[32];

void testtimer(void)
{
    rt_pin_mode(0x02, PIN_MODE_OUTPUT);
    int level = rt_hw_interrupt_disable();

    while (1)
    {
        rt_pin_write(0x02, PIN_LOW);

        rt_pin_write(0x02, PIN_HIGH);
        Ddl_Delay1us(1);
    }
    rt_hw_interrupt_enable(level);
}
MSH_CMD_EXPORT(testtimer, an adc sample);

q31_t tt[8];
int64_t s_result, c_result, p_result;
uint16_t m = 50 * 32 * 5 - 64;

__attribute__((always_inline)) static void get_vol(int i, int ch, q31_t *so, int chv, q31_t *sov)
{
    int tstamp = 0;
    arm_sub_q15(k[ch], k[ch] + 8, input, 8);
    arm_dot_prod_q15(input, sint, 8, &s_result);
    arm_dot_prod_q15(input, cost, 8, &c_result);
    tt[2 + i * 2] = (q31_t)(c_result >> 10);
    tt[2 + i * 2 + 1] = (q31_t)(s_result >> 10);
    if (tt[2 + i * 2] < (1 << 18) && tt[2 + i * 2 + 1] < (1 << 18))
    {
        tstamp = 1;
        tt[2 + i * 2] = tt[2 + i * 2] << 10;
        tt[2 + i * 2 + 1] = tt[2 + i * 2 + 1] << 10;
    }
    arm_cmplx_mag_q31(tt + 2 + 2 * i, so, 1);
    arm_sub_q15(k[chv], k[chv] + 8, input, 8);
    arm_dot_prod_q15(input, sint, 8, &s_result);
    arm_dot_prod_q15(input, cost, 8, &c_result);
    tt[0] = (q31_t)(c_result >> 10);
    tt[1] = (q31_t)(s_result >> 10);
    if (tt[0] < (1 << 18) && tt[1] < (1 << 18))
    {
        tt[0] = tt[0] << 10;
        tt[1] = tt[1] << 10;
        arm_cmplx_mag_q31(&tt[0], sov, 1);
    }
    else
    {
        arm_cmplx_mag_q31(&tt[0], sov, 1);
        *sov = (*sov) << 10;
    }
    // if (*so == 0 || *sov == 0)
    // {
    //     cospi[i * 16 + (m % 16)] = 0;
    // }
    // else
    // {
    //     float t1 = 1.0 * tt[0] / (*so) * tt[2] / (*sov);
    //     float t2 = 1.0 * tt[1] / (*so) * tt[3] / (*sov);
    //     cospi[i * 16 + (m % 16)] = (t1 + t2) / 4.0;
    // }
    if (tstamp == 0)
    {
        *so = (*so) << 10;
        tt[2 + i * 2] = (tt[2 + i * 2] << 5);
        tt[2 + i * 2 + 1] =(tt[2 + i * 2 + 1] << 5);
    }else{
        tt[2 + i * 2] = (tt[2 + i * 2] >> 5);
        tt[2 + i * 2 + 1] =(tt[2 + i * 2 + 1] >> 5);

    }
}
float p_vv[3] = {0};
void dma_test(void)
{
    adc_event = rt_event_create("adc", RT_IPC_FLAG_PRIO);
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    /* LED0 Port/Pin initialization */
    PORT_Init(PortC, Pin07, &stcPortInit);
    AdcConfig();

    /* Config DMA. */
    DmatestConfig();

    /*
     * Config timer0.
     * Timer0 generates event EVT_TMR02_GCMA every second.
     * This event will trigger ADC sequence A to start conversion.
     */
    TimerConfig();
    // testTimerConfig();
    q15_t dc[8] = {0};
    q31_t so[8] = {0};
    q31_t averageso[8] = {0};

    rt_uint32_t recved = 0;
    uint16_t protectc[8] = {0};
    uint16_t mode1c[8] = {0};
    uint16_t protectdata;
            uint32_t itemp[4];

    uint32_t lastcmpvalue[8];
    rt_event_recv(adc_event, ADC_EVENT_READY, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 0, &recved);
    //TODO Izh未检测
    while (1)
    {
        protectdata = 0;
        recved = 0;
        /* ADC1 sequence A will be triggered by timer0 every second. */
        rt_err_t event_result = rt_event_recv(adc_event, ADC_EVENT_READY, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &recved);
        if (recved & ADC_EVENT_READY)
        {
            m++;
            // m_u32AdcDmaIrqFlag &= ~ADC1_SA_DMA_IRQ_BIT;
            float32_t res[32];
            int useiah = 0, useibh = 0, useich = 0;
            // TIMER0_WriteCntReg(TMR_UNIT, Tim0_ChannelB, 0);
            // int level = rt_hw_interrupt_disable();
            //              rt_pin_write(0x02, PIN_LOW);
            // testTimerStart();
            // testTimerStop();
            // rt_hw_interrupt_enable(level);
            uint32_t index;
            q15_t imvar;
            PORT_SetBits(PortC, Pin07);
            rt_enter_critical();

            arm_max_q15(k[Ia], 16, &imvar, &index);
            if (imvar > 3300)
            {
                CALVAL(0, Iah, Ua, 1)
            }
            else
            {
                arm_min_q15(k[Ia], 16, &imvar, &index);
                if (imvar < 800)
                {
                    CALVAL(0, Iah, Ua, 1)
                }
                else
                {
                    CALVAL(0, Ia, Ua, 0)
                }
            }

            arm_max_q15(k[Ib], 16, &imvar, &index);
            if (imvar > 3300)
            {
                CALVAL(1, Ibh, Ub, 1)
            }
            else
            {
                arm_min_q15(k[Ib], 16, &imvar, &index);
                if (imvar < 800)
                {
                    CALVAL(1, Ibh, Ub, 1)
                }
                else
                {
                    CALVAL(1, Ib, Ub, 0)
                }
            }

            arm_max_q15(k[Ic], 16, &imvar, &index);
            if (imvar > 3300)
            {
                CALVAL(2, Ich, Uc, 1)
            }
            else
            {
                arm_min_q15(k[Ic], 16, &imvar, &index);
                if (imvar < 800)
                {
                    CALVAL(2, Ich, Uc, 1)
                }
                else
                {
                    CALVAL(2, Ic, Uc, 0)
                }
            }
            arm_sub_q15(k[Izh], k[Izh] + 8, input, 8);
            arm_dot_prod_q15(input, sint, 8, &s_result);
            arm_dot_prod_q15(input, cost, 8, &c_result);
            tt[0] = (q31_t)(c_result >> 10);
            tt[1] = (q31_t)(s_result >> 10);
            if (tt[0] < (1 << 18) && tt[1] < (1 << 18))
            {
                tt[0] = tt[0] << 10;
                tt[1] = tt[1] << 10;
                arm_cmplx_mag_q31(tt, &so[4], 1);
                ivalue[16 * 3 + (m % 16)] = so[4] / GETIRATIO(3,1);
            }
            else
            {
                arm_cmplx_mag_q31(tt, &so[4], 1);
                ivalue[16 * 3 + (m % 16)] = (so[4] << 10) / GETIRATIO(3,1);
            }

            itemp[0] = ivalue[(m % 16)];
            itemp[1] = ivalue[16 + (m % 16)];
            itemp[2] = ivalue[32 + (m % 16)];
            arm_max_q31(itemp, 3, &itemp[3], &index);
            if (cmp[0] != 0)
            {
                //TODO cmptime change detect
                if (lastcmpvalue[0] != cmp[0])
                {
                    protectc[0] = 0;
                }
                lastcmpvalue[0] = cmp[0];
                if (cmp[0] < itemp[3])
                {
                    if (cmp[1] <= protectc[0])
                    {
                        protectdata |= 1 << 0;
                        protectc[0] = 0;
                    }
                    protectc[0]++;
                }
                else
                {
                    protectc[0] = 0;
                }
            }
            else
            {
                protectc[0] = 0;
            }
            if (cmp[2] != 0)
            {
                //TODO cmptime change detect
                if (lastcmpvalue[1] != cmp[2])
                {
                    protectc[1] = 0;
                }
                lastcmpvalue[1] = cmp[2];
                if (cmp[2] < itemp[3])
                {
                    int tratio = (itemp[3] * 10 / cmp[+2]);
                    if (cmp[3] <= protectc[1])
                    {
                        protectdata |= (1 << 4) << 0;
                        protectc[1] = 0;
                    }
                    if (mode1c[0] >= 5000)
                    {
                        protectc[1] += (mode1c[0] / 5000);
                        mode1c[0] = 0;
                    }
                    mode1c[0] += tratio * tratio;
                }
                else
                {
                    protectc[1] = 0;
                }
            }
            else
            {
                protectc[1] = 0;
            }

            uint32_t tems[2] = {0}, ti = 0;
            tems[0] = (tt[2] + tt[4] + tt[6]) << 12;
            tems[1] = (tt[3] + tt[5] + tt[7]) << 12;
            arm_cmplx_mag_q31(tems, &ti, 1);
            ti=ti>>7;//因为出来已经小了2^5
            if (cmp[4] != 0)
            {
                //TODO cmptime change detect
                if (lastcmpvalue[2] != cmp[4])
                {
                    protectc[2] = 0;
                }
                lastcmpvalue[2] = cmp[4];
                if (cmp[4] < ti)
                {
                    if (cmp[5] <= protectc[2])
                    {
                        protectdata |= 1 << 1;
                        protectc[2] = 0;
                    }
                    protectc[2]++;
                }
                else
                {
                    protectc[2] = 0;
                }
            }
            else
            {
                protectc[2] = 0;
            }

            if (cmp[12] != 0)
            {
                //TODO cmptime change detect
                if (lastcmpvalue[6] != cmp[12])
                {
                    protectc[6] = 0;
                }
                lastcmpvalue[6] = cmp[12];
                if (cmp[12] < ivalue[16 * 3 + (m % 16)])
                {
                    if (cmp[13] <= protectc[6])
                    {
                        protectdata |= 1 << 3;
                        protectc[6] = 0;
                    }
                    protectc[6]++;
                }
                else
                {
                    protectc[6] = 0;
                }
            }
            else
            {
                protectc[6] = 0;
            }

            if (protectdata != 0)
            {
                protect_trigger(protectdata);
                protectdata = 0;
            }

            p_vv[0] = 0.99 * p_vv[0] + 0.001 * pvalue[(m % 16)];
            p_vv[1] = 0.99 * p_vv[1] + 0.001 * pvalue[16 + (m % 16)];
            p_vv[2] = 0.99 * p_vv[2] + 0.001 * pvalue[32 + (m % 16)];
            q31_t t_result = 0;
            if (m % 16 == 0)
            {
                arm_mean_q31(&pvalue[0], 16, &t_result);
                tenergy[0] += (0.1 * 0.02) * t_result;
                arm_mean_q31(&pvalue[16], 16, &t_result);
                tenergy[1] += (0.1 * 0.02) * t_result;
                arm_mean_q31(&pvalue[32], 16, &t_result);
                tenergy[2] += (0.1 * 0.02) * t_result;
            }
            if (m % 64 == 0)
            {
                arm_mean_q31(ivalue, 16, &senddata[1]);
                arm_mean_q31(ivalue + 16, 16, &senddata[2]);
                arm_mean_q31(ivalue + 16 * 2, 16, &senddata[3]);
                arm_mean_q31(ivalue + 16 * 3, 16, &senddata[4]);
                arm_mean_q31(uvalue, 16, &senddata[5]);
                arm_mean_q31(uvalue + 16, 16, &senddata[6]);
                arm_mean_q31(uvalue + 16 * 2, 16, &senddata[7]);
            }

            if (m == 50 * 32 * 5 || refresh_data == 1)
            {
                *(float *)&senddata[11] = p_vv[0];
                *(float *)&senddata[12] = p_vv[1];
                *(float *)&senddata[13] = p_vv[2];
                energy[0] += tenergy[0];
                energy[1] += tenergy[1];
                energy[2] += tenergy[2];
                tenergy[0] = 0;
                tenergy[1] = 0;
                tenergy[2] = 0;
                *(float *)&senddata[8] = energy[0];
                *(float *)&senddata[9] = energy[1];
                *(float *)&senddata[10] = energy[2];
                if (m % 16 == 0)
                {
                    m = 0;
                }
                if (refresh_data == 1)
                {
                    refresh_data = 0;
                }
                else
                {
                    data_ready = 1;
                }
            }
            rt_exit_critical();
            PORT_ResetBits(PortC, Pin07);
            // rt_pin_write(0x02, PIN_HIGH);
            // rt_kprintf("Time:%d ", TIMER0_GetCntReg(TMR_UNIT, Tim0_ChannelB));
            // rt_kprintf("s: %d，c： %d \n", s_result, c_result);
            if (m % (64) == 0)
            {
                 rt_kprintf("最大电流 %d \n",itemp[3]);
                 rt_kprintf("保护电流 %d \n",cmp[2]);

                rt_kprintf("零序 %d mA tt2,%d,tt3,%d,tt4,%d,tt5,%d,tt6,%d,tt7,%d\n", ti,tt[2],tt[3],tt[4],tt[5],tt[6],tt[7]);
                rt_kprintf("protectc:%d: \n", protectc[0]);
                rt_kprintf("A路功率：%d W\n", (int32_t)p_vv[0]);
                rt_kprintf("Ia: 交流： %d.%.2d A\n", ivalue[(m % 16)] / 1000, (ivalue[(m % 16)] % 1000) / 10);
                rt_kprintf("Ib: 交流： %d.%.2d A\n", ivalue[16 + (m % 16)] / 1000, (ivalue[16 + (m % 16)] % 1000) / 10);
                rt_kprintf("Ic: 交流： %d.%.2d A\n", ivalue[32 + (m % 16)] / 1000, (ivalue[32 + (m % 16)] % 1000) / 10);
                rt_kprintf("Id: 交流： %d.%.2d A\n", ivalue[48 + (m % 16)] / 1000, (ivalue[48 + (m % 16)] % 1000) / 10);
                // if (useiah == 0)
                // {
                // rt_kprintf("Ia: 一阶： %d.%.2d A\n", ((so[0]+19380)/2491)/1000,(((so[0]+19380)/2491)%1000)/10);
                // }
                // else
                // {
                //     rt_kprintf("Ia: 直流：%d，一阶： %d mA\n", dc[0], so[0]);
                // }
                // if (useibh == 0)
                // {
                //     rt_kprintf("Ib: 直流：%d，一阶： %d mA\n", dc[1], so[1]);
                // }
                // else
                // {
                //     rt_kprintf("Ib: 直流：%d，一阶： %d mA\n", dc[1], so[1]);
                // }
                // if (useich == 0)
                // {
                //     rt_kprintf("Ic: 直流：%d，一阶： %d mA\n", dc[2], so[2]);
                // }
                // else
                // {
                //     rt_kprintf("Ic: 直流：%d，一阶： %d mA\n", dc[2], so[2]);
                // }
                // rt_kprintf("Iz: 直流：%d，一阶： %d \n", dc[3], so[3] >> 12);
                rt_kprintf("Ua: 交流： %d \n", uvalue[(m % 16)] );
                rt_kprintf("Ub: 交流： %d \n", uvalue[16 + (m % 16)] );
                rt_kprintf("Uc: 交流： %d \n", uvalue[32 + (m % 16)]);
                // rt_kprintf("A: 有功系数： %ld \n", (int32_t)(cospi[0]*1000));
                // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n",*(uint32_t *)&cospi[0],*(uint32_t *)& cospi[1],*(uint32_t *)& cospi[2],*(uint32_t *)& cospi[3],*(uint32_t *)& cospi[4],*(uint32_t *)& cospi[5],*(uint32_t *)& cospi[6],*(uint32_t *)& cospi[7],*(uint32_t *)& cospi[8],*(uint32_t *)& cospi[9],*(uint32_t *)& cospi[10],*(uint32_t *)& cospi[11],*(uint32_t *)& cospi[12],*(uint32_t *)& cospi[13],*(uint32_t *)& cospi[14],*(uint32_t *)& cospi[15]);
                // rt_kprintf("A: 有功功率： %d \n", (int32_t)(cospi[0] * (ivalue[0] / 24905.0) * ((uvalue[0] + 36821.0) / 406414.0)));
                rt_kprintf("energe1: %d，\r\n", (int32_t)(energy[0] * 1000));
                rt_kprintf("cmp0: %d，cmp2: %d，cmp4: %d，\r\n", cmp[0], cmp[2], cmp[4]);
                // rt_kprintf("Uc: 直流：%d，一阶： %d \n", dc[6], so[6]);
                // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", k[Ia][0],k[Ia][1],k[Ia][2],k[Ia][3],k[Ia][4],k[Ia][5],k[Ia][6],k[Ia][7],k[Ia][8],k[Ia][9],k[Ia][10],k[Ia][11],k[Ia][12],k[Ia][13],k[Ia][14],k[Ia][15]);
                // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", pvalue[0], pvalue[1], pvalue[2], pvalue[3], pvalue[4], pvalue[5], pvalue[6], pvalue[7], pvalue[8], pvalue[9], pvalue[10], pvalue[11], pvalue[12], pvalue[13], pvalue[14], pvalue[15]);
                rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", ivalue[0], ivalue[1], ivalue[2], ivalue[3], ivalue[4], ivalue[5], ivalue[6], ivalue[7], ivalue[8], ivalue[9], ivalue[10], ivalue[11], ivalue[12], ivalue[13], ivalue[14], ivalue[15]);
                // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", k[4][0], k[4][1], k[4][2], k[4][3], k[4][4], k[4][5], k[4][6], k[4][7], k[4][8], k[4][9], k[4][10], k[4][11], k[4][12], k[4][13], k[4][14], k[4][15]);
	            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", k[5][0], k[5][1], k[5][2], k[5][3], k[5][4], k[5][5], k[5][6], k[5][7], k[5][8], k[5][9], k[5][10], k[5][11], k[5][12], k[5][13], k[5][14], k[5][15]);
                // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", uvalue[0], uvalue[1], uvalue[2], uvalue[3], uvalue[4], uvalue[5], uvalue[6], uvalue[7], uvalue[8], uvalue[9], uvalue[10], uvalue[11], uvalue[12], uvalue[13], uvalue[14], uvalue[15]);
                // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", k[Ibh][0], k[Ibh][1], k[Ibh][2], k[Ibh][3], k[Ibh][4], k[Ibh][5], k[Ibh][6], k[Ibh][7], k[Ibh][8], k[Ibh][9], k[Ibh][10], k[Ibh][11], k[Ibh][12], k[Ibh][13], k[Ibh][14], k[Ibh][15]);
                rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", k[Uc][0], k[Uc][1], k[Uc][2], k[Uc][3], k[Uc][4], k[Uc][5], k[Uc][6], k[Uc][7], k[Uc][8], k[Uc][9], k[Uc][10], k[Uc][11], k[Uc][12], k[Uc][13], k[Uc][14], k[Uc][15]);
                // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", k[Ic][0], k[Ic][1], k[Ic][2], k[Ic][3], k[Ic][4], k[Ic][5], k[Ic][6], k[Ic][7], k[Ic][8], k[Ic][9], k[Ic][10], k[Ic][11], k[Ic][12], k[Ic][13], k[Ic][14], k[Ic][15]);
                // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d \n", k[Ich][0], k[Ich][1], k[Ich][2], k[Ich][3], k[Ich][4], k[Ich][5], k[Ich][6], k[Ich][7], k[Ich][8], k[Ich][9], k[Ich][10], k[Ich][11], k[Ich][12], k[Ich][13], k[Ich][14], k[Ich][15]);
                rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", k[Izh][0], k[Izh][1], k[Izh][2], k[Izh][3], k[Izh][4], k[Izh][5], k[Izh][6], k[Izh][7], k[Izh][8], k[Izh][9], k[Izh][10], k[Izh][11], k[Izh][12], k[Izh][13], k[Izh][14], k[Izh][15]);
            } // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[1][0],k[1][1],k[1][4],k[1][5],k[1][6],k[1][7],k[1][10],k[1][11],k[1][12],k[1][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[4][0],k[0][1],k[0][4],k[0][5],k[0][6],k[0][7],k[0][10],k[0][11],k[0][12],k[0][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[5][0],k[1][1],k[1][4],k[1][5],k[1][6],k[1][7],k[1][10],k[1][11],k[1][12],k[1][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[0][0],k[0][1],k[0][4],k[0][5],k[0][6],k[0][7],k[0][10],k[0][11],k[0][12],k[0][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[1][0],k[1][1],k[1][4],k[1][5],k[1][6],k[1][7],k[1][10],k[1][11],k[1][12],k[1][13]);
        }
    }
}
MSH_CMD_EXPORT(dma_test, an adc sample);

void dma_test2(void)
{
    int m = 0;
    AdcConfig();

    /* Config DMA. */
    DmatestConfig();

    /*
     * Config timer0.
     * Timer0 generates event EVT_TMR02_GCMA every second.
     * This event will trigger ADC sequence A to start conversion.
     */
    TimerConfig();
    testTimerConfig();

    while (1)
    {
        /* ADC1 sequence A will be triggered by timer0 every second. */
        if (m_u32AdcDmaIrqFlag & ADC1_SA_DMA_IRQ_BIT)
        {
            if (m % (50 * 10) == 50 * 10 - 1)
            {
                m = 0;
            }
            m_u32AdcDmaIrqFlag &= ~ADC1_SA_DMA_IRQ_BIT;
            arm_rfft_instance_q15 S;
            float32_t res[32];
            // TIMER0_WriteCntReg(TMR_UNIT, Tim0_ChannelB,0);
            // int level = rt_hw_interrupt_disable();
            // testTimerStart();

            arm_shift_q15(k[12], 3, input, 32);

            arm_rfft_q15(&S, input, output);
            arm_shift_q15(moutput + 2, 1, moutput + 2, 30);
            arm_cmplx_mag_q15(output, moutput, 32);
            // testTimerStop();
            // rt_hw_interrupt_enable(level);

            rt_kprintf("Time:%d ", TIMER0_GetCntReg(TMR_UNIT, Tim0_ChannelB));
            arm_shift_q15(moutput, -3, moutput, 32);
            rt_kprintf("直流：%d，一阶： %d,二阶： %d,三阶： %d \n", moutput[0], moutput[1], moutput[2], moutput[3]);
            rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[4][0], k[4][1], k[4][3], k[4][4], k[4][5], k[4][6], k[4][7], k[4][8], k[4][9], k[4][10], k[4][11], k[4][12], k[4][13], k[4][14], k[4][15]);
	            rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[5][0], k[5][1], k[5][3], k[5][4], k[5][5], k[5][6], k[5][7], k[5][8], k[5][9], k[5][10], k[5][11], k[5][12], k[5][13], k[5][14], k[5][15]);

            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[0][0],k[0][1],k[0][4],k[0][5],k[0][6],k[0][7],k[0][10],k[0][11],k[0][12],k[0][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[1][0],k[1][1],k[1][4],k[1][5],k[1][6],k[1][7],k[1][10],k[1][11],k[1][12],k[1][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[4][0],k[0][1],k[0][4],k[0][5],k[0][6],k[0][7],k[0][10],k[0][11],k[0][12],k[0][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[5][0],k[1][1],k[1][4],k[1][5],k[1][6],k[1][7],k[1][10],k[1][11],k[1][12],k[1][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[0][0],k[0][1],k[0][4],k[0][5],k[0][6],k[0][7],k[0][10],k[0][11],k[0][12],k[0][13]);
            // rt_kprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", k[1][0],k[1][1],k[1][4],k[1][5],k[1][6],k[1][7],k[1][10],k[1][11],k[1][12],k[1][13]);
            m++;
        }
    }
}
MSH_CMD_EXPORT(dma_test2, an adc sample);
