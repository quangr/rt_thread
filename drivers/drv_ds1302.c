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
#include "drv_irq.h"
uint32_t timechanged=0;

#define clk_pin (0x28)
#define rst_pin (0x08)
#define dat_pin (0x29)

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

#include <board.h>
#include <rtdevice.h>
#include <time.h>
#include "drv_ds1302.h"

void Delay(void)
{
        int y0;
        for (y0 = 15; y0--;)
                ;
}

struct ds1302_device
{
        struct rt_device rtc_parent;
};
static struct ds1302_device ds1302_dev;

/* bcd to hex */
static unsigned char bcd_to_hex(unsigned char data)
{
        unsigned char temp;

        temp = ((data >> 4) * 10 + (data & 0x0f));
        return temp;
}

/* hex_to_bcd */
static unsigned char hex_to_bcd(unsigned char data)
{
        unsigned char temp;

        temp = (((data / 10) << 4) + (data % 10));
        return temp;
}
void pulse_clk(void)
{
        rt_pin_write(clk_pin, PIN_HIGH);
        Delay();
        rt_pin_write(clk_pin, PIN_LOW);
}
void rtc_write(uint8_t value)
{
        rt_pin_mode(dat_pin, PIN_MODE_OUTPUT);
        for (int i = 0; i < 8; i++)
        {
                rt_pin_write(dat_pin, (value >> i) & 1 ? PIN_HIGH : PIN_LOW);
                pulse_clk();
        }
}
uint8_t rtc_read()
{
        uint8_t input_value = 0;
        uint8_t bit = 0;
        rt_pin_mode(dat_pin, PIN_MODE_INPUT);
        for (int i = 0; i < 8; ++i)
        {
                bit = rt_pin_read(dat_pin);
                input_value |= (bit << i);
                pulse_clk();
        }
        return input_value;
}

/* ds1302 read register */
rt_uint8_t ds1302_read_reg(rt_uint8_t reg)
{
        volatile uint8_t cmd_byte = (0x81 | (reg << 1));
        uint8_t result = 0;
        rt_pin_write(rst_pin, PIN_HIGH);
        Delay();

        rtc_write(cmd_byte);
        Delay();
        result = rtc_read();
        Delay();
        rt_pin_write(rst_pin, PIN_LOW);

        return result;
}

/* ds1302 write register */
rt_err_t ds1302_write_reg(rt_uint8_t reg, rt_uint8_t data)
{
        uint8_t cmd_byte = (0x80 | (reg << 1));
        rt_pin_write(rst_pin, PIN_HIGH);
        Delay();
        rtc_write(cmd_byte);
        Delay();
        rtc_write(data);
        Delay();
        rt_pin_write(rst_pin, PIN_LOW);
        Delay();
}

static rt_err_t rt_ds1302_open(rt_device_t dev, rt_uint16_t flag)
{
        if (dev->rx_indicate != RT_NULL)
        {
                /* open interrupt */
        }

        return RT_EOK;
}

static rt_size_t rt_ds1302_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
        return RT_EOK;
}
static int wd(int year, int month, int day)
{
        /* using C99 compound literals in a single line: notice the splicing */
        return (
                   day + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) + (365 * (year + 4800 - ((14 - month) / 12))) + ((year + 4800 - ((14 - month) / 12)) / 4) - ((year + 4800 - ((14 - month) / 12)) / 100) + ((year + 4800 - ((14 - month) / 12)) / 400) - 32045) %
               7;
}

static rt_err_t rt_ds1302_control(rt_device_t dev, int cmd, void *args)
{
        rt_err_t ret = RT_EOK;
        time_t *time;
        struct tm time_temp;
        rt_uint8_t buff[7];

        RT_ASSERT(dev != RT_NULL);
        rt_memset(&time_temp, 0, sizeof(struct tm));

        switch (cmd)
        {
        case RT_DEVICE_CTRL_RTC_GET_TIME:
                time = (time_t *)args;
                if (ret == RT_EOK)
                {
                        time_temp.tm_year = bcd_to_hex(ds1302_read_reg(kYearReg)) + 100;
                        time_temp.tm_mon = bcd_to_hex(ds1302_read_reg(kMonthReg) - 1);
                        time_temp.tm_mday = bcd_to_hex(ds1302_read_reg(kDateReg));
                        time_temp.tm_hour = bcd_to_hex(ds1302_read_reg(kHourReg));
                        time_temp.tm_min = bcd_to_hex(ds1302_read_reg(kMinuteReg));
                        time_temp.tm_sec = bcd_to_hex(ds1302_read_reg(kSecondReg));

                        *time = mktime(&time_temp);
                }
                break;

        case RT_DEVICE_CTRL_RTC_SET_TIME:
        {
                struct tm *time_new;

                time = (time_t *)args;
                time_new = localtime(time);
                int y = time_new->tm_year - 100;
                int m = time_new->tm_mon + 1;
                int d = time_new->tm_mday;
                rt_kprintf("y:%d,m:%d,d:%d,get weekday:%d", y, m, d, wd(y, m, d) + 1);
                ds1302_write_reg(kYearReg, hex_to_bcd(time_new->tm_year - 100));
                ds1302_write_reg(kMonthReg, hex_to_bcd(time_new->tm_mon + 1));
                ds1302_write_reg(kDateReg, hex_to_bcd(time_new->tm_mday));
                rt_enter_critical();
                ds1302_write_reg(kHourReg, hex_to_bcd(time_new->tm_hour));
                ds1302_write_reg(kMinuteReg, hex_to_bcd(time_new->tm_min));
                ds1302_write_reg(kSecondReg, hex_to_bcd(time_new->tm_sec));
                ds1302_write_reg(kDayReg, hex_to_bcd(wd(y, m, d) + 1));
                timechanged=1;
                rt_exit_critical();
        }
        break;
        default:
                break;
        }
        return RT_EOK;
}
extern uint8_t key_clock[7];

#define Kyear key_clock[0]
#define Kmonth key_clock[1]
#define Kdate key_clock[2]
#define Kweek key_clock[3]
#define Khour key_clock[4]
#define Kmin key_clock[5]
#define Ksec key_clock[6]

void rt_read_timer(uint32_t *nowtime, uint8_t *nowweek)
{

        *nowweek = bcd_to_hex(ds1302_read_reg(kDayReg));
        Kyear = bcd_to_hex(ds1302_read_reg(kYearReg));
        Kmonth = bcd_to_hex(ds1302_read_reg(kMonthReg));
        Kdate = bcd_to_hex(ds1302_read_reg(kDateReg));
        Ksec = bcd_to_hex(ds1302_read_reg(kSecondReg));
        Kmin = bcd_to_hex(ds1302_read_reg(kMinuteReg));
        Khour = bcd_to_hex(ds1302_read_reg(kHourReg));
        Kweek = *nowweek;
        *nowtime = Ksec + Kmin * 60 + Khour * 60 * 60;
}

/* ds1302 device int  */
int rt_hw_ds1302_init(void)
{
        //TODO charge
        //TODO FASTER read
        uint8_t data;
        rt_pin_mode(clk_pin, PIN_MODE_OUTPUT);
        rt_pin_mode(rst_pin, PIN_MODE_OUTPUT);
        rt_pin_write(clk_pin, PIN_LOW);
        rt_pin_write(rst_pin, PIN_LOW);
        Delay();
        ds1302_write_reg(0xe, 0);
        Delay();

        ds1302_dev.rtc_parent.type = RT_Device_Class_RTC;
        ds1302_dev.rtc_parent.init = RT_NULL;
        ds1302_dev.rtc_parent.open = rt_ds1302_open;
        ds1302_dev.rtc_parent.close = RT_NULL;
        ds1302_dev.rtc_parent.read = rt_ds1302_read;
        ds1302_dev.rtc_parent.write = RT_NULL;
        ds1302_dev.rtc_parent.control = rt_ds1302_control;
        ds1302_dev.rtc_parent.user_data = RT_NULL; /* no private */
        rt_device_register(&ds1302_dev.rtc_parent, "rtc", RT_DEVICE_FLAG_RDWR);
        return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_ds1302_init);
/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
