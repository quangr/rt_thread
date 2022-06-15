#ifndef __TM1650_H
#define __TM1650_H
#include "hc32_ddl.h"
#include <rtthread.h>
#include <rtdevice.h>




#define				CLK_H						PORT_SetBits(PortB, Pin08)
#define				CLK_L						PORT_ResetBits(PortB, Pin08)

#define				DIO_H						PORT_SetBits(PortB, Pin09)
#define				DIO_L						PORT_ResetBits(PortB, Pin09)

#define				SDA_READ				PORT_GetBit(PortB, Pin09)


#define KEY_BACK  		84//GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14)// °´¼ü
#define KEY_UP  		  92//GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_15)// 
//#define KEY_COILSON  	GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10)//
//#define KEY_COILSOFF  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_13)// 
#define KEY_ENTER  		76//GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9)// 
//#define KEY_ADD     	GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)// 
//#define KEY_DEC   		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6)// 
#define KEY_DOWN  		68//	GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8)// 



unsigned char Scan_Key(void);
void TM1650_Init(void);
void TM1650_Set(unsigned char add, unsigned char dat);























#endif  

