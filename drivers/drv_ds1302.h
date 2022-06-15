/*
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-10-30     CDT          first version
 */
 
#ifndef __DRV_DS1302_H__
#define __DRV_DS1302_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <rtdef.h>
#include "hc32_ddl.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
enum Register {
  kSecondReg       = 0,
  kMinuteReg       = 1,
  kHourReg         = 2,
  kDateReg         = 3,
  kMonthReg        = 4,
  kDayReg          = 5,
  kYearReg         = 6
  };


/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
rt_err_t hc32_install_irq_handler(struct hc32_irq_config *irq_config,
                                    void (*irq_hdr)(void),
                                    rt_bool_t irq_enable);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_IRQ_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
