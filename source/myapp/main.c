/**
 *******************************************************************************
 * @file  main.c
 * @brief Main program template.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2020-06-30        CDT         First version
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "hc32_ddl.h"
#include <tm1650.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <sys\socket.h>
#include "arm_math.h"
#include "at24cxx.h"

#define LED_PIN (0x2D)

#define LED0_PORT (PortE)
#define LED0_PIN (Pin01)

#define LED0_ON() (PORT_SetBits(LED0_PORT, LED0_PIN))
#define LED0_OFF() (PORT_ResetBits(LED0_PORT, LED0_PIN))
int16_t testInput[16] = {2304, 2284, 2229, 2145, 2048, 1950, 1866, 1811, 1792, 1811, 1866, 1950, 2048, 2145, 2229, 2284};
int16_t testInput2[32] = {2048, 2088, 2127, 2162, 2194, 2219, 2239, 2250, 2254, 2250, 2239, 2219, 2194, 2162, 2127, 2088, 2048, 2007, 1968, 1933, 1901, 1876, 1856, 1845, 1841, 1845, 1856, 1876, 1901, 1933, 1968, 2007};
q15_t testInput_q15[32], testInput2_q15[32];
q15_t testOutput_q15[64];
int16_t testOutput[64];
q31_t q31test[32];
q15_t sintt[8] = {0, 12539, 23170, 30273, 32767, 30273, 23170, 12539};
q15_t costt[8] = {32767, 30274, 23170, 12540, 0, -12540, -23170, -30274};
void data_init();

#if (defined(EC200X)) || (defined(AIR720))
volatile extern uint16_t setting_changed;
#endif
volatile extern uint32_t timechanged;

volatile extern uint32_t timer_s[8];
volatile extern uint8_t weekday_s[4];

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
void switch_on(int data);
void switch_off(int data);

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
/**
 * @brief ICG parameters configuration
 */
/* The ICG area is filled with F by default, HRC = 16MHZ,
    Please modify this value as required */
//#if defined ( __GNUC__ ) && !defined (__CC_ARM) /* GNU Compiler */
//const uint32_t u32ICG[] __attribute__((section(".icg_sec"))) =
//#elif defined (__CC_ARM)
//const uint32_t u32ICG[] __attribute__((at(0x400))) =
//#elif defined (__ICCARM__)
//__root const uint32_t u32ICG[] @ 0x400 =
//#else
//#error "unsupported compiler!!"
//#endif
//{
//    /* ICG 0~ 3 */
//    0xFFFFFFFFUL,
//    0xFFFFFFFFUL,
//    0xFFFFFFFFUL,
//    0xFFFFFFFFUL,
//    /* ICG 4~ 7 */
//    0xFFFFFFFFUL,
//    0xFFFFFFFFUL,
//    0xFFFFFFFFUL,
//    0xFFFFFFFFUL,
//};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of template project
 * @param  None
 * @retval int32_t return value, if needed
 */
volatile extern uint32_t senddata[64];
int mqtt_test(int argc, char *argv[]);
void TM1629B_Init(void);
void KeyScan(void);
void rt_read_timer(uint32_t *nowtime, uint8_t *nowweek);
extern void dma_test(void);
//TODO 时钟校准会影响判断吗？？？
//TODO 时钟启动初始化不动了问题
int32_t main(void)
{
  data_init();
  DebugUnset();
  rt_thread_t tid = rt_thread_create("dma_thread", dma_test, (void *)RT_NULL,
                                     2048, 0, 20);
  if (tid != RT_NULL)
  {
    rt_kprintf("enable dma");
    rt_thread_startup(tid);
  }
  //    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
  //    config.baud_rate = BAUD_RATE_115200;
  //    config.rx_bufsz = BSP_UART2_RX_BUFSIZE;
  //   config.tx_bufsz = BSP_UART2_TX_BUFSIZE;
  //	  rt_kprintf("first");
  //    rt_device_control(&uart_device->parent, RT_DEVICE_CTRL_CONFIG, &config);
  //  rt_err_t result = rt_device_open(uart_device, RT_DEVICE_OFLAG_RDONLY|RT_DEVICE_FLAG_DMA_RX);
  TM1629B_Init();
  //      net_test2();
  // TM1650_Init();
  // TM1650_Set(0x48,0x51);
  // TM1650_Set(0x68,0x5b);
  rt_pin_mode(0x27, PIN_MODE_OUTPUT);

  int i = 0;
  int16_t testInput[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int16_t testInput2[16] = {3505, 3207, 2735, 2162, 1583, 1080, 727, 576, 653, 947, 1413, 1981, 2564, 3072, 3429, 3581};
  uint8_t tempdata_on = 0, tempdata_off = 0;
  int ii = 0;
  uint32_t nowtime;
  uint32_t lasttime;
  uint8_t nowweek;
  rt_read_timer(&nowtime, &nowweek);
  lasttime = nowtime;
  rt_pin_mode(0x0f, PIN_MODE_OUTPUT);
  while (1)
  {
    /* Printout time information */
    ii++;
#ifndef RENODE
    KeyScan();
#endif
    if (ii % 10 == 0)
    {
#ifndef RENODE
      CallTm1629();
#endif
    };
    if (ii >= 100)
    {
      // PORT_SetBits(PortA, Pin15);
      rt_read_timer(&nowtime, &nowweek);
      // PORT_ResetBits(PortA, Pin15);
      // rt_kprintf("nowtime:%d,lasttime %d",nowtime,lasttime);
      //     rt_kprintf("now tick:%d",rt_tick_get());
      ii = 0;
      if ((nowtime + 86400 - lasttime) % 86400 == 1)
      {
        rt_kprintf("week:%d,nowtime %d", nowweek, nowtime);
        rt_enter_critical();
        for (size_t i = 0; i < 4; i++)
        {

          if (weekday_s[i] & (1 << (nowweek - 1)))
          {
            if (nowtime == timer_s[i * 2])
            {
              tempdata_on |= (1 << i);
            }
            if (nowtime == timer_s[i * 2 + 1])
            {
              tempdata_off |= (1 << i);
            }
          }
        }
        rt_exit_critical();
        lasttime = nowtime;
      }
      else
      {
        if (nowtime != lasttime)
        {
          uint32_t timediff = (nowtime + 86400 - lasttime) % 86400;
          if (timediff < 15)
          {
            lasttime = nowtime;
            rt_enter_critical();
            for (size_t i = 0; i < 4; i++)
            {

              if (weekday_s[i] & (1 << (nowweek - 1)))
              {
                // rt_kprintf("nowtime: %ld\n",nowtime);
                // rt_kprintf("%dstoptime: %ld\n",i,timer_s[i*2+1]);
                // rt_kprintf("%dstarttime: %ld\n",i,timer_s[i*2]);
                if (nowtime > lasttime)
                {
                  if ((nowtime >= timer_s[i * 2]) && (lasttime < timer_s[i * 2]))
                  {
                    tempdata_on |= (1 << i);
                  }
                  if ((nowtime >= timer_s[i * 2 + 1]) && (lasttime < timer_s[i * 2 + 1]))
                  {
                    tempdata_off |= (1 << i);
                  }
                }
                else
                {
                  if ((nowtime >= timer_s[i * 2]) || (lasttime < timer_s[i * 2]))
                  {
                    tempdata_on |= (1 << i);
                  }
                  if ((nowtime >= timer_s[i * 2 + 1]) || (lasttime < timer_s[i * 2 + 1]))
                  {
                    tempdata_off |= (1 << i);
                  }
                }
              }
            }
            rt_exit_critical();
          }
          else if (timediff > 86400 - 15 && timediff < 86400)
          {
          }
          else
          {
            lasttime = nowtime;
          }
        }

        // rt_kprintf("week:%d",nowweek);
      }
      if (tempdata_on != 0 || tempdata_off != 0)
      {
        if (tempdata_on != 0)
        {
          rt_kprintf("%dontrigger\n", tempdata_on);
          switch_on(tempdata_on);
          tempdata_on = 0;
        }
        if (tempdata_off != 0)
        {
          rt_kprintf("%dofftrigger\n", tempdata_off);
          switch_off(tempdata_off);
          tempdata_off = 0;
        }
#if (defined(EC200X)) || (defined(AIR720))
        setting_changed = 1;
#endif
      }
    };
    rt_thread_delay(RT_TICK_PER_SECOND / 100);

    // int64_t s_result,c_result;
    // rt_read_timer(&nowtime,&nowweek);
    // arm_sub_q15(testInput,testInput+8,testInput,8);
    // arm_dot_prod_q15(testInput,sintt,8,&s_result);
    // arm_dot_prod_q15(testInput,costt,8,&c_result);
    // rt_kprintf("直流：%ld，一阶： %ld \n", c_result, s_result);
    // if (senddata[49] == 0)
    // {
    //   for (size_t i = 0; i < 4; i++)
    //   {
    //     if (weekday_s[i] & (1 << nowweek))
    //     {
    //       // rt_kprintf("nowtime: %ld\n",nowtime);
    //       // rt_kprintf("%dstoptime: %ld\n",i,timer_s[i*2+1]);
    //       // rt_kprintf("%dstarttime: %ld\n",i,timer_s[i*2]);
    //       if (nowtime == timer_s[i * 2])
    //       {
    //         rt_kprintf("%dontrigger\n", i);
    //         tempdata_on |= (1 << i);
    //       }
    //       if (nowtime == timer_s[i * 2 + 1])
    //       {
    //         rt_kprintf("%dofftrigger\n", i);
    //         tempdata_off |= (1 << i);
    //       }
    //     }
    //   }
    //   if (tempdata_on != 0)
    //   {
    //     switch_on(tempdata_on);
    //     tempdata_on = 0;
    //   }
    //   if (tempdata_off != 0)
    //   {
    //     switch_off(tempdata_off);
    //     tempdata_off = 0;
    //   }
    // }
    // else
    // {
    //   if (weekday_s[0] & (1 << nowweek))
    //   {
    //     // rt_kprintf("nowtime: %ld\n",nowtime);
    //     // rt_kprintf("%dstoptime: %ld\n",i,timer_s[i*2+1]);
    //     // rt_kprintf("%dstarttime: %ld\n",i,timer_s[i*2]);
    //     if (nowtime == timer_s[0])
    //     {
    //       rt_kprintf("%dontrigger\n", i);
    //       tempdata_on |= 0x7;
    //     }
    //     if (nowtime == timer_s[1])
    //     {
    //       rt_kprintf("%dofftrigger\n", i);
    //       tempdata_off |= 0x7;
    //     }
    //   }
    //   if (tempdata_on != 0)
    //   {
    //     switch_on(tempdata_on);
    //     tempdata_on = 0;
    //   }
    //   if (tempdata_off != 0)
    //   {
    //     switch_off(tempdata_off);
    //     tempdata_off = 0;
    //   }
    // }

    //        rt_pin_write(0x2f, PIN_HIGH);
    // arm_shift_q15(testInput+i,3,testInput_q15,16);

    // q31_t mmoutput[32];
    // q15_t dc;
    // q31_t so,tt[2];

    // arm_mean_q15(testInput,16,&dc);
    // tt[0]=(q31_t)(c_result);
    // tt[1]=(q31_t)(s_result);
    // arm_cmplx_mag_q31(tt,&so,1);
    // rt_kprintf("直流：%d，一阶： %d \n", dc, so);

    // arm_mean_q15(testInput2,16,&dc);
    // arm_sub_q15(testInput2,testInput2+8,testInput2,8);
    // arm_dot_prod_q15(testInput2,sintt,8,&s_result);
    // arm_dot_prod_q15(testInput2,costt,8,&c_result);
    // rt_kprintf("直流：%ld，一阶： %ld \n", c_result, s_result);
    // tt[0]=(q31_t)(c_result);
    // tt[1]=(q31_t)(s_result);
    // arm_cmplx_mag_q31(tt,&so,1);
    // rt_kprintf("直流：%d，一阶： %d \n", dc, so);

    // i++;
    // rt_kprintf("1");

    //       rt_pin_write(0x2f, PIN_LOW);
    //        rt_thread_mdelay(1000);
    //	if(rt_device_read((rt_device_t) uart_device,0,&a,1)!=0){	rt_kprintf("'%c'",a);}else{rt_thread_delay(100);}
    //  rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    //		rt_pin_write(LED_PIN, PIN_LOW);
    //		rt_pin_write(LED_PIN, PIN_HIGH);
    //	rt_thread_mdelay(1000);
    //rt_kprintf("c");
  }
  /* add your code here */
  //        while(1){
  //		}
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
