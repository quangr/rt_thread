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
#include <rtthread.h>
#include <hc32_ddl.h>
#include "rtdevice.h"

#define I2C_UNIT (M4_I2C1)
/* Define port and pin for SDA and SCL */
#define I2C_SCL_PORT (PortC)
#define I2C_SCL_PIN (Pin05)
#define I2C_SDA_PORT (PortC)
#define I2C_SDA_PIN (Pin04)
#define I2C_GPIO_SCL_FUNC (Func_I2c1_Scl)
#define I2C_GPIO_SDA_FUNC (Func_I2c1_Sda)
#define E2_MEM_ADR_LEN (1u)

#define I2C_FCG_USE (PWC_FCG1_PERIPH_I2C1)
#define E2_ADDRESS (0x50u)

#define TIMEOUT (0x40000ul)

struct rt_i2c_bus_device i2c1_bus;

static en_result_t I2C_Write(uint16_t u8DevAddr, uint8_t *pu8Data, uint32_t u32Size, uint32_t u32TimeOut)
{
    en_result_t enRet;

    I2C_SoftwareResetCmd(I2C_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_UNIT, Disable);
    enRet = I2C_Start(I2C_UNIT, u32TimeOut);
    if (Ok == enRet)
    {
        enRet = I2C_TransAddr(I2C_UNIT, u8DevAddr, I2CDirTrans, u32TimeOut);

        if (Ok == enRet)
        {
            enRet = I2C_TransData(I2C_UNIT, pu8Data, u32Size, u32TimeOut);
        }
    }

    // I2C_Stop(I2C_UNIT, u32TimeOut);
    // I2C_Cmd(I2C_UNIT, Disable);

    return enRet;
}

/**
 ******************************************************************************
 ** \brief  I2C memory read
 **
 ** \param  u8DevAddr             The slave address
 ** \param  u16MemAddr            The memory address
 ** \param  pu8Data               Pointer to the data buffer
 ** \param  u32Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
static en_result_t I2C_Read(uint8_t u8DevAddr, uint8_t *pu8Data, uint32_t u32Size, uint32_t u32TimeOut)
{
    en_result_t enRet;
 
    I2C_SoftwareResetCmd(I2C_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_UNIT, Disable);
    if (1ul == u32Size)
    {
        I2C_AckConfig(I2C_UNIT, I2c_NACK);
    }

    enRet = I2C_TransAddr(I2C_UNIT, (uint8_t)u8DevAddr, I2CDirReceive, u32TimeOut);
    if (Ok == enRet)
    {
        enRet = I2C_MasterDataReceiveAndStop(I2C_UNIT, pu8Data, u32Size, u32TimeOut);
    }

    I2C_AckConfig(I2C_UNIT, I2c_ACK);
    return enRet;
}

static rt_err_t i2c_hw_init(void)
{
    stc_i2c_init_t stcI2cInit;
    en_result_t enRet;
    float32_t fErr;
    PORT_SetFunc(I2C_SCL_PORT, I2C_SCL_PIN, I2C_GPIO_SCL_FUNC, Disable);
    PORT_SetFunc(I2C_SDA_PORT, I2C_SDA_PIN, I2C_GPIO_SDA_FUNC, Disable);

    /* Enable I2C Peripheral*/
    PWC_Fcg1PeriphClockCmd(I2C_FCG_USE, Enable);

    I2C_DeInit(I2C_UNIT);

    MEM_ZERO_STRUCT(stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV8;
    stcI2cInit.u32Baudrate = 100000ul;
    stcI2cInit.u32SclTime = 0ul;
    enRet = I2C_Init(I2C_UNIT, &stcI2cInit, &fErr);

    I2C_BusWaitCmd(I2C_UNIT, Enable);

    return enRet;
    if (enRet == Ok)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

static rt_size_t i2c_xfer(struct rt_i2c_bus_device *bus, struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    I2C_Cmd(I2C_UNIT, Enable);
    I2C_Start(I2C_UNIT, bus->timeout);
    struct rt_i2c_msg *msg;
    rt_int32_t i, ret;
    en_result_t a;

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        if (msg->flags & RT_I2C_RD)
        {
            a = I2C_Read((uint8_t)msg->addr, msg->buf, msg->len, bus->timeout);
        }
        else
        {
            if(msg->flags& RT_I2C_NO_START){
                a= I2C_TransData(I2C_UNIT, msg->buf, msg->len, bus->timeout);

            }
            a = I2C_Write((uint8_t)msg->addr, msg->buf, msg->len, bus->timeout);
        }
        if(a!=Ok){
            return 0;
        }

    }
    ret = i;
    I2C_Stop(I2C_UNIT, bus->timeout);
    return ret;
    
}

static const struct rt_i2c_bus_device_ops i2c_bus_ops =
    {
        i2c_xfer,
        RT_NULL,
        RT_NULL};

int rt_i2c_hw_init(void)
{
    rt_pin_mode(0x1c, PIN_MODE_OUTPUT);
    rt_pin_write(0x1c, PIN_LOW);
    i2c_hw_init();

    i2c1_bus.ops = &i2c_bus_ops;
    i2c1_bus.timeout = TIMEOUT;
    rt_i2c_bus_device_register(&i2c1_bus, "i2c");
    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_i2c_hw_init);