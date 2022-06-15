#ifndef __TM1650_H
#define __TM1650_H
#include <rtthread.h>
#include "hc32_ddl.h"
#include <rtdevice.h>

typedef int int16;

typedef unsigned short uint16;

typedef long int32;

typedef unsigned long uint32;

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;


typedef uint32_t  ulong;
typedef uint8_t  uchar;




#define				CLK_H						PORT_SetBits(PortB, Pin09)
#define				CLK_L						PORT_ResetBits(PortB, Pin09)

#define				STB_H						PORT_SetBits(PortC, Pin12)
#define				STB_L						PORT_ResetBits(PortC, Pin12)

#define				SDA_READ				PORT_GetBit(PortB, Pin08)

#define stb PCout(7)
#define dio_rd PORT_GetBit(PortB, Pin08)
#define dio_set PORT_SetBits(PortB, Pin08)
#define dio_crl PORT_ResetBits(PortB, Pin08)




typedef	union
{
	u16  LED;
   struct {
                uint16   	_LED_Watch :  1;    //�鿴   ��λ
                uint16  	_LED_Set   :  1;    //����
                uint16  	_LED_Warning :1;    //����
                uint16  	_LED_Timer :  1;    //��ʱ
                uint16  	_LED_L1A :    1;    //��һ������ܵ���/����ָʾ�ƣ�����������һ����һ����һ�α�ʾ����
                uint16   	_LED_L1S :    1;    //��һ���������/����ָʾ�ƣ�����������ʾ�롢һ����һ����һ�α�ʾ����
                uint16  	_LED_L2A :    1;    //�ڶ�������ܵ���/����ָʾ�ƣ�����������һ����һ����һ�α�ʾ����
                uint16   	_LED_L2S :    1;    //�ڶ����������/����ָʾ�ƣ�����������ʾ�롢һ����һ����һ�α�ʾ����
                uint16  	_LED_L3A :    1;    //����������ܵ���/����ָʾ�ƣ�����������һ����һ����һ�α�ʾ����
                uint16   	_LED_L3S :    1;    //�������������/����ָʾ�ƣ�����������ʾ�롢һ����һ����һ�α�ʾ����	
                uint16  	_LED_L4A :    1;    //����������ܵ���/����ָʾ�ƣ�����������һ����һ����һ�α�ʾ����
                uint16   	_LED_L4S :    1;    //�������������/����ָʾ�ƣ�����������ʾ�롢һ����һ����һ�α�ʾ����	
								uint16   	_F500ms  :    1;
								uint16   	_F1S     :    1;
		            
		 
	}bits;
}_LEDLIGHT;

extern _LEDLIGHT LEDLIGHT;
extern uchar cnt,key1629buf[4];
extern u8  Tm500msCnt;  //
extern u8  Tm1sCnt; 	
#define LED_Watch    LEDLIGHT.bits._LED_Watch
#define LED_Set  	   LEDLIGHT.bits._LED_Set
#define LED_Warning  LEDLIGHT.bits._LED_Warning
#define LED_Timer  	 LEDLIGHT.bits._LED_Timer
#define LED_L1A      LEDLIGHT.bits._LED_L1A
#define LED_L1S      LEDLIGHT.bits._LED_L1S
#define LED_L2A      LEDLIGHT.bits._LED_L2A
#define LED_L2S      LEDLIGHT.bits._LED_L2S
#define LED_L3A      LEDLIGHT.bits._LED_L3A
#define LED_L3S  		 LEDLIGHT.bits._LED_L3S
#define LED_L4A      LEDLIGHT.bits._LED_L4A
#define LED_L4S  		 LEDLIGHT.bits._LED_L4S
#define F500ms  		 LEDLIGHT.bits._F500ms
#define F1S  		 		 LEDLIGHT.bits._F1S










uchar Scan_Key(void);
void get_1629key(void);
void disp1629(void);
void TM1629B_Init(void);
void smg_BtoD(u8 d,ulong val,u8  line);
void GET_LED(void);
void disp_3wsmg(u8 *p,u8 line);
void CallTm1629();
void disp_set3wsmg(u8 w,u8 *p, u8 line);
void Disp_SetSmg(u8 d,ulong val,u8  line);















#endif  

