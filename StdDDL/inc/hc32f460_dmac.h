/*****************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file hc32f460_dmac.h
 **
 ** A detailed description is available at
 ** @link DmacGroup DMAC description @endlink
 **
 **   - 2018-11-18  CDT  First version for Device Driver Library of DMAC.
 **
 ******************************************************************************/
#ifndef __HC32F460_DMAC_H__
#define __HC32F460_DMAC_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_common.h"
#include "ddl_config.h"

#if (DDL_DMAC_ENABLE == DDL_ON)


/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 *******************************************************************************
 ** \defgroup DmacGroup Direct Memory Access Control(DMAC)
 **
 ******************************************************************************/
//@{

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/**
 *******************************************************************************
 ** \brief DMA Channel
 **
 ******************************************************************************/
typedef enum en_dma_channel
{
    DmaCh0                          = 0u,    ///< DMA channel 0
    DmaCh1                          = 1u,    ///< DMA channel 1
    DmaCh2                          = 2u,    ///< DMA channel 2
    DmaCh3                          = 3u,    ///< DMA channel 3
    DmaChMax                        = 4u     ///< DMA channel max
}en_dma_channel_t;

/**
 *******************************************************************************
 ** \brief DMA transfer data width
 **
 ******************************************************************************/
typedef enum en_dma_transfer_width
{
    Dma8Bit                         = 0u,    ///< 8 bit transfer via DMA
    Dma16Bit                        = 1u,    ///< 16 bit transfer via DMA
    Dma32Bit                        = 2u     ///< 32 bit transfer via DMA
}en_dma_transfer_width_t;

/**
 *******************************************************************************
 ** \brief DMA flag
 **
 ******************************************************************************/
typedef enum en_dma_flag
{
    DmaTransferComplete             = 0u,    ///< DMA transfer complete
    DmaBlockComplete                = 1u,    ///< DMA block transfer complete
    DmaTransferErr                  = 2u,    ///< DMA transfer error
    DmaReqErr                       = 3u,    ///< DMA transfer request error
    DmaFlagMax                      = 4u
}en_dma_flag_t;

/**
 *******************************************************************************
 ** \brief DMA address mode
 **
 ******************************************************************************/
typedef enum en_dma_address_mode
{
    AddressFix                      = 0u,    ///< Address fixed
    AddressIncrease                 = 1u,    ///< Address increased
    AddressDecrease                 = 2u,    ///< Address decreased
}en_dma_address_mode_t;

/**
 *******************************************************************************
 ** \brief DMA link list pointer mode
 **
 ******************************************************************************/
typedef enum en_dma_llp_mode
{
    LlpWaitNextReq                  = 0u,    ///< DMA trigger transfer after wait next request
    LlpRunNow                       = 1u,    ///< DMA trigger transfer now
}en_dma_llp_mode_t;

/**
 *******************************************************************************
 ** \brief DMA interrupt selection
 **
 ******************************************************************************/
typedef enum en_dma_irq_sel
{
    TrnErrIrq                       = 0u,    ///< Select DMA transfer error interrupt
    TrnReqErrIrq                    = 1u,    ///< Select DMA transfer req over error interrupt
    TrnCpltIrq                      = 2u,    ///< Select DMA transfer completion interrupt
    BlkTrnCpltIrq                   = 3u,    ///< Select DMA block completion interrupt
    DmaIrqSelMax                    = 4u
}en_dma_irq_sel_t;

/**
 *******************************************************************************
 ** \brief DMA re_config count mode
 **
 ******************************************************************************/
typedef enum en_dma_recfg_cnt_mode
{
    CntFix                          = 0u,    ///< Fix
    CntSrcAddr                      = 1u,    ///< Source address mode
    CntDesAddr                      = 2u,    ///< Destination address mode
}en_dma_recfg_cnt_mode_t;

/**
 *******************************************************************************
 ** \brief DMA re_config destination address mode
 **
 ******************************************************************************/
typedef enum en_dma_recfg_daddr_mode
{
    DaddrFix                        = 0u,    ///< Fix
    DaddrNseq                       = 1u,    ///< No_sequence address
    DaddrRep                        = 2u,    ///< Repeat address
}en_dma_recfg_daddr_mode_t;

/**
 *******************************************************************************
 ** \brief DMA re_config source address mode
 **
 ******************************************************************************/
typedef enum en_dma_recfg_saddr_mode
{
    SaddrFix                        = 0u,    ///< Fix
    SaddrNseq                       = 1u,    ///< No_sequence address
    SaddrRep                        = 2u,    ///< Repeat address
}en_dma_recfg_saddr_mode_t;

/**
 *******************************************************************************
 ** \brief DMA channel status
 **
 ******************************************************************************/
typedef enum en_dma_ch_flag
{
    DmaSta                          = 0u,   ///< DMA status.
    ReCfgSta                        = 1u,   ///< DMA re_configuration status.
    DmaCh0Sta                       = 2u,   ///< DMA channel 0 status.
    DmaCh1Sta                       = 3u,   ///< DMA channel 1 status.
    DmaCh2Sta                       = 4u,   ///< DMA channel 2 status.
    DmaCh3Sta                       = 5u,   ///< DMA channel 3 status.
}en_dma_ch_flag_t;

/**
 *******************************************************************************
 ** \brief  DMA common trigger source select
 **
 ******************************************************************************/
typedef enum en_dma_com_trigger
{
    DmaComTrigger_1   = 0x1,            ///< Select common trigger 1.
    DmaComTrigger_2   = 0x2,            ///< Select common trigger 2.
    DmaComTrigger_1_2 = 0x3,            ///< Select common trigger 1 and 2.
} en_dma_com_trigger_t;

/**
 *******************************************************************************
 ** \brief DMA llp descriptor
 **
 ******************************************************************************/
typedef struct stc_dma_llp_descriptor
{
    uint32_t SARx;                             ///< DMA source address register
    uint32_t DARx;                             ///< DMA destination address register
    union
    {
        uint32_t DTCTLx;
        stc_dma_dtctl_field_t DTCTLx_f;        ///< DMA data control register
    };
    union
    {
        uint32_t RPTx;
        stc_dma_rpt_field_t RPTx_f;            ///< DMA repeat control register
    };
    union
    {
        uint32_t SNSEQCTLx;
        stc_dma_snseqctl_field_t SNSEQCTLx_f;  ///< DMA source no-sequence control register
    };
    union
    {
        __IO uint32_t DNSEQCTLx;
        stc_dma_dnseqctl_field_t DNSEQCTLx_f;  ///< DMA destination no-sequence control register
    };
    union
    {
        uint32_t LLPx;
        stc_dma_llp_field_t LLPx_f;            ///< DMA link-list-pointer register
    };
    union
    {
        uint32_t CHxCTL;
        stc_dma_ch0ctl_field_t CHxCTL_f;       ///< DMA channel control register
    };
}stc_dma_llp_descriptor_t;

/**
 *******************************************************************************
 ** \brief DMA no-sequence function configuration
 **
 ******************************************************************************/
typedef struct stc_dma_nseq_cfg
{
    uint32_t                u32Offset;      ///< DMA no-sequence offset.
    uint16_t                u16Cnt;         ///< DMA no-sequence count.
}stc_dma_nseq_cfg_t;

/**
 *******************************************************************************
 ** \brief DMA no-sequence function configuration
 **
 ******************************************************************************/
typedef struct stc_dma_nseqb_cfg
{
    uint32_t                u32NseqDist;    ///< DMA no-sequence district interval.
    uint16_t                u16CntB;        ///< DMA no-sequence count.
}stc_dma_nseqb_cfg_t;

/**
 *******************************************************************************
 ** \brief DMA re_config configuration
 **
 ******************************************************************************/
typedef struct stc_dma_recfg_ctl
{
    uint16_t                    u16SrcRptBSize; ///< The source repeat size.
    uint16_t                    u16DesRptBSize; ///< The destination repeat size.
    en_dma_recfg_saddr_mode_t   enSaddrMd;      ///< DMA re_config source address mode.
    en_dma_recfg_daddr_mode_t   enDaddrMd;      ///< DMA re_config destination address mode.
    en_dma_recfg_cnt_mode_t     enCntMd;        ///< DMA re_config count mode.
    stc_dma_nseq_cfg_t          stcSrcNseqBCfg; ///< The source no_sequence re_config.
    stc_dma_nseq_cfg_t          stcDesNseqBCfg; ///< The destination no_sequence re_config.
}stc_dma_recfg_ctl_t;

/**
 *******************************************************************************
 ** \brief DMA channel configuration
 **
 ******************************************************************************/
typedef struct stc_dma_ch_cfg
{
    en_dma_address_mode_t   enSrcInc;       ///< DMA source address update mode.
    en_dma_address_mode_t   enDesInc;       ///< DMA destination address update mode.
    en_functional_state_t   enSrcRptEn;     ///< Enable source repeat function or not.
    en_functional_state_t   enDesRptEn;     ///< Enable destination repeat function or not.
    en_functional_state_t   enSrcNseqEn;    ///< Enable source no_sequence function or not.
    en_functional_state_t   enDesNseqEn;    ///< Enable destination no_sequence function or not.
    en_dma_transfer_width_t enTrnWidth;     ///< DMA transfer data width.
    en_functional_state_t   enLlpEn;        ///< Enable linked list pointer function or not.
    en_dma_llp_mode_t       enLlpMd;        ///< Dma linked list pointer mode.
    en_functional_state_t   enIntEn;        ///< Enable interrupt function or not.
}stc_dma_ch_cfg_t;


/**
 *******************************************************************************
 ** \brief DMA configuration
 **
 ******************************************************************************/
typedef struct stc_dma_config
{
    uint16_t                u16BlockSize;       ///< Transfer block size = 1024, when 0 is set.
    uint16_t                u16TransferCnt;     ///< Transfer counter.
    uint32_t                u32SrcAddr;         ///< The source address.
    uint32_t                u32DesAddr;         ///< The destination address.
    uint16_t                u16SrcRptSize;      ///< The source repeat size.
    uint16_t                u16DesRptSize;      ///< The destination repeat size.
    uint32_t                u32DmaLlp;          ///< The Dma linked list pointer address
    stc_dma_nseq_cfg_t      stcSrcNseqCfg;      ///< The source no_sequence configuration.
    stc_dma_nseq_cfg_t      stcDesNseqCfg;      ///< The destination no_sequence configuration.
    stc_dma_ch_cfg_t        stcDmaChCfg;        ///< The Dma channel configuration.
}stc_dma_config_t;

/**
 * @brief  Dma LLP(linked list pointer) descriptor structure definition
 */

typedef struct
{
    uint32_t u32LlpEn;          /*!< Specifies the DMA LLP function.
                                    This parameter can be a value of @ref DMA_Llp_En */

    uint32_t u32LlpRun;         /*!< Specifies the DMA LLP auto or wait REQ.
                                    This parameter can be a value of @ref DMA_Llp_Mode */

    uint32_t u32LlpAddr;        /*!< Specifies the DMA list pointer address for LLP function. */

} stc_dma_llp_init_t;

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define DMA_CNT                             (10u)
#define DMA_IDLE                            (0u)
#define DMA_BUSY                            (1u)

#define DMACH0                              (0x01u)
#define DMACH1                              (0x02u)
#define DMACH2                              (0x04u)
#define DMACH3                              (0x08u)

#define DMATIMEOUT1                         (0x5000u)
#define DMATIMEOUT2                         (0x1000u)

#define DMA_CHCTL_DEFAULT                   (0x00001000ul)
#define DMA_DTCTL_DEFAULT                   (0x00000001ul)
#define DMA_DAR_DEFAULT                     (0x00000000ul)
#define DMA_SAR_DEFAULT                     (0x00000000ul)
#define DMA_RPT_DEFAULT                     (0x00000000ul)
#define DMA_LLP_DEFAULT                     (0x00000000ul)
#define DMA_SNSEQCTL_DEFAULT                (0x00000000ul)
#define DMA_DNSEQCTL_DEFAULT                (0x00000000ul)
#define DMA_RCFGCTL_DEFAULT                 (0x00000000ul)

/*****************  Bits definition for DMA_INTSTAT0 register  ****************/
#define DMA_INTSTAT0_TRNERR_Pos             (0U)                                /*!< DMA_INTSTAT0: TRNERR Position */
#define DMA_INTSTAT0_REQERR_Pos             (16U)                               /*!< DMA_INTSTAT0: REQERR Position */

/*****************  Bits definition for DMA_INTSTAT1 register  ****************/
#define DMA_INTSTAT1_TC_Pos                 (0U)                                /*!< DMA_INTSTAT1: TC Position */
#define DMA_INTSTAT1_BTC_Pos                (16U)                               /*!< DMA_INTSTAT1: BTC Position */

/*****************  Bits definition for DMA_INTMASK0 register  ****************/
#define DMA_INTMASK0_MSKTRNERR_Pos          (0U)                                /*!< DMA_INTMASK0: MSKTRNERR Position */
#define DMA_INTMASK0_MSKREQERR_Pos          (16U)                               /*!< DMA_INTMASK0: MSKREQERR Position */

/*****************  Bits definition for DMA_INTMASK1 register  ****************/
#define DMA_INTMASK1_MSKTC_Pos              (0U)                                /*!< DMA_INTMASK1: MSKTC Position */
#define DMA_INTMASK1_MSKBTC_Pos             (16U)                               /*!< DMA_INTMASK1: MSKBTC Position */

/*****************  Bits definition for DMA_INTCLR0 register  *****************/
#define DMA_INTCLR0_CLRTRNERR_Pos           (0U)                                /*!< DMA_INTCLR0: CLRTRNERR Position */
#define DMA_INTCLR0_CLRREQERR_Pos           (16U)                               /*!< DMA_INTCLR0: CLRREQERR Position */

/*****************  Bits definition for DMA_INTCLR1 register  *****************/
#define DMA_INTCLR1_CLRTC_Pos               (0U)                                /*!< DMA_INTCLR1: CLRTC Position */
#define DMA_INTCLR1_CLRBTC_Pos              (16U)                               /*!< DMA_INTCLR1: CLRBTC Position */

/*******************  Bits definition for DMA_CHEN register  ******************/
#define DMA_CHEN_CHEN_Pos                   (0U)                                /*!< DMA_CHEN: CHEN Position */

/**************  Bits definition for DMA_DTCTLx(x=0~3) register  **************/
#define DMA_DTCTL_BLKSIZE_Pos               (0ul)                                /*!< DMA_DTCTLx: BLKSIZE Position */
#define DMA_DTCTL_BLKSIZE_Msk               (0x3FFul << DMA_DTCTL_BLKSIZE_Pos)   /*!< DMA_DTCTLx: BLKSIZE Mask 0x000003FF */
#define DMA_DTCTL_BLKSIZE                   (DMA_DTCTL_BLKSIZE_Msk)

#define DMA_DTCTL_CNT_Pos                   (16ul)                               /*!< DMA_DTCTLx: CNT Position */
#define DMA_DTCTL_CNT_Msk                   (0xFFFFul << DMA_DTCTL_CNT_Pos)      /*!< DMA_DTCTLx: CNT Mask 0xFFFF0000 */
#define DMA_DTCTL_CNT                       (DMA_DTCTL_CNT_Msk)

/***************  Bits definition for DMA_RPTx(x=0~3) register  ***************/
#define DMA_RPT_SRPT_Pos                    (0ul)                                /*!< DMA_RPTx: SRPT Position */
#define DMA_RPT_SRPT_Msk                    (0x3FFul << DMA_RPT_SRPT_Pos)        /*!< DMA_RPTx: SRPT Mask 0x000003FF */
#define DMA_RPT_SRPT                        (DMA_RPT_SRPT_Msk)

#define DMA_RPT_DRPT_Pos                    (16ul)                               /*!< DMA_RPTx: DRPT Position */
#define DMA_RPT_DRPT_Msk                    (0x3FFul << DMA_RPT_DRPT_Pos)        /*!< DMA_RPTx: DRPT Mask 0x03FF0000 */
#define DMA_RPT_DRPT                        (DMA_RPT_DRPT_Msk)

/***************  Bits definition for DMA_RPTBx(x=0~3) register  ***************/
#define DMA_RPTB_SRPTB_Pos                  (0ul)                                /*!< DMA_RPTBx: SRPTB Position */
#define DMA_RPTB_SRPTB_Msk                  (0x3FFul << DMA_RPTB_SRPTB_Pos)      /*!< DMA_RPTBx: SRPTB Mask 0x000003FF */
#define DMA_RPTB_SRPTB                      (DMA_RPTB_SRPTB_Msk)

#define DMA_RPTB_DRPTB_Pos                  (16ul)                               /*!< DMA_RPTBx: DRPTB Position */
#define DMA_RPTB_DRPTB_Msk                  (0x3FFul << DMA_RPTB_DRPTB_Pos)      /*!< DMA_RPTBx: DRPTB Mask 0x03FF0000 */
#define DMA_RPTB_DRPTB                      (DMA_RPTB_DRPTB_Msk)

/*************  Bits definition for DMA_SNSEQCTLx(x=0~3) register  ************/
#define DMA_SNSEQCTL_SOFFSET_Pos            (0ul)                                /*!< DMA_SNSEQCTLx: SOFFSET Position */
#define DMA_SNSEQCTL_SOFFSET_Msk            (0xFFFFFul << DMA_SNSEQCTL_SOFFSET_Pos)      /*!< DMA_SNSEQCTLx: SOFFSET Mask 0x000FFFFF */
#define DMA_SNSEQCTL_SOFFSET                (DMA_SNSEQCTL_SOFFSET_Msk)

#define DMA_SNSEQCTL_SNSCNT_Pos             (20ul)                               /*!< DMA_SNSEQCTLx: SNSCNT Position */
#define DMA_SNSEQCTL_SNSCNT_Msk             (0xFFFul << DMA_SNSEQCTL_SNSCNT_Pos)         /*!< DMA_SNSEQCTLx: SNSCNT Mask 0xFFF00000 */
#define DMA_SNSEQCTL_SNSCNT                 (DMA_SNSEQCTL_SNSCNT_Msk)

/*************  Bits definition for DMA_SNSEQCTLBx(x=0~3) register  ************/
#define DMA_SNSEQCTLB_SNSDIST_Pos           (0ul)                                /*!< DMA_SNSEQCTLBx: SNSDIST Position */
#define DMA_SNSEQCTLB_SNSDIST_Msk           (0xFFFFFul << DMA_SNSEQCTLB_SNSDIST_Pos)     /*!< DMA_SNSEQCTLBx: SNSDIST Mask 0x000FFFFF */
#define DMA_SNSEQCTLB_SNSDIST               (DMA_SNSEQCTLB_SNSDIST_Msk)

#define DMA_SNSEQCTLB_SNSCNTB_Pos           (20ul)                               /*!< DMA_SNSEQCTLBx: SNSCNTB Position */
#define DMA_SNSEQCTLB_SNSCNTB_Msk           (0xFFFul << DMA_SNSEQCTLB_SNSCNTB_Pos)       /*!< DMA_SNSEQCTLBx: SNSCNTB Mask 0xFFF00000 */
#define DMA_SNSEQCTLB_SNSCNTB               (DMA_SNSEQCTLB_SNSCNTB_Msk)

/*************  Bits definition for DMA_DNSEQCTLx(x=0~3) register  ************/
#define DMA_DNSEQCTL_DOFFSET_Pos            (0ul)                                /*!< DMA_DNSEQCTLx: DOFFSET Position */
#define DMA_DNSEQCTL_DOFFSET_Msk            (0xFFFFFul << DMA_DNSEQCTL_DOFFSET_Pos)      /*!< DMA_DNSEQCTLx: DOFFSET Mask 0x000FFFFF */
#define DMA_DNSEQCTL_DOFFSET                (DMA_DNSEQCTL_DOFFSET_Msk)

#define DMA_DNSEQCTL_DNSCNT_Pos             (20ul)                               /*!< DMA_DNSEQCTLx: DNSCNT Position */
#define DMA_DNSEQCTL_DNSCNT_Msk             (0xFFFul << DMA_DNSEQCTL_DNSCNT_Pos)         /*!< DMA_DNSEQCTLx: DNSCNT Mask 0xFFF00000 */
#define DMA_DNSEQCTL_DNSCNT                 (DMA_DNSEQCTL_DNSCNT_Msk)

/*************  Bits definition for DMA_DNSEQCTLx(x=0~3) register  ************/
#define DMA_DNSEQCTLB_DNSDIST_Pos           (0ul)                                /*!< DMA_DNSEQCTLBx: DNSDIST Position */
#define DMA_DNSEQCTLB_DNSDIST_Msk           (0xFFFFFul << DMA_DNSEQCTLB_DNSDIST_Pos)      /*!< DMA_DNSEQCTLBx: DNSDIST Mask 0x000FFFFF */
#define DMA_DNSEQCTLB_DNSDIST               (DMA_DNSEQCTLB_DNSDIST_Msk)

#define DMA_DNSEQCTLB_DNSCNTB_Pos           (20ul)                               /*!< DMA_DNSEQCTLBx: DNSCNTB Position */
#define DMA_DNSEQCTLB_DNSCNTB_Msk           (0xFFFul << DMA_DNSEQCTLB_DNSCNTB_Pos)         /*!< DMA_DNSEQCTLBx: DNSCNTB Mask 0xFFF00000 */
#define DMA_DNSEQCTLB_DNSCNTB               (DMA_DNSEQCTLB_DNSCNTB_Msk)

/****************  Bits definition for DMA_LLPx(x=0~7) register  **************/
#define DMA_LLP_LLP_Pos                     (2ul)                                /*!< DMA_LLPx: LLP Position */
#define DMA_LLP_LLP_Msk                     (0x3FFFFFFFul << DMA_LLP_LLP_Pos)    /*!< DMA_LLPx: LLP Mask 0xFFFFFFC */
#define DMA_LLP_LLP                         (DMA_LLP_LLP_Msk)

/***************  Bits definition for DMA_CHxCTL(x=0~3) register  *************/
#define DMA_CHCTL_SINC_Pos                  (0ul)                                /*!< DMA_CHxCTL: SINC Position */
#define DMA_CHCTL_SINC_Msk                  (0x3ul << DMA_CHCTL_SINC_Pos)        /*!< DMA_CHxCTL: SINC Mask 0x00000003 */
#define DMA_CHCTL_SINC                      (DMA_CHCTL_SINC_Msk)

#define DMA_CHCTL_DINC_Pos                  (2ul)                                /*!< DMA_CHxCTL: DINC Position */
#define DMA_CHCTL_DINC_Msk                  (0x3ul << DMA_CHCTL_DINC_Pos)        /*!< DMA_CHxCTL: DINC Mask 0x0000000C */
#define DMA_CHCTL_DINC                      (DMA_CHCTL_DINC_Msk)

#define DMA_CHCTL_SRPTEN_Pos                (4ul)                                /*!< DMA_CHxCTL: SRPTEN Position */
#define DMA_CHCTL_SRPTEN_Msk                (0x1ul << DMA_CHCTL_SRPTEN_Pos)      /*!< DMA_CHxCTL: SRPTEN Mask 0x00000010 */
#define DMA_CHCTL_SRPTEN                    (DMA_CHCTL_SRPTEN_Msk)

#define DMA_CHCTL_DRPTEN_Pos                (5ul)                                /*!< DMA_CHxCTL: DRPTEN Position */
#define DMA_CHCTL_DRPTEN_Msk                (0x1ul << DMA_CHCTL_DRPTEN_Pos)      /*!< DMA_CHxCTL: DRPTEN Mask 0x00000020 */
#define DMA_CHCTL_DRPTEN                    (DMA_CHCTL_DRPTEN_Msk)

#define DMA_CHCTL_SNSEQEN_Pos               (6ul)                                /*!< DMA_CHxCTL: SNSEQEN Position */
#define DMA_CHCTL_SNSEQEN_Msk               (0x1ul << DMA_CHCTL_SNSEQEN_Pos)      /*!< DMA_CHxCTL: SNSEQEN Mask 0x00000040 */
#define DMA_CHCTL_SNSEQEN                   (DMA_CHCTL_SNSEQEN_Msk)

#define DMA_CHCTL_DNSEQEN_Pos               (7ul)                                /*!< DMA_CHxCTL: DNSEQEN Position */
#define DMA_CHCTL_DNSEQEN_Msk               (0x1ul << DMA_CHCTL_DNSEQEN_Pos)      /*!< DMA_CHxCTL: DNSEQEN Mask 0x00000080 */
#define DMA_CHCTL_DNSEQEN                   (DMA_CHCTL_DNSEQEN_Msk)

#define DMA_CHCTL_HSIZE_Pos                 (8ul)                                /*!< DMA_CHxCTL: HSIZE Position */
#define DMA_CHCTL_HSIZE_Msk                 (0x3ul << DMA_CHCTL_HSIZE_Pos)       /*!< DMA_CHxCTL: HSIZE Mask 0x00000300 */
#define DMA_CHCTL_HSIZE                     (DMA_CHCTL_HSIZE_Msk)

#define DMA_CHCTL_LLPEN_Pos                 (10ul)                               /*!< DMA_CHxCTL: LLPEN Position */
#define DMA_CHCTL_LLPEN_Msk                 (0x1ul << DMA_CHCTL_LLPEN_Pos)       /*!< DMA_CHxCTL: LLPEN Mask 0x00000400 */
#define DMA_CHCTL_LLPEN                     (DMA_CHCTL_LLPEN_Msk)

#define DMA_CHCTL_LLPRUN_Pos                (11ul)                               /*!< DMA_CHxCTL: LLPRUN Position */
#define DMA_CHCTL_LLPRUN_Msk                (0x1ul << DMA_CHCTL_LLPRUN_Pos)      /*!< DMA_CHxCTL: LLPRUN Mask 0x00000800 */
#define DMA_CHCTL_LLPRUN                    (DMA_CHCTL_LLPRUN_Msk)

#define DMA_CHCTL_IE_Pos                    (12ul)                               /*!< DMA_CHxCTL: IE Position */
#define DMA_CHCTL_IE_Msk                    (0x1ul << DMA_CHCTL_IE_Pos)          /*!< DMA_CHxCTL: IE Mask 0x00001000 */
#define DMA_CHCTL_IE                        (DMA_CHCTL_IE_Msk)

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
void DMA_Cmd(M4_DMA_TypeDef* pstcDmaReg, en_functional_state_t enNewState);
en_result_t DMA_EnableIrq(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, en_dma_irq_sel_t enIrqSel);
en_result_t DMA_DisableIrq(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, en_dma_irq_sel_t enIrqSel);
en_flag_status_t DMA_GetIrqFlag(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, en_dma_irq_sel_t enIrqSel);
en_result_t DMA_ClearIrqFlag(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, en_dma_irq_sel_t enIrqSel);
en_result_t DMA_ChannelCmd(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, en_functional_state_t enNewState);
void DMA_InitReConfig(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, const stc_dma_recfg_ctl_t* pstcDmaReCfg);
void DMA_ReCfgCmd(M4_DMA_TypeDef* pstcDmaReg,en_functional_state_t enNewState);
en_flag_status_t DMA_GetChFlag(M4_DMA_TypeDef* pstcDmaReg, en_dma_ch_flag_t enDmaChFlag);

en_result_t DMA_SetSrcAddress(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint32_t u32Address);
en_result_t DMA_SetDesAddress(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint32_t u32Address);
en_result_t DMA_SetBlockSize(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint16_t u16BlkSize);
en_result_t DMA_SetTransferCnt(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint16_t u16TrnCnt);
en_result_t DMA_SetSrcRptSize(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint16_t u16Size);
en_result_t DMA_SetDesRptSize(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint16_t u16Size);
en_result_t DMA_SetSrcRptbSize(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint16_t u16Size);
en_result_t DMA_SetDesRptbSize(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint16_t u16Size);
en_result_t DMA_SetSrcNseqCfg(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, const stc_dma_nseq_cfg_t* pstcSrcNseqCfg);
en_result_t DMA_SetSrcNseqBCfg(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, const stc_dma_nseqb_cfg_t* pstcSrcNseqBCfg);
en_result_t DMA_SetDesNseqCfg(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, const stc_dma_nseq_cfg_t* pstDesNseqCfg);
en_result_t DMA_SetDesNseqBCfg(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, const stc_dma_nseqb_cfg_t* pstDesNseqBCfg);
en_result_t DMA_SetLLP(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, uint32_t u32Pointer);
en_result_t DMA_LlpInit(M4_DMA_TypeDef *DMAx, uint8_t u8Ch, const stc_dma_llp_init_t *pstcDmaLlpInit);

void DMA_SetTriggerSrc(const M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, en_event_src_t enSrc);
void DMA_SetReConfigTriggerSrc(en_event_src_t enSrc);
void DMA_ComTriggerCmd(M4_DMA_TypeDef* pstcDmaReg,  uint8_t u8Ch, en_dma_com_trigger_t enComTrigger, en_functional_state_t enNewState);
void DMA_ReConfigComTriggerCmd(en_dma_com_trigger_t enComTrigger, en_functional_state_t enNewState);

void DMA_ChannelCfg(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, const stc_dma_ch_cfg_t* pstcChCfg);
void DMA_InitChannel(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch, const stc_dma_config_t* pstcDmaCfg);
void DMA_DeInit(M4_DMA_TypeDef* pstcDmaReg, uint8_t u8Ch);



//@} // DmacGroup

#ifdef __cplusplus
}
#endif

#endif /* DDL_DMAC_ENABLE */

#endif /* __HC32F460_DMAC_H__*/

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
