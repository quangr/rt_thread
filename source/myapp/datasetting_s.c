#include <board_config.h>
#include <rtthread.h>
#include "arm_math.h"
#include "at24cxx.h"

#define SWITCH_ON_0 1 << 0
#define SWITCH_ON_1 1 << 1
#define SWITCH_ON_2 1 << 2
#define SWITCH_ON_3 1 << 3

#define PROTECT_ON_0 1 << 4
#define PROTECT_ON_1 1 << 5
#define PROTECT_ON_2 1 << 6
#define PROTECT_ON_3 1 << 7

#define TIMER_ON_0 1 << 8
#define TIMER_ON_1 1 << 9
#define TIMER_ON_2 1 << 10
#define TIMER_ON_3 1 << 11

#define PROTECT_SETTING_START 24

#define TIMER_SETTING_START 40

#define MODE_SETTING_START 49

volatile uint32_t senddata[64] = {0};
volatile uint16_t data_ready = 0, state_changed = 0, control_modifings = 0;

#define ROM_TIME_POS 0
#define ROM_TIME_length 8

#define ROM_PIA_I_POS 8
#define ROM_PIA_I_length 2

#define ROM_TIA_I_POS 10
#define ROM_TIA_I_length 2

#define ROM_PIA_II_POS 12
#define ROM_PIA_II_length 2

#define ROM_TIA_II_POS 14
#define ROM_TIA_II_length 2

#define ROM_WEEKDAYA_POS 40
#define ROM_WEEKDAYA_length 1

#define ROM_TIMESETTINGA_POS 41
#define ROM_TIMESETTINGA_length 1

#define ROM_ONTIMEA_POS 42
#define ROM_ONTIMEA_length 2

#define ROM_DURATIONA_POS 44
#define ROM_DURATIONA_length 2

rt_mutex_t data_lock = RT_NULL;
at24cxx_device_t erom = RT_NULL;
//TODO WRITE READ RECHECK
uint32_t cmp[16];
uint32_t timer_s[8];
uint8_t weekday_s[4];
int refresh_data = 0;

void data_init()
{

    data_lock = rt_mutex_create("data_lock", RT_IPC_FLAG_FIFO);
    RT_ASSERT(data_lock != RT_NULL);
    // time_t *now; /* Save the current time value obtained */
    erom = at24cxx_init("i2c1", 0);
    if (at24cxx_check(erom) == RT_ERROR)
    {
        at24cxx_deinit(erom);
        erom = RT_NULL;
        rt_kprintf("EROM打开失败！您的所有设置在断电后消失");
        return;
    }
    uint8_t buf[64];
    at24cxx_read(erom, 0, buf, 64);
    // now = (time_t *)buf;
    int t, tt, ttt, tttt;
    for (size_t i = 0; i < 4; i++)
    {
        t = *(uint16_t *)(buf + ROM_PIA_I_POS + 8 * i);
        tt = (*(uint16_t *)(buf + ROM_PIA_I_POS + 8 * i + 2));
        ttt = (*(uint16_t *)(buf + ROM_PIA_I_POS + 8 * i + 4));
        tttt = (*(uint16_t *)(buf + ROM_PIA_I_POS + 8 * i + 6));
        if (i == 1)
        {
            cmp[i * 4] = t * 10;
            cmp[i * 4 + 1] = tt;
            cmp[i * 4 + 2] = ttt * 12;
            cmp[i * 4 + 3] = tttt > 999 ? 999 : tttt;
        }

        senddata[PROTECT_SETTING_START + i * 4] = t;
        senddata[PROTECT_SETTING_START + i * 4 + 1] = tt;
        senddata[PROTECT_SETTING_START + i * 4 + 2] = ttt;
        senddata[PROTECT_SETTING_START + i * 4 + 3] = tttt > 999 ? 999 : tttt;

        //TODO sendvalue
        if (t == 0 && ttt == 0)
        {

            rt_kprintf("%d路保护设置：关!\n", i);
        }
        else
        {
            rt_kprintf("%d路保护设置：保护电流一段%dmA,保护时间%dtick,保护电流二段%dmA,保护时间%ds", i, t, tt, ttt, tttt);
        }
        t = *(uint16_t *)(buf + ROM_WEEKDAYA_POS + 6 * i);
        tt = (*(uint16_t *)(buf + ROM_WEEKDAYA_POS + 6 * i + 2));
        ttt = (*(uint16_t *)(buf + ROM_WEEKDAYA_POS + 6 * i + 4));
        senddata[TIMER_SETTING_START + i * 3] = t;
        senddata[TIMER_SETTING_START + i * 3 + 1] = tt;
        senddata[TIMER_SETTING_START + i * 3 + 2] = ttt;
        weekday_s[i] = (t & 0xff00) >> 8;
        timer_s[i * 2] = (t & 0x1) * 43200 + tt;
        timer_s[i * 2 + 1] = (((t & 0x2) >> 1) * 43200 + ttt + (t & 0x1) * 43200 + tt) % 86400;
        rt_kprintf("%d路定时设置：星期设置：%d,开始时间：%ld, 时长：%ld\n", i, (t & 0xff00) >> 8, (t & 0x1) * 43200 + tt, (t & 0x2) * 43200 + ttt);
    }
    char string[1024] = {0};

    for (size_t i = 0; i < (sizeof(senddata) / sizeof(int)) - 1; i++)
    {
        sprintf(&string[strlen(string)], "%d,", senddata[i]);
    }
    rt_kprintf("senddata=%s", string);

    // rt_kprintf("上次通电时间：%s\n", ctime(now));
    // // // char *string=ctime(&now);
    // // /* Obtain Time */
    // while (time(now) == (time_t)-1)
    // {
    // };

    // at24cxx_write(erom, 0, now, sizeof(time_t));
    // senddata[MODE_SETTING_START] = buf[ROM_MODE_POS];
}
void switch_on(int data)
{
    // rt_kprintf("swtich_on senddata[0]=%d, data= %d \n", senddata[0], data);
    // rt_kprintf("exp1=%d, exp1= %d \n", senddata[0], data);
    data = data & 0xf;
    if (data == (data & (senddata[0] & 0xf)))
    {
        return;
    }
    rt_enter_critical();
    senddata[0] |= data;
    if (data & 0x1 != 0)
    {
        senddata[0] &= ~(0xff << 4);
    }
    if (data & 0x1)
    {
        PORT_SetBits(PortA, Pin12);
    }
    if (data & 0x2)
    {
        PORT_SetBits(PortC, Pin13);
    }
    if (data & 0x4)
    {
        PORT_SetBits(PortC, Pin14);
    }
    if (data & 0x8)
    {
        PORT_SetBits(PortC, Pin15);
    }
    rt_exit_critical();
}

void switch_off(int data)
{
    data = data & 0xf;
    if (data & senddata[0] == 0)
    {
        return;
    }
    rt_kprintf("swtich_off senddata[0]=%d \n", senddata[0]);
    rt_enter_critical();
    senddata[0] = senddata[0] & (~data);
    if (data & 0x1)
    {
        PORT_ResetBits(PortA, Pin12);
    }
    if (data & 0x2)
    {
        PORT_ResetBits(PortC, Pin13);
    }
    if (data & 0x4)
    {
        PORT_ResetBits(PortC, Pin14);
    }
    if (data & 0x8)
    {
        PORT_ResetBits(PortC, Pin15);
    }
    rt_exit_critical();
}

void protect_set(uint16_t line, uint16_t bound_I, uint16_t time_I, uint16_t bound_II, uint16_t time_II)
{
    rt_kprintf("%d路保护设置：保护电流一段%dmA,保护时间%dtick,保护电流二段%dmA,保护时间%ds", line, bound_I, time_I, bound_II, time_II);
    if (line > 3)
        return;
    RT_ASSERT(data_lock != RT_NULL);
    rt_mutex_take(data_lock, RT_WAITING_FOREVER);
    senddata[PROTECT_SETTING_START + line * 4] = bound_I;
    senddata[PROTECT_SETTING_START + line * 4 + 1] = time_I;
    senddata[PROTECT_SETTING_START + line * 4 + 2] = bound_II;
    senddata[PROTECT_SETTING_START + line * 4 + 3] = time_II;
    uint16_t buf[4];
    buf[0] = bound_I;
    buf[1] = time_I;
    buf[2] = bound_II;
    buf[3] = time_II;
    if (erom != RT_NULL)
    {
        at24cxx_write(erom, ROM_PIA_I_POS + 8 * line, (uint8_t *)buf, 8);
    }
    rt_enter_critical();
    cmp[line * 4] = bound_I * 10;
    cmp[line * 4 + 1] = time_I;
    cmp[line * 4 + 2] = bound_II * 12;
    cmp[line * 4 + 3] = time_II;
    rt_exit_critical();
    rt_mutex_release(data_lock);
}

void protect_trigger(uint16_t lineset)
{
    lineset = lineset & 0xff;
    if (PORT_GetBit(PortA, Pin12) == Reset)
    {
        lineset &= ~0x33;
    }
    if (PORT_GetBit(PortC, Pin15) == Reset)
    {
        lineset &= ~0x8;
    }
    if (lineset == 0)
    {
        return;
    }
    state_changed = 1;
    uint8_t temp = ((lineset >> 4) | lineset) & 0xf;
    rt_enter_critical();
    senddata[0] &= ~0x1;
    senddata[0] |= lineset << 4;
    if (temp & 0x33)
    {
        PORT_ResetBits(PortA, Pin12);
    }
    if (temp & 0x8)
    {
        PORT_ResetBits(PortC, Pin15);
    }
    rt_exit_critical();
}
void setting_reset()
{
    RT_ASSERT(data_lock != RT_NULL);
    rt_mutex_take(data_lock, RT_WAITING_FOREVER);
    uint8_t buf[52] = {0};
    if (erom != RT_NULL)
    {
        at24cxx_write(erom, ROM_PIA_I_POS, buf, 52);
    }
    rt_mutex_release(data_lock);
}

MSH_CMD_EXPORT(setting_reset, reset setting);

void set_protect(int argc, char *argv[])
{
    if (argc > 5)
    {
        protect_set(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
    }
}

MSH_CMD_EXPORT(set_protect, set protect setting);

//TODO burst mode 同步时间设置
void timer_set(int line, uint8_t weekday, uint32_t starttime, uint32_t duration)
{
    rt_kprintf("%d路定时设置：星期设置：%d,开始时间：%ld,时长：%ld\n", line, weekday, starttime, duration);
    if (line > 3 || duration > 86400 || starttime > 86400)
        return;
    RT_ASSERT(data_lock != RT_NULL);
    uint8_t tsetting = (starttime / 43200) | (duration / 43200) << 1;
    uint16_t b_starttime = starttime % 43200, b_duration = duration % 43200;
    uint8_t buf[6];
    buf[0] = tsetting;
    buf[1] = weekday;
    *(uint16_t *)&buf[2] = b_starttime;
    *(uint16_t *)&buf[4] = b_duration;
    rt_mutex_take(data_lock, RT_WAITING_FOREVER);
    *(uint8_t *)(&senddata[TIMER_SETTING_START + line * 3]) = tsetting;
    *((uint8_t *)(&senddata[TIMER_SETTING_START + line * 3]) + 1) = weekday;
    senddata[TIMER_SETTING_START + line * 3 + 1] = b_starttime;
    senddata[TIMER_SETTING_START + line * 3 + 2] = b_duration;
    weekday_s[line] = weekday;
    timer_s[line * 2] = starttime;
    timer_s[line * 2 + 1] = (starttime + duration) % 86400;
    if (erom != RT_NULL)
    {
        at24cxx_write(erom, ROM_WEEKDAYA_POS + 6 * line, (uint8_t *)buf, 6);
    }

    rt_mutex_release(data_lock);
}

void set_timer(int argc, char *argv[])
{
    if (argc > 3)
    {
        timer_set(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    }
}

// void set_mode(int mode)
// {

//     if (mode == senddata[MODE_SETTING_START])
//     {
//         return;
//     }
//     if (mode = 1)
//     {
//         rt_mutex_take(data_lock, RT_WAITING_FOREVER);
//         senddata[MODE_SETTING_START] = 1;
//         uint8_t buf = mode;
//         if (erom != RT_NULL)
//         {
//             at24cxx_write(erom, ROM_MODE_POS, &buf, 1);
//         }
//         rt_mutex_release(data_lock);
//     }
//     if (mode = 0)
//     {
//         rt_mutex_take(data_lock, RT_WAITING_FOREVER);
//         senddata[MODE_SETTING_START] = 0;
//         uint8_t buf = mode;
//         if (erom != RT_NULL)
//         {
//             at24cxx_write(erom, ROM_MODE_POS, &buf, 1);
//         }
//         rt_mutex_release(data_lock);
//     }
// }

MSH_CMD_EXPORT(set_timer, set protect setting);

// void protect_trigger(uint16_t lineset)
// {
//     state_changed = 1;
//     lineset = lineset & 0xf;
//     rt_enter_critical();
//     senddata[0]&=~lineset;
//     senddata[0]|=lineset<<4;
//     if (senddata[MODE_SETTING_START] == 0)
//     {
//         if (lineset & 0x1)
//         {
//             PORT_ResetBits(PortA, Pin12);
//         }
//         if (lineset & 0x2)
//         {
//             PORT_ResetBits(PortC, Pin13);
//         }
//         if (lineset & 0x4)
//         {
//             PORT_ResetBits(PortC, Pin14);
//         }
//         if (lineset & 0x8)
//         {
//             PORT_ResetBits(PortC, Pin15);
//         }
//     }
//     else
//     {
//         PORT_ResetBits(PortA, Pin12);
//         PORT_ResetBits(PortC, Pin13);
//         PORT_ResetBits(PortC, Pin14);
//     }
//     rt_exit_critical();
//     rt_kprintf("保护启动!\n");
// }
