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
#include <board_config.h>
#include <rtthread.h>
#include "arm_math.h"
#include <rtdevice.h>

//rt_event_t mqtt_event=RT_NULL;

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

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of template project
 * @param  None
 * @retval int32_t return value, if needed
 */
// static const char send_data[] = "I'm on the top I can't get back.";
// static const char url[20] = "81.68.161.73";

// int32_t net_test(void)
// {

//     char *recv_data;
//     struct hostent *host;
//     struct sockaddr_in server_addr;
//     int sock = -1, bytes_received;
//     if ((sock = socket(AF_AT, SOCK_STREAM, IPPROTO_TCP)) < 0)
//     {
//         rt_kprintf("Create socket failed!");
//         goto __exit;
//     }
//     int ret;
//     host = gethostbyname(url);
//     server_addr.sin_family = AF_AT;
//     server_addr.sin_port = htons(5000);
//     server_addr.sin_addr = *((struct in_addr *)host->h_addr);
//     ;
//     rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
//     if ((ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) < 0)
//     {
//         rt_kprintf("Connect <%d> fail! ret:%d", sock, ret);
//         goto __exit;
//     }
//     recv_data = rt_malloc(1024);
//     rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
//     while (1)
//     {
//         bytes_received = recv(sock, recv_data, 1024 - 1, 0);
//         if (bytes_received < 0)
//         {
//             closesocket(sock);
//             rt_kprintf("\nreceived error,close the socket.\r\n");
//             rt_free(recv_data);
//             break;
//         }
//         else if (bytes_received == 0)
//         {
//             closesocket(sock);
//             rt_kprintf("\nreceived error,close the socket.\r\n");
//             rt_free(recv_data);
//             break;
//         }
//         recv_data[bytes_received] = '\0';
//         if (strncmp(recv_data, "q", 1) == 0 || strncmp(recv_data, "Q", 1) == 0)
//         {
//             closesocket(sock);
//             rt_kprintf("\n got a 'q' or 'Q',close the socket.\r\n");
//             rt_free(recv_data);
//             break;
//         }
//         else
//         {
//             if (strncmp(recv_data, "a", 1) == 0 || strncmp(recv_data, "A", 1) == 0)
//             {
//                 rt_pin_write(LED_PIN, PIN_LOW);
//             }
//             if (strncmp(recv_data, "b", 1) == 0 || strncmp(recv_data, "B", 1) == 0)
//             {
//                 rt_pin_write(LED_PIN, PIN_HIGH);
//             }
//             rt_kprintf("\nReceived data = %s ", recv_data);
//         }
//         ret = send(sock, send_data, strlen(send_data), 0);
//         if (ret < 0)
//         {
//             closesocket(sock);
//             rt_kprintf("\nsend error,close the socket.\r\n");
//             rt_free(recv_data);
//             break;
//         }
//         else if (ret == 0)
//         {
//             rt_kprintf("\n Send warning,send function return 0.\r\n");
//         }
//     }
// __exit:
//     rt_kprintf("lose");
// }
// // MSH_CMD_EXPORT(net_test, a tcp client sample);

// void d_write(unsigned char flag,unsigned char dat)
// {
//     rt_pin_write(cs, PIN_LOW);
//     rt_pin_write(cd, flag);
//     rt_pin_write(enwr, PIN_LOW);
//     rt_pin_write(enrd, PIN_HIGH);
//     unsigned short data=(PORT_GetData(PortB)&0xff00)|(dat&0xff);
//     PORT_SetData(PortB,data);
//     rt_pin_write(enwr, PIN_HIGH);
//     rt_pin_write(cs, PIN_HIGH);
// }
// void WriteAddress(unsigned char x1,unsigned char y1)
// {
// 	unsigned char a,h;
// 	if(x1 > 127)x1 = 127;
// 	x1 += 0;
// 	a = x1 & 0x0f;
// 	h = x1 / 0x10;
// 	if(y1 > 7) y1 = 7;
// 	d_write(0,0x40);
//  	d_write(0,0xb0 | y1);
//  	d_write(0,0x10 + h);
//  	d_write(0,0x0 + a);	
// }
// void Clear(unsigned char xnub,unsigned char ynub,unsigned char px,unsigned char py)
// {
// 	unsigned char z,i,c = 0;

// 	z = px;

// 	for(i = 0; i< ynub; i++)
// 	{
// 		 WriteAddress(z,py++);
// 		 for(c = 0;c < xnub;c++)
// 		 {
//                 //write(1,0x00);
// 				 d_write(1,0xf0);	
				 
// 		 } 	

// 	}	
	
// }

// int32_t net_test2(void)
// {
//     //	uchar n;
//     //	if(contrast < 32 || contrast > 60)n = 0x2f;
//     //	else n = (uchar)contrast;
//     rt_pin_mode(0x0f, PIN_MODE_OUTPUT);
//     rt_pin_mode(0x10, PIN_MODE_OUTPUT);
//     rt_pin_mode(0x11, PIN_MODE_OUTPUT);
//     rt_pin_mode(0x12, PIN_MODE_OUTPUT);
//     rt_pin_mode(0x13, PIN_MODE_OUTPUT);
//     rt_pin_mode(0x14, PIN_MODE_OUTPUT);
//     rt_pin_mode(0x15, PIN_MODE_OUTPUT);
//     rt_pin_mode(0x16, PIN_MODE_OUTPUT);
//     rt_pin_mode(0x17, PIN_MODE_OUTPUT);

//     rt_pin_mode(enwr, PIN_MODE_OUTPUT);
//     rt_pin_mode(enrd, PIN_MODE_OUTPUT);
//     rt_pin_mode(cs, PIN_MODE_OUTPUT);
//     rt_pin_mode(cd, PIN_MODE_OUTPUT);
//     rt_pin_mode(reset, PIN_MODE_OUTPUT);

//     rt_pin_write(cs, PIN_LOW);
//     rt_thread_delay(2);
//     rt_pin_write(reset, PIN_LOW);
//     rt_thread_delay(2);
//     rt_pin_write(reset, PIN_HIGH);
//     rt_thread_delay(200);
//     d_write(0, 0xAE); //Display OFF //ÏÔÊ¾¹Ø
//     d_write(0, 0x40); //Set Display Start Line = com0
//     d_write(0, 0xA2); //1/64 Duty 1/9 Bias //Æ«Ñ¹±ÈÉè¶¨
//     d_write(0, 0xA0); //ADC select  --> right//LCDÕýÏòÏÔÊ¾
//     d_write(0, 0xC8); //com1 --> com64//µÚ0ÐÐÔÚÆÁÄ»ÏÂ·½
//     d_write(0, 0x23); //Íâ²¿Rb/Ra//¶Ô±È¶È´Öµ÷
//     d_write(0, 0x81); //Sets V0 //½øÈëÏ¸µ÷ÃüÁî
//     d_write(0, 0x2F); //ÄÚ²¿µçÎ»Æ÷µ÷½Ú¶Ô±È¶È
//     d_write(0, 0x2F); //voltage follower ON  regulator ON  booster ON//¿ª¹ØÄÚ²¿µçÔ´
//     d_write(0, 0xA6); //Normal Display (not reverse dispplay)//Õý³£ÏÔÊ¾
//     d_write(0, 0xA4); //Entire Display Disable//²»È«ÆÁÏÔÊ¾

//     //LcmClear();
//     d_write(0, 0xAF); //Display ON //ÏÔÊ¾¿ª
//     rt_thread_delay(2);
//     Clear(128, 8, 0, 0);
//     rt_thread_delay(2);
//               rt_pin_mode(0x0f, PIN_MODE_OUTPUT);

//     rt_pin_write(0x0f, PIN_HIGH);

// }


extern q15_t sint[8] ;
extern q15_t cost[8] ;
void get_vol()
{
    q15_t input1[8]={4096,4096,4096,4096,4096,4096,4096,4096},input2[8]={4096,4096,4096,4096,-4096,-4096,-4096,-4096};
    int64_t s_result, c_result, p_result;
    q31_t tt[8];
    q31_t so;
    arm_dot_prod_q15(input1, sint, 8, &s_result);
    arm_dot_prod_q15(input2, cost, 8, &c_result);
    tt[0] = (q31_t)(c_result);
    tt[1] = (q31_t)(s_result);

    arm_cmplx_mag_q31(tt , &so, 1);
  
}

MSH_CMD_EXPORT(get_vol, a vol test);
/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
