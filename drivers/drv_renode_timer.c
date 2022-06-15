#include <rtthread.h>
#include <hc32_ddl.h>
#include "drv_irq.h"
#include "renode.h"

extern uint8_t key_clock[7];
uint32_t timechanged=0;
#define Kyear key_clock[0]
#define Kmonth key_clock[1]
#define Kdate key_clock[2]
#define Kweek key_clock[3]
#define Khour key_clock[4]
#define Kmin key_clock[5]
#define Ksec key_clock[6]
struct ds1302_device
{
    struct rt_device rtc_parent;
};
static struct ds1302_device ds1302_dev;
static rt_err_t rt_ds1302_open(rt_device_t dev, rt_uint16_t flag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* open interrupt */
    }

    return RT_EOK;
}
static int wd(int year, int month, int day)
{
        /* using C99 compound literals in a single line: notice the splicing */
        return (
                   day + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) + (365 * (year + 4800 - ((14 - month) / 12))) + ((year + 4800 - ((14 - month) / 12)) / 4) - ((year + 4800 - ((14 - month) / 12)) / 100) + ((year + 4800 - ((14 - month) / 12)) / 400) - 32045) %
               7;
}

static rt_size_t rt_ds1302_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return RT_EOK;
}

void rt_read_timer(uint32_t *nowtime, uint8_t *nowweek)
{
    uint8_t *t1 =(uint8_t *) rtc;
    Kyear = *t1;
    t1++;
    Kmonth = *t1;
    t1++;
    Kdate = *t1;
    t1++;
    Khour = *t1;
    t1++;
    Kmin = *t1;
    t1++;
    Ksec = *t1;
    t1++;
    Kweek = *t1;
    *nowtime = Ksec + Kmin * 60 + Khour * 60 * 60;
    *nowweek=Kweek;
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
            uint8_t *t1 = (uint8_t *)rtc;
            time_temp.tm_year = *t1 + 100;
            t1++;
            time_temp.tm_mon = *t1 - 1;
            t1++;
            time_temp.tm_mday = *t1;
            t1++;
            time_temp.tm_hour = *t1;
            t1++;
            time_temp.tm_min = *t1;
            t1++;
            time_temp.tm_sec = *t1;

            *time = mktime(&time_temp);
        }
        break;
    //TODO AFTER SET DATE HOW TO KNOW IF WE SHOULD TURN ON SOMETHING
    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
        struct tm *time_new;

        time = (time_t *)args;
        time_new = localtime(time);
        int y = time_new->tm_year - 100;
        int m = time_new->tm_mon + 1;
        int d = time_new->tm_mday;
        uint8_t *t1 = (uint8_t *)rtc;
        *t1 =(uint8_t)(time_new->tm_year - 100);
        t1++;
        *t1 = (uint8_t)(time_new->tm_mon + 1);
        t1++;
        *t1 = (uint8_t)(time_new->tm_mday);
        t1++;
        rt_enter_critical();
        *t1 = (uint8_t)(time_new->tm_hour);
        t1++;
        *t1 = (uint8_t)(time_new->tm_min);
        t1++;
        *t1 = (uint8_t)(time_new->tm_sec);
        t1++;
        *t1 = (uint8_t)(wd(y, m, d) + 1);
        timechanged=1;
        rt_exit_critical();
    }
    break;
    default:
        break;
    }
    return RT_EOK;
}

/* ds1302 device int  */
int rt_hw_ds1302_init(void)
{

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
