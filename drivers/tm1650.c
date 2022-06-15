#include "tm1650.h"

void TM1650_DATAINPUT_init(void) //????I/O??????
{
    rt_pin_mode(0x19, PIN_MODE_INPUT);
}

void I2CStart(void) //????
{
    CLK_H;
    DIO_H;
    rt_thread_delay(5);
    DIO_L;
}

void I2Cask(void) //ACK??
{
    unsigned char timeout = 1;
    CLK_H;
    rt_thread_delay(5);
    CLK_L;
    TM1650_DATAINPUT_init();
    while ((SDA_READ) && (timeout <= 250))
    {
        timeout++;
    }
    TM1650_Init();
    rt_thread_delay(5);
    CLK_L;
}

void I2CStop(void) //????
{
    CLK_H;
    DIO_L;
    rt_thread_delay(5);
    DIO_H;
}

void I2CWrByte(unsigned char oneByte) //?????????,????
{
    unsigned char i;
    CLK_L;
    rt_thread_delay(1);
    for (i = 0; i < 8; i++)
    {
        if (oneByte & 0x80)
            DIO_H;
        else
            DIO_L;
        oneByte = oneByte << 1;
        CLK_L;
        rt_thread_delay(2);
        CLK_H;
        rt_thread_delay(2);
        CLK_L;
    }
}

unsigned char Scan_Key(void) // ????
{
    unsigned char i;
    unsigned char rekey = 0;
    I2CStart();
    I2CWrByte(0x49); //?????
    I2Cask();
    TM1650_DATAINPUT_init();
    CLK_H;
    rt_thread_delay(1);
    //DIO_H;
    for (i = 0; i < 8; i++)
    {
        CLK_H;
        rekey = rekey << 1;
        if (SDA_READ)
        {
            rekey++;
        }
        rt_thread_delay(2);
        CLK_L;
    }
    TM1650_Init();
    rt_thread_delay(2);
    I2Cask();
    I2CStop();
    return (rekey);
}

void TM1650_Set(unsigned char add, unsigned char dat) //68 ,6A,6C,6E,
{
    I2CStart();
    I2CWrByte(add); //???????
    I2Cask();
    I2CWrByte(dat);
    I2Cask();
    I2CStop();
}

void TM1650_Init(void)
{
    rt_pin_mode(0x18, PIN_MODE_OUTPUT);
    rt_pin_mode(0x19, PIN_MODE_OUTPUT);
}
