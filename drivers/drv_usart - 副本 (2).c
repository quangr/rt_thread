/*
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-10-30     CDT          first version
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <rtdevice.h>
#include <rthw.h>

#include "drv_usart.h"
#include "board_config.h"
#include "drv_irq.h"
#include "drv_dma.h"
#include "hc32_ddl.h"

#ifdef RT_USING_SERIAL

#if !defined(BSP_USING_UART1) && !defined(BSP_USING_UART2) && !defined(BSP_USING_UART3) && \
    !defined(BSP_USING_UART4)
#error "Please define at least one BSP_USING_UARTx"
/* UART instance can be selected at menuconfig -> Hardware Drivers Config -> On-chip Peripheral Drivers -> Enable UART */
#endif

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
/* HC32 config Rx timeout */
struct hc32_uart_rxto
{
    M4_TMR0_TypeDef *TMR0_Instance;
    rt_uint32_t     channel;

    rt_size_t       timeout_bits;

    struct hc32_irq_config irq_config;
};

/* HC32 config uart class */
struct hc32_uart_config
{
    struct hc32_irq_config  rxerr_irq_config;

    struct hc32_irq_config  rx_irq_config;

    struct hc32_irq_config  tx_irq_config;

#ifdef RT_SERIAL_USING_DMA
    struct hc32_uart_rxto   *rx_timeout;
    struct dma_config       *dma_rx;

    struct dma_config       *dma_tx;
#endif
};

/* HC32 UART index */
struct uart_index
{
    rt_uint32_t      index;
    M4_USART_TypeDef *Instance;
};

/* HC32 UART irq handler */
struct uart_irq_handler
{
    void (*rxerr_irq_handler)(void);
    void (*rx_irq_handler)(void);
    void (*tx_irq_handler)(void);
    void (*tc_irq_handler)(void);
    void (*rxto_irq_handler)(void);
    void (*dma_rx_irq_handler)(void);
};

/* HC32 uart dirver class */
struct hc32_uart
{
    struct rt_serial_device serial;

    const char *name;

    M4_USART_TypeDef *Instance;

    struct hc32_uart_config config;

#ifdef RT_SERIAL_USING_DMA
    rt_size_t   dma_rx_last_index;
#endif

    rt_uint16_t uart_dma_flag;
};

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#ifndef UART_CONFIG
#define UART_CONFIG(uart_name, USART)                                          \
    {                                                                          \
        .name = uart_name,                                                     \
        .Instance = M4_##USART,                                                \
         .config = {                                                           \
            .rxerr_irq_config = {                                              \
                .irq = USART##_RXERR_INT_IRQn,                                 \
                .irq_prio = USART##_RXERR_INT_PRIO,                            \
                .int_src = INT_##USART##_EI,                                   \
            },                                                                 \
            .rx_irq_config = {                                                 \
                .irq = USART##_RX_INT_IRQn,                                    \
                .irq_prio = USART##_RX_INT_PRIO,                               \
                .int_src = INT_##USART##_RI,                                   \
            },                                                                 \
            .tx_irq_config = {                                                 \
                .irq = USART##_TX_INT_IRQn,                                    \
                .irq_prio = USART##_TX_INT_PRIO,                               \
                .int_src = INT_##USART##_TI,                                   \
            },                                                                 \
         },                                                                    \
    }
#endif /* UART_CONFIG */

#ifndef UART_RXTO_CONFIG
#define UART_RXTO_CONFIG(USART)                                                \
    {                                                                          \
        .TMR0_Instance = USART##_RXTO_TMR0_UNIT,                               \
        .channel  = USART##_RXTO_TMR0_CH,                                      \
        .timeout_bits = 100UL,                                                  \
        .irq_config = {                                                        \
            .irq      = USART##_RXTO_INT_IRQn,                                 \
            .irq_prio = USART##_RXTO_INT_PRIO,                                 \
            .int_src  = INT_##USART##_RTO,                                     \
        }                                                                      \
    }
#endif /* UART_RXTO_CONFIG */

#ifndef UART_DMA_RX_CONFIG
#define UART_DMA_RX_CONFIG(USART)                                              \
    {                                                                          \
        .Instance = USART##_RX_DMA_UNIT,                                       \
        .channel  = USART##_RX_DMA_CH,                                         \
        .trigger_evt_src = EVT_##USART##_RI,                                   \
        .irq_config = {                                                        \
            .irq      = USART##_RX_DMA_INT_IRQn,                               \
            .irq_prio = USART##_RX_DMA_INT_PRIO,                               \
            .int_src  = USART##_RX_DMA_INT_SRC,                                \
        }                                                                      \
    }
#endif /* UART_DMA_RX_CONFIG */

#ifndef UART_DMA_TX_CONFIG
#define UART_DMA_TX_CONFIG(USART)                                              \
    {                                                                          \
        .Instance = USART##_TX_DMA_UNIT,                                       \
        .channel  = USART##_TX_DMA_CH,                                         \
        .trigger_evt_src = EVT_##USART##_TI,                                   \
        .irq_config = {                                                        \
            .irq      = USART##_TC_INT_IRQn,                                   \
            .irq_prio = USART##_TC_INT_PRIO,                                   \
            .int_src  = USART##_TC_DMA_INT_SRC,                                     \
        }                                                                      \
    }
#endif /* UART_DMA_TX_CONFIG */

#define READ_REG32(REG)                 (REG)

#define DMA_CH_REG(reg_base, ch)                                               \
    (*(uint32_t *)((uint32_t)(&(reg_base)) + ((ch) * 0x40UL)))

#define DMA_TRANS_CNT(unit, ch)                                                \
    (READ_REG32(DMA_CH_REG((unit)->MONDTCTL0, (ch))) >> DMA_DTCTL_CNT_Pos)

#define USART_TCI_ENABLE(unit)                                                 \
    SET_REG32_BIT(unit->CR1, USART_INT_TC)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
#ifdef RT_SERIAL_USING_DMA
static void hc32_dma_config(struct rt_serial_device *serial, rt_ubase_t flag);
#endif

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
enum
{
#ifdef BSP_USING_UART1
    UART1_INDEX,
#endif
#ifdef BSP_USING_UART2
    UART2_INDEX,
#endif
#ifdef BSP_USING_UART3
    UART3_INDEX,
#endif
#ifdef BSP_USING_UART4
    UART4_INDEX,
#endif
    UART_INDEX_MAX,
};

static const struct uart_index uart_map[] =
{
#ifdef BSP_USING_UART1
    {UART1_INDEX, M4_USART1},
#endif
#ifdef BSP_USING_UART2
    {UART2_INDEX, M4_USART2},
#endif
#ifdef BSP_USING_UART3
    {UART3_INDEX, M4_USART3},
#endif
#ifdef BSP_USING_UART4
    {UART4_INDEX, M4_USART4},
#endif
};

static struct hc32_uart uart_obj[] =
{
#ifdef BSP_USING_UART1
    UART_CONFIG("uart1", USART1),
#endif
#ifdef BSP_USING_UART2
    UART_CONFIG("uart2", USART2),
#endif
#ifdef BSP_USING_UART3
    UART_CONFIG("uart3", USART3),
#endif
#ifdef BSP_USING_UART4
    UART_CONFIG("uart4", USART4),
#endif
};

static const struct uart_irq_handler uart_irq_handlers[sizeof(uart_obj) / sizeof(uart_obj[0])];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
static rt_size_t rt_serial_update_write_index(struct rt_ringbuffer  *rb,
        rt_uint16_t     write_index)
{
    rt_uint16_t size;
    RT_ASSERT(rb != RT_NULL);

    /* whether has enough space */
    size = rt_ringbuffer_space_len(rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < write_index)
        write_index = size;

    if (rb->buffer_size - rb->write_index > write_index)
    {
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += write_index;
        return write_index;
    }

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = write_index - (rb->buffer_size - rb->write_index);

    return write_index;
}


static uint32_t hc32_get_uart_index(M4_USART_TypeDef *Instance)
{
    uint32_t index = UART_INDEX_MAX;

    for (uint8_t i = 0U; i < ARRAY_SZ(uart_map); i++)
    {
        if (uart_map[i].Instance == Instance)
        {
            index = uart_map[i].index;
            RT_ASSERT(index < UART_INDEX_MAX)
            break;
        }
    }

    return index;
}


static rt_err_t hc32_configure(struct rt_serial_device *serial,
                               struct serial_configure *cfg)
{

    en_result_t enRet = Ok;
    struct hc32_uart *uart;
    stc_usart_uart_init_t uart_init = {
        UsartIntClkCkNoOutput,
        UsartClkDiv_1,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSampleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    };

    if (BIT_ORDER_LSB == cfg->bit_order)
    {
        uart_init.enDirection = UsartDataLsbFirst;
    }
    else
    {
        uart_init.enDirection = UsartDataMsbFirst;
    }
    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        uart_init.enStopBit = UsartOneStopBit;
        break;
    case STOP_BITS_2:
        uart_init.enStopBit = UsartTwoStopBit;
        break;
    default:
        uart_init.enStopBit = UsartOneStopBit;
        break;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        uart_init.enParity = UsartParityNone;
        break;
    case PARITY_EVEN:
        uart_init.enParity = UsartParityEven;
        break;
    case PARITY_ODD:
        uart_init.enParity = UsartParityOdd;
        break;
    default:
        uart_init.enParity = UsartParityNone;
        break;
    }

    switch (cfg->data_bits)
    {
    case DATA_BITS_8:
        uart_init.enDataLength = UsartDataBits8;
        break;
    default:
        return -RT_ERROR;
    }


    RT_ASSERT(RT_NULL != cfg);
    RT_ASSERT(RT_NULL != serial)


    uart = rt_container_of(serial, struct hc32_uart, serial);
    RT_ASSERT(RT_NULL != uart->Instance);

    rt_err_t rt_hw_board_uart_init(M4_USART_TypeDef * USARTx);
    if (RT_EOK != rt_hw_board_uart_init(uart->Instance))
    {
        return -RT_ERROR;
    }
    /* Enable USART clock */
    uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1 | PWC_FCG1_PERIPH_USART2 | \
                             PWC_FCG1_PERIPH_USART3 | PWC_FCG1_PERIPH_USART4;
    PWC_Fcg1PeriphClockCmd(u32Fcg1Periph, Enable);

    enRet = USART_UART_Init(uart->Instance, &uart_init);
    if (enRet != Ok)
    {
        return -RT_ERROR;
    }


    /* Set baudrate */
    enRet = SetUartBaudrate(uart->Instance, 115200ul);
    if (enRet != Ok)
    {
        return -RT_ERROR;
    }


    /* Register RX error interrupt */
    hc32_install_irq_handler(&uart->config.rxerr_irq_config,
                             uart_irq_handlers[hc32_get_uart_index(uart->Instance)].rxerr_irq_handler,
                             RT_TRUE);

    USART_FuncCmd(uart->Instance, UsartRxInt, Enable);

    if ((serial->parent.flag & RT_DEVICE_FLAG_RDWR) || \
            (serial->parent.flag & RT_DEVICE_FLAG_RDONLY))
    {
        USART_FuncCmd(uart->Instance, UsartRx, Enable);
    }

    if ((serial->parent.flag & RT_DEVICE_FLAG_RDWR) || \
            (serial->parent.flag & RT_DEVICE_FLAG_WRONLY))
    {
        USART_FuncCmd(uart->Instance, UsartTx, Enable);
    }



    return RT_EOK;
}

static rt_err_t hc32_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct hc32_uart *uart;
    uint32_t uart_index;
    rt_ubase_t ctrl_arg = (rt_ubase_t)arg;
//#ifdef RT_SERIAL_USING_DMA
//#endif

    RT_ASSERT(RT_NULL != serial);

    uart = rt_container_of(serial, struct hc32_uart, serial);
    RT_ASSERT(RT_NULL != uart->Instance);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CONFIG:
        switch (ctrl_arg) {
        case RT_SERIAL_RX_BLOCKING:
        case RT_SERIAL_RX_NON_BLOCKING:{
            if (uart->uart_dma_flag) {
                hc32_dma_config(serial, ctrl_arg);
            }
            else {
                uart_index = hc32_get_uart_index(uart->Instance);
                hc32_install_irq_handler(&uart->config.rx_irq_config, uart_irq_handlers[uart_index].rx_irq_handler, RT_TRUE);
            }
            break;}
        case RT_SERIAL_TX_BLOCKING:
        case RT_SERIAL_TX_NON_BLOCKING:{
            if (uart->uart_dma_flag) {
                hc32_dma_config(serial, ctrl_arg);
                USART_FuncCmd(uart->Instance, UsartTx , Disable);
                USART_FuncCmd(uart->Instance, UsartTxCmpltInt, Disable);
                /* Install TC irq handler */
                uart_index = hc32_get_uart_index(uart->Instance);
                hc32_install_irq_handler(&uart->config.dma_tx->irq_config,
                                         uart_irq_handlers[uart_index].tc_irq_handler,
                                         RT_TRUE);
            }
            else {
							if(uart->Instance==M4_USART2){
							rt_kprintf("reopenhere");
							}
                uart_index = hc32_get_uart_index(uart->Instance);
//                USART_FuncCmd(uart->Instance, UsartTxEmptyInt, Enable);
                hc32_install_irq_handler(&uart->config.tx_irq_config,
                                         uart_irq_handlers[uart_index].tx_irq_handler,
                                         RT_TRUE);

            }

            break;}
        }
        break;

    /* Disable interrupt */
    case RT_DEVICE_CTRL_CLR_INT:
        if (RT_DEVICE_FLAG_INT_RX == ctrl_arg)
        {
            /* Disable RX irq */
            NVIC_DisableIRQ(uart->config.rx_irq_config.irq);
            enIrqResign(uart->config.rx_irq_config.irq);
        }
        else
        {
            /* Disable TX irq */
            NVIC_DisableIRQ(uart->config.tx_irq_config.irq);
            //may problem
            USART_FuncCmd(uart->Instance, UsartTxCmpltInt, Disable);
            enIrqResign(uart->config.tx_irq_config.irq);
        }
        break;

    /* Enable interrupt */
    case RT_DEVICE_CHECK_OPTMODE:

        if (RT_DEVICE_FLAG_TX_BLOCKING == ctrl_arg)
        {
            if (uart->uart_dma_flag) {return RT_SERIAL_TX_BLOCKING_NO_BUFFER;}
            else {return RT_SERIAL_TX_BLOCKING_BUFFER;}

        }
        break;


    case RT_DEVICE_CTRL_CLOSE:
        USART_DeInit(uart->Instance);
        break;
    }

    return RT_EOK;
}

static int hc32_putc(struct rt_serial_device *serial, char c)
{
    struct hc32_uart *uart;

    RT_ASSERT(RT_NULL != serial);

    uart = rt_container_of(serial, struct hc32_uart, serial);
    RT_ASSERT(RT_NULL != uart->Instance);
    while (USART_GetStatus(uart->Instance, UsartTxEmpty) != Set);
    USART_SendData(uart->Instance, c);

    return 1;
}

static int hc32_getc(struct rt_serial_device *serial)
{
    int ch = -1;
    struct hc32_uart *uart;

    RT_ASSERT(RT_NULL != serial);

    uart = rt_container_of(serial, struct hc32_uart, serial);
    RT_ASSERT(RT_NULL != uart->Instance);

    if (Set == USART_GetStatus(uart->Instance, UsartRxNoEmpty))
    {
        ch = (rt_uint8_t)USART_RecData(uart->Instance);
    }

    return ch;
}

#ifdef RT_SERIAL_USING_DMA

static rt_size_t hc32_transmit(struct rt_serial_device *serial,
                               rt_uint8_t *buf,
                               rt_size_t size,
                               rt_uint32_t tx_flag)
{

	struct hc32_uart *uart;
    M4_DMA_TypeDef *DMA_Instance;
    uint8_t ch;

    RT_ASSERT(RT_NULL != serial);
    RT_ASSERT(RT_NULL != buf);

    if (size == 0)
    {
        return 0;
    }

    uart = rt_container_of(serial, struct hc32_uart, serial);

//    if (RT_SERIAL_TX_NON_BLOCKING == tx_flag)
    if (uart->uart_dma_flag) {
        DMA_Instance = uart->config.dma_tx->Instance;
        ch = uart->config.dma_tx->channel;

        if (Reset == USART_GetStatus(uart->Instance, UsartTxComplete))
        {
            RT_ASSERT(0);
        }

        DMA_SetSrcAddress(DMA_Instance, ch, (uint32_t)buf);
        DMA_SetTransferCnt(DMA_Instance, ch, size);
        DMA_ChannelCmd(DMA_Instance, ch, Enable);
        /* Clear DMA flag. */
        DMA_ClearIrqFlag(DMA_Instance, ch, TrnCpltIrq);
        /* Enable peripheral circuit trigger function. */
        PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

        /* Set DMA trigger source. */

        USART_FuncCmd(uart->Instance, UsartTx, Enable);
        USART_FuncCmd(uart->Instance, UsartTxCmpltInt, Enable);
        return size;
    }
    else {
//		PORT_ResetBits(PortE,Pin01);
            USART_FuncCmd(uart->Instance, UsartTx, Disable);
						USART_FuncCmd(uart->Instance, UsartTxEmptyInt, Disable);
//			rt_kprintf("s2");
            USART_FuncCmd(uart->Instance, UsartTxAndTxEmptyInt, Enable);
    }
    return 0;
}

#endif

static void hc32_uart_rx_irq_handler(struct hc32_uart *uart)
{
    RT_ASSERT(RT_NULL != uart);
    char ch = hc32_getc(&uart->serial);
    if (ch != -1) {
        rt_ringbuffer_put(&(((struct rt_serial_tx_fifo *)uart->serial.serial_rx)->rb), &ch, 1);
    }
    rt_hw_serial_isr(&uart->serial, RT_SERIAL_EVENT_RX_IND);
}

static void hc32_uart_tx_irq_handler(struct hc32_uart *uart)
{												
//rt_kprintf("s1");

	char ch;
   if (rt_ringbuffer_getchar(&(((struct rt_serial_tx_fifo *)uart->serial.serial_tx)->rb), &ch) == 1)
                {            USART_SendData(uart->Instance, ch);}
	
    RT_ASSERT(RT_NULL != uart);
	if(rt_ringbuffer_data_len(&(((struct rt_serial_tx_fifo *)uart->serial.serial_tx)->rb))==0){
	USART_FuncCmd(uart->Instance, UsartTxEmptyInt, Disable);}
    rt_hw_serial_isr(&uart->serial, RT_SERIAL_EVENT_TX_DONE);
}

static void hc32_uart_rxerr_irq_handler(struct hc32_uart *uart)
{
    RT_ASSERT(RT_NULL != uart);
    RT_ASSERT(RT_NULL != uart->Instance);

    if (Set == USART_GetStatus(uart->Instance, (UsartParityErr | UsartFrameErr)))
    {
        USART_RecData(uart->Instance);
    }

    USART_ClearStatus(uart->Instance, (UsartParityErr | \
                                       UsartFrameErr | \
                                       UsartOverrunErr));
}

#ifdef RT_SERIAL_USING_DMA
static void hc32_uart_rx_timeout(struct rt_serial_device *serial)
{
    struct hc32_uart *uart;
    uint32_t timeout_bits;
    M4_TMR0_TypeDef* TMR0_Instance;
    uint8_t ch;

    RT_ASSERT(RT_NULL != serial);

    uart = rt_container_of(serial, struct hc32_uart, serial);
    RT_ASSERT(RT_NULL != uart->Instance);

    TMR0_Instance = uart->config.rx_timeout->TMR0_Instance;
    ch = uart->config.rx_timeout->channel;
    timeout_bits = uart->config.rx_timeout->timeout_bits;

    stc_clk_freq_t stcClkTmp;
    stc_tim0_base_init_t stcTimerCfg;
    stc_tim0_trigger_init_t StcTimer0TrigInit;

    MEM_ZERO_STRUCT(stcClkTmp);
    MEM_ZERO_STRUCT(stcTimerCfg);
    MEM_ZERO_STRUCT(StcTimer0TrigInit);

    /* Timer0 peripheral enable */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM01, Enable);
    TIMER0_WriteCntReg(TMR0_Instance, Tim0_ChannelA, 0u);
    TIMER0_WriteCntReg(TMR0_Instance, Tim0_ChannelB, 0u);
    /* Config register for channel B */
    stcTimerCfg.Tim0_CounterMode = Tim0_Sync;
    stcTimerCfg.Tim0_SyncClockSource = Tim0_Pclk1;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv8;
    uint32_t cmp_val;
    if (stcTimerCfg.Tim0_ClockDivision == Tim0_ClkDiv0)
    {
        cmp_val = (timeout_bits - 4UL);
    }
    else if (Tim0_ClkDiv2 == stcTimerCfg.Tim0_ClockDivision)
    {
        cmp_val = (timeout_bits / 2UL - 2UL);
    }
    else
    {
        cmp_val = (timeout_bits / (1UL << (stcTimerCfg.Tim0_ClockDivision)) - 1UL);
    }
//      rt_kprintf("%ul",cmp_val);
    DDL_ASSERT(cmp_val <= 0xFFFFUL);
    stcTimerCfg.Tim0_CmpValue = cmp_val;
    TIMER0_BaseInit(TMR0_Instance, ch, &stcTimerCfg);

    /* Clear compare flag */
    TIMER0_ClearFlag(TMR0_Instance, ch);

    /* Config timer0 hardware trigger */
    StcTimer0TrigInit.Tim0_InTrigEnable = false;
    StcTimer0TrigInit.Tim0_InTrigClear = true;
    StcTimer0TrigInit.Tim0_InTrigStart = true;
    StcTimer0TrigInit.Tim0_InTrigStop = false;
    TIMER0_HardTriggerInit(TMR0_Instance, ch, &StcTimer0TrigInit);

    /* Register RTO interrupt */
    hc32_install_irq_handler(&uart->config.rx_timeout->irq_config,
                             uart_irq_handlers[hc32_get_uart_index(uart->Instance)].rxto_irq_handler,
                             RT_TRUE);

    USART_ClearStatus(uart->Instance, UsartRxTimeOut);
    USART_FuncCmd(uart->Instance, UsartTimeOut, Enable);
    USART_FuncCmd(uart->Instance, UsartTimeOutInt, Enable);

}

static void hc32_dma_config(struct rt_serial_device *serial, rt_ubase_t flag)
{
    struct hc32_uart *uart;
    stc_dma_config_t dma_init;
    M4_DMA_TypeDef *DMA_Instance;
    uint32_t DMA_ch;
    uint32_t u32Fcg0Periph = PWC_FCG0_PERIPH_AOS;

    RT_ASSERT(RT_NULL != serial);

    uart = rt_container_of(serial, struct hc32_uart, serial);
    RT_ASSERT(RT_NULL != uart->Instance);

    if (RT_DEVICE_FLAG_DMA_RX == flag)
    {
        stc_dma_llp_init_t llp_init;
        struct rt_serial_rx_fifo *rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;

        RT_ASSERT(RT_NULL != uart->config.rx_timeout->TMR0_Instance);
        RT_ASSERT(RT_NULL != uart->config.dma_rx->Instance);

        /* Initialization uart rx timeout for DMA */
        hc32_uart_rx_timeout(serial);

        uart->dma_rx_last_index = 0UL;

        /* Get DMA unit&channel */
        DMA_Instance = uart->config.dma_rx->Instance;
        DMA_ch = uart->config.dma_rx->channel;

        /* Enable DMA clock */
        u32Fcg0Periph |= (M4_DMA1 == DMA_Instance) ? PWC_FCG0_PERIPH_DMA1 : PWC_FCG0_PERIPH_DMA2;
        PWC_Fcg0PeriphClockCmd(u32Fcg0Periph, Enable);

        /* Disable DMA */
        DMA_ChannelCmd(DMA_Instance, DMA_ch, Disable);
        /* Initialize DMA */
        MEM_ZERO_STRUCT(dma_init);
        dma_init.stcDmaChCfg.enIntEn     = Enable;
        dma_init.u32SrcAddr   = ((uint32_t)(&uart->Instance->DR) + 2UL);
        dma_init.u32DesAddr  = (uint32_t)rx_fifo->buffer;
        dma_init.stcDmaChCfg.enTrnWidth = Dma8Bit;
        dma_init.u16BlockSize = 1UL;
        dma_init.u16TransferCnt  = serial->config.rx_bufsz;
        dma_init.stcDmaChCfg.enSrcInc    = AddressFix;
        dma_init.stcDmaChCfg.enDesInc    = AddressIncrease;
        DMA_InitChannel(DMA_Instance, DMA_ch, &dma_init);


        /* Initialize LLP */
        static stc_dma_llp_descriptor_t llp_desc;
        llp_init.u32LlpEn  = Enable;
        llp_init.u32LlpRun = LlpWaitNextReq;
        llp_init.u32LlpAddr = (uint32_t)&llp_desc;
        DMA_LlpInit(DMA_Instance, DMA_ch, &llp_init);

        /* Configure LLP descriptor */
        llp_desc.SARx  = dma_init.u32SrcAddr;
        llp_desc.DARx  = dma_init.u32DesAddr;
        llp_desc.DTCTLx = (((uint32_t)dma_init.u16TransferCnt ) << DMA_DTCTL_CNT_Pos) | (dma_init.u16BlockSize << DMA_DTCTL_BLKSIZE_Pos);
        llp_desc.LLPx  = (uint32_t)&llp_desc;
        llp_desc.CHxCTL = (dma_init.stcDmaChCfg.enSrcInc  | ((uint32_t)dma_init.stcDmaChCfg.enDesInc) << (DMA_CHCTL_DINC_Pos) | ((uint32_t)dma_init.stcDmaChCfg.enTrnWidth) << (DMA_CHCTL_HSIZE_Pos) | \
                           ((uint32_t)llp_init.u32LlpEn) << DMA_CHCTL_LLPEN_Pos              | ((uint32_t)llp_init.u32LlpRun) << DMA_CHCTL_LLPRUN_Pos  | ((uint32_t)llp_init.u32LlpEn) << DMA_CHCTL_IE_Pos );

        /* Register DMA interrupt */
        hc32_install_irq_handler(&uart->config.dma_rx->irq_config,
                                 uart_irq_handlers[hc32_get_uart_index(uart->Instance)].dma_rx_irq_handler,
                                 RT_TRUE);

        /* Enable DMA module */
        DMA_Cmd(DMA_Instance, Enable);
        DMA_EnableIrq(DMA_Instance, DMA_ch, BlkTrnCpltIrq);
        DMA_ClearIrqFlag(DMA_Instance, DMA_ch, TrnCpltIrq);
        DMA_SetTriggerSrc(DMA_Instance, DMA_ch, uart->config.dma_rx->trigger_evt_src);
        DMA_ChannelCmd(DMA_Instance, DMA_ch, Enable);
    }
    else if (RT_DEVICE_FLAG_DMA_TX == flag)
    {
        RT_ASSERT(RT_NULL != uart->config.dma_tx->Instance);

        DMA_Instance = uart->config.dma_tx->Instance;
        DMA_ch = uart->config.dma_tx->channel;

        /* Enable DMA clock */
        u32Fcg0Periph |= (M4_DMA1 == DMA_Instance) ? PWC_FCG0_PERIPH_DMA1 : PWC_FCG0_PERIPH_DMA2;
        PWC_Fcg0PeriphClockCmd(u32Fcg0Periph, Enable);

        /* Disable DMA */
        DMA_ChannelCmd(DMA_Instance, DMA_ch, Disable);

        /* Initialize DMA */
        MEM_ZERO_STRUCT(dma_init);
        dma_init.stcDmaChCfg.enIntEn     = Enable;
        dma_init.u32SrcAddr   = 0UL;
        dma_init.u32DesAddr  = (uint32_t)(&uart->Instance->DR);
        dma_init.stcDmaChCfg.enTrnWidth = Dma8Bit;
        dma_init.u16BlockSize = 1UL;
        dma_init.u16TransferCnt  = 0UL;
        dma_init.stcDmaChCfg.enSrcInc    = AddressIncrease;
        dma_init.stcDmaChCfg.enDesInc    = AddressFix;
        DMA_InitChannel(DMA_Instance, DMA_ch, &dma_init);

        /* Enable DMA module */
        DMA_Cmd(DMA_Instance, Enable);
        DMA_EnableIrq(DMA_Instance, DMA_ch, BlkTrnCpltIrq);
        DMA_SetTriggerSrc(DMA_Instance, DMA_ch, uart->config.dma_tx->trigger_evt_src);
    }
}

static void hc32_uart_tc_irq_handler(struct hc32_uart *uart)
{
    RT_ASSERT(uart != RT_NULL);

    USART_FuncCmd(uart->Instance, UsartTx, Disable);
    USART_FuncCmd(uart->Instance, UsartTxCmpltInt, Disable);
    if (uart->serial.parent.open_flag & RT_DEVICE_FLAG_DMA_TX)
    {
        rt_hw_serial_isr(&uart->serial, RT_SERIAL_EVENT_TX_DMADONE);
    }
}

static void hc32_uart_dma_rx_irq_handler(struct hc32_uart *uart)
{
    struct rt_serial_device *serial;
    rt_size_t recv_len;
    rt_base_t level;

    RT_ASSERT(RT_NULL != uart);
    RT_ASSERT(RT_NULL != uart->Instance);

    serial = &uart->serial;

    level = rt_hw_interrupt_disable();
    recv_len = serial->config.rx_bufsz - uart->dma_rx_last_index;
    uart->dma_rx_last_index = 0UL;
    rt_hw_interrupt_enable(level);

    if (recv_len)
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
    }
}

static void hc32_uart_rxto_irq_handler(struct hc32_uart *uart)
{
    rt_base_t level;
    rt_size_t cnt;
    rt_size_t recv_len;
    rt_size_t recv_total_index;

    cnt = DMA_TRANS_CNT(uart->config.dma_rx->Instance , uart->config.dma_rx->channel);
    recv_total_index = uart->serial.config.rx_bufsz - cnt;
    if (0UL != recv_total_index)
    {
        level = rt_hw_interrupt_disable();
        recv_len = recv_total_index - uart->dma_rx_last_index;
        uart->dma_rx_last_index = recv_total_index;
        rt_hw_interrupt_enable(level);

        if (recv_len)
        {
            rt_hw_serial_isr(&uart->serial, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
        }
    }
    TIMER0_Cmd(uart->config.rx_timeout->TMR0_Instance, uart->config.rx_timeout->channel, Disable);
    USART_ClearStatus(uart->Instance, UsartRxTimeOut);

}
#endif

#if defined(BSP_USING_UART1)
static void hc32_uart1_rx_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rx_irq_handler(&uart_obj[UART1_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void hc32_uart1_tx_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_tx_irq_handler(&uart_obj[UART1_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void hc32_uart1_rxerr_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rxerr_irq_handler(&uart_obj[UART1_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

#if defined(RT_SERIAL_USING_DMA)
static void hc32_uart1_tc_irq_handler(void)
{
#if defined(BSP_UART1_TX_USING_DMA)
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_tc_irq_handler(&uart_obj[UART1_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

static void hc32_uart1_rxto_irq_handler(void)
{
#if defined(BSP_UART1_RX_USING_DMA)
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rxto_irq_handler(&uart_obj[UART1_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

static void hc32_uart1_dma_rx_irq_handler(void)
{
    rt_kprintf("s");
#if defined(BSP_UART1_RX_USING_DMA)
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_dma_rx_irq_handler(&uart_obj[UART1_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}
#endif /* RT_SERIAL_USING_DMA */
#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART2)
static void hc32_uart2_rx_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rx_irq_handler(&uart_obj[UART2_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void hc32_uart2_tx_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_tx_irq_handler(&uart_obj[UART2_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void hc32_uart2_rxerr_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rxerr_irq_handler(&uart_obj[UART2_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

#if defined(RT_SERIAL_USING_DMA)
static void hc32_uart2_tc_irq_handler(void)
{
#if defined(BSP_UART2_TX_USING_DMA)
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_tc_irq_handler(&uart_obj[UART2_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

static void hc32_uart2_rxto_irq_handler(void)
{
#if defined(BSP_UART2_RX_USING_DMA)
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rxto_irq_handler(&uart_obj[UART2_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

static void hc32_uart2_dma_rx_irq_handler(void)
{
#if defined(BSP_UART2_RX_USING_DMA)
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_dma_rx_irq_handler(&uart_obj[UART2_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}
#endif /* RT_SERIAL_USING_DMA */
#endif /* BSP_USING_UART2 */

#if defined(BSP_USING_UART3)
static void hc32_uart3_rx_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rx_irq_handler(&uart_obj[UART3_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void hc32_uart3_tx_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_tx_irq_handler(&uart_obj[UART3_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void hc32_uart3_rxerr_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rxerr_irq_handler(&uart_obj[UART3_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* BSP_USING_UART3 */

#if defined(BSP_USING_UART4)
static void hc32_uart4_rx_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rx_irq_handler(&uart_obj[UART4_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void hc32_uart4_tx_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_tx_irq_handler(&uart_obj[UART4_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void hc32_uart4_rxerr_irq_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    hc32_uart_rxerr_irq_handler(&uart_obj[UART4_INDEX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* BSP_USING_UART4 */


static const struct uart_irq_handler uart_irq_handlers[] =
{
#ifdef BSP_USING_UART1
    {   hc32_uart1_rxerr_irq_handler, hc32_uart1_rx_irq_handler, hc32_uart1_tx_irq_handler,
        hc32_uart1_tc_irq_handler, hc32_uart1_rxto_irq_handler,  hc32_uart1_dma_rx_irq_handler
    },
#endif
#ifdef BSP_USING_UART2
    {   hc32_uart2_rxerr_irq_handler, hc32_uart2_rx_irq_handler, hc32_uart2_tx_irq_handler,
        hc32_uart2_tc_irq_handler, hc32_uart2_rxto_irq_handler,  hc32_uart2_dma_rx_irq_handler
    },
#endif
#ifdef BSP_USING_UART3
    {hc32_uart3_rxerr_irq_handler, hc32_uart3_rx_irq_handler, hc32_uart3_tx_irq_handler},
#endif
#ifdef BSP_USING_UART4
    {hc32_uart4_rxerr_irq_handler, hc32_uart4_rx_irq_handler, hc32_uart4_tx_irq_handler},
#endif
};

static void hc32_uart_get_dma_config(void)
{
#ifdef BSP_USING_UART1
    uart_obj[UART1_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART1_RX_USING_DMA
    uart_obj[UART1_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;

    static struct hc32_uart_rxto uart1_rx_timeout = UART_RXTO_CONFIG(USART1);
    uart_obj[UART1_INDEX].config.rx_timeout = &uart1_rx_timeout;

    static struct dma_config uart1_dma_rx = UART_DMA_RX_CONFIG(USART1);
    uart_obj[UART1_INDEX].config.dma_rx = &uart1_dma_rx;
#endif
#ifdef BSP_UART1_TX_USING_DMA
    uart_obj[UART1_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;

    static struct dma_config uart1_dma_tx = UART_DMA_TX_CONFIG(USART1);
    uart_obj[UART1_INDEX].config.dma_tx = &uart1_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART2
    uart_obj[UART2_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART2_RX_USING_DMA
    uart_obj[UART2_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;

    static struct hc32_uart_rxto uart2_rx_timeout = UART_RXTO_CONFIG(USART2);
    uart_obj[UART2_INDEX].config.rx_timeout = &uart2_rx_timeout;

    static struct dma_config uart2_dma_rx = UART_DMA_RX_CONFIG(USART2);
    uart_obj[UART2_INDEX].config.dma_rx = &uart2_dma_rx;
#endif
#ifdef BSP_UART2_TX_USING_DMA
    uart_obj[UART2_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;

    static struct dma_config uart2_dma_tx = UART_DMA_TX_CONFIG(USART2);
    uart_obj[UART2_INDEX].config.dma_tx = &uart2_dma_tx;
#endif
#endif

}

static const struct rt_uart_ops hc32_uart_ops =
{
    .configure = hc32_configure,
    .control = hc32_control,
    .putc = hc32_putc,
    .getc = hc32_getc,
    .transmit = hc32_transmit
};

int hc32_hw_uart_init(void)
{
    rt_err_t result = RT_EOK;
    rt_size_t obj_num = sizeof(uart_obj) / sizeof(struct hc32_uart);
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    hc32_uart_get_dma_config();

    for (int i = 0; i < obj_num; i++)
    {
        /* init UART object */
        uart_obj[i].serial.ops    = &hc32_uart_ops;
//        config.rx_bufsz=0;
        uart_obj[i].serial.config = config;
	//		if(i==0){
        uart_obj[i].serial.config.tx_bufsz = 0;
		//}

        /* register UART device */
        result = rt_hw_serial_register(&uart_obj[i].serial,
                                       uart_obj[i].name,
                                       (RT_DEVICE_FLAG_RDWR   |
                                        RT_SERIAL_RX_NON_BLOCKING |
                                        RT_SERIAL_TX_BLOCKING ),
                                       &uart_obj[i]);
        RT_ASSERT(result == RT_EOK);
    }

    return result;
}

INIT_BOARD_EXPORT(hc32_hw_uart_init);

#endif /* RT_USING_SERIAL */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
