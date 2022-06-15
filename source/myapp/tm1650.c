#include "tm1650.h"
#include "TM1629keyled.h"

_LEDLIGHT LEDLIGHT;

//                          0, 1     2    3    4    5    6    7    8    9    a     b    c    d    e    f ,  p    n,   -    Y    L   _   ����ʾ
const u8 smgtable[25] = {
	0x3f,
	0x06,
	0x5b,
	0x4f,
	0x66,
	0x6d,
	0x7d,
	0x07,
	0x7f,
	0x6f,
	0x77,
	0x7c,
	0x39,
	0x5e,
	0x79,
	0x71,
	0x73,
	0x37,
	0x40,
	0x6E,
	0x38,
	0x08,
	0x00,
	0x00,
	0x00,
};
u8 smg[16];
u16 smg_16h[4];
uchar cnt, key1629buf[4];
u8 Tm500msCnt; //
u8 Tm1sCnt;
extern uchar sel_bit;
extern u16 cope_keynub;
extern u16 keybackcnt;
extern u8 dismod;
extern u8 sel_line;
extern u8 sys_mainx;
extern uchar sel_bit;

void smg_BtoD(u8 d, ulong val, u8 line) //d == 0,��С����d == 1,���һλС����d == 2,���2λС����line�ڼ��������
{
	u8 w[3], f = 0, a, w1[3], n; //w[3]һ������ܹ�3λ   f = С�����־λ
	ulong v1, v;
	if (val > 99999)
		v = 99999;
	else
		v = val;
	if (d > 9)
	{
		if (d == 10)
		{
			w1[0] = smgtable[0];
			w1[1] = smgtable[0x0f];
			w1[2] = smgtable[0x0f];
		} //d == 10 off
		else if (d == 11)
		{
			w1[0] = smgtable[24];
			w1[1] = smgtable[0x00];
			w1[2] = smgtable[17];
		} //d == 11 0n
		else if (d == 12)
		{
			w1[0] = smgtable[24];
			w1[1] = smgtable[24];
			w1[2] = smgtable[24];
		} //d == 12  ����ܺ���������ʾ
		else if (d == 13)
		{
			w1[0] = smgtable[18];
			w1[1] = smgtable[16];
			w1[2] = smgtable[18];
		} //d == 13  P__
		else if (d == 14)
		{
			w1[0] = smgtable[19];
			w1[1] = smgtable[14];
			w1[2] = smgtable[5];
		} //d == 14  YES
		else if (d == 15)
		{
			w1[0] = smgtable[24];
			w1[1] = smgtable[17];
			w1[2] = smgtable[0];
		} //d == 15  NO
		else if (d > 19 && d < 25)
		{
			w1[0] = smgtable[16];
			w1[1] = smgtable[20];
			if (d == 20)
				w1[2] = 0;
			else
				w1[2] = smgtable[d % 10]; //����ʱ
		}
		else if (d > 30 && d < 35)
		{
			w1[0] = smgtable[16];
			w1[1] = smgtable[5];
			if (d == 30)
				w1[2] = 0;
			else
				w1[2] = smgtable[d % 10]; //����ʱ
		}
		else if (d == 35)
		{
			w1[0] = smgtable[3];
			w1[1] = smgtable[16];
			w1[2] = smgtable[0];
		} //d == 13  3P0  ��ƽ��
	}
	else
	{
		if (v > 9999)
		{
			if (d == 2)
			{
				v /= 100;
				f = 0;
			}
			else if (d == 1 || d == 0)
			{
				v = 999;
				f = 0;
			}
		} //����999��û��С����
		else if (v > 999)
		{
			if (d == 2)
			{
				v /= 10;
				f = 1;
			}
			else if (d == 1)
			{
				v = v / 10;
				f = 0;
			}
			else if (d == 0)
			{
				v = 999;
				f = 0;
			}
		}
		else
			f = d;
		v1 = v;
		w[0] = v / 100;
		v = v % 100;
		w[1] = v / 10;
		w[2] = v % 10;

		w1[0] = smgtable[w[0]]; //��һλҪ��ʾ�Ķ�
		w1[1] = smgtable[w[1]];
		w1[2] = smgtable[w[2]];
		if (d == 0)
		{
			if (v1 < 100)
			{
				w1[0] = 0;
			}
			if (v1 < 10)
			{
				w1[1] = 0;
			}
		}
		if (d == 1)
		{
			if (v1 < 100)
				w1[0] = 0;
			if (f == 1)
				w1[1] |= 0x80;
		}
		else if (d == 2)
		{
			if (f == 2)
				w1[0] |= 0x80;
			else if (f == 1)
				w1[1] |= 0x80;
		}
	}
	disp_3wsmg(w1, line);
}

void Disp_SetSmg(u8 d, ulong val, u8 line) //d == 0,��С����d == 1,���һλС����d == 2,���2λС����line�ڼ��������
{
	u8 w[5], f = 0, a, w1[5], n, *p, b; //w[3]һ������ܹ�3λ   f = С�����־λ
	ulong v1, v;
	if (val > 99999)
		v = 99999;
	else
		v = val;

	v1 = v;
	w[0] = v / 10000;
	v = v % 10000;
	w[1] = v / 1000;
	v = v % 1000;
	w[2] = v / 100;
	v = v % 100;
	w[3] = v / 10;
	w[4] = v % 10;
	w1[0] = smgtable[w[0]]; //��һλҪ��ʾ�Ķ�
	w1[1] = smgtable[w[1]];
	w1[2] = smgtable[w[2]];
	w1[3] = smgtable[w[3]];
	w1[4] = smgtable[w[4]];
	if (d == 0)
	{
		b = 2;
		p = &w1[2];
	}

	else if (d == 2)
	{
		w1[2] |= 0x80;
		if (line == sel_line)
		{
			if (sel_bit == 4)
			{
				b = 2;
				p = &w1[b];
			}
			else if (sel_bit == 3)
			{
				b = 2;
				p = &w1[b];
			}
			else
			{
				b = 0;
				p = &w1[b];
			}
		}
		else
		{
			b = 0;
			p = &w1[0];
		}
	}

	disp_set3wsmg(b, &w1[b], line); //b��������鱻��ʾ���ֽ�λ��ַ��w1����ʾ���ֽ�ָ��void disp_set3wsmg(u8 w,u8 *p, u8 line)
}

void GET_LED(void)
{
	if (LED_Watch)
		smg[7] |= 0x20;
	else
		smg[7] &= 0xdf;
	if (LED_Set)
		smg[5] |= 0x20;
	else
		smg[5] &= 0xdf;
	if (LED_Warning)
		smg[3] |= 0x20;
	else
		smg[3] &= 0xdf;
	if (LED_Timer)
		smg[1] |= 0x20;
	else
		smg[1] &= 0xdf;
	if (LED_L1A)
		smg[1] |= 0x10;
	else
		smg[1] &= 0xef;
	if (LED_L1S)
		smg[3] |= 0x10;
	else
		smg[3] &= 0xef;
	if (LED_L2A)
		smg[5] |= 0x10;
	else
		smg[5] &= 0xef;
	if (LED_L2S)
		smg[7] |= 0x10;
	else
		smg[7] &= 0xef;
	if (LED_L3A)
		smg[9] |= 0x10;
	else
		smg[9] &= 0xef;
	if (LED_L3S)
		smg[11] |= 0x10;
	else
		smg[11] &= 0xef;
	if (LED_L4A)
		smg[13] |= 0x10;
	else
		smg[13] &= 0xef;
	if (LED_L4S)
		smg[15] |= 0x10;
	else
		smg[15] &= 0xef;
}
void Delay_us2(u16 i) //us??
{
	u8 n = 5;
	for (; i > 0; i--)
	{
		n = 5;
		while (n)
			n--;
	}
}

/*****************************************************************************
 *��Ȩ��Ϣ��������΢�������޹�˾
 *�� �� ����TM1622-V1.0
 *��ǰ�汾��V1.0
 *MCU �ͺţ�STC12C5608AD
 *����������Keil uVision4
 *����Ƶ�ʣ�11.0592MHZ       
 *������ڣ�2013-09-09
 *�����ܣ�1.LCD������32SEG*8COM(��2��4COM LCD����ɣ����ֱ���ʾ0~9.
 *����������1.�˳���ΪTM1622����LCD��ʾ���򣬽����ο�֮�á�
            2.����ֱ��ʹ�ñ����̳�����ɾ�����ʧ�ģ�����˾���е��κ�����             
********************************************************************************/

//==================1629==========================

void TM1629B_Init(void)
{
	rt_pin_mode(0x18, PIN_MODE_OUTPUT_OD);
	rt_pin_mode(0x19, PIN_MODE_OUTPUT);
	rt_pin_mode(0x2c, PIN_MODE_OUTPUT);

	sys_mainx = 1;
	dismod = 1;
#ifdef SANXIANG
	phase_mod = 3;
#endif
#ifdef DANXIANG
	phase_mod = 1;
#endif
	JD1OUT = 1;

}

void TM1629B_DATAINPUT_init(void) //????I/O??????
{
	rt_pin_mode(0x18, PIN_MODE_INPUT);
}
void TM1629B_DATAOUTPUT_init(void) //????I/O??????
{
	rt_pin_mode(0x18, PIN_MODE_OUTPUT_OD);
}

/*********************************   
���ܣ� TM1629д����
**********************************/
void TM1629B_write(uchar wr_data0)
{
	uchar i;
	TM1629B_DATAOUTPUT_init();

	//  STB_H;
	//  CLK_H;
	//  dio_set;
	//	Delay_us2(1);
	//         STB_L;           //����Ч��Ƭѡ�źţ�Ƭѡ�ź��ǵ͵�ƽ��Ч��
	Delay_us2(1);
	for (i = 0; i < 8; i++) //��ʼ����8λ���ݣ�ÿѭ��һ�δ���һλ����
	{
		CLK_L;
		Delay_us2(1);
		if ((wr_data0 & 0x01) == 0x01)
			dio_set; //��λ���ݣ���λ��ǰ!
		else
			dio_crl;
		//   Delay_us2(1);
		wr_data0 >>= 1;
		Delay_us2(1);
		CLK_H;
		Delay_us2(1);
	}
	//	 dio_set;
	//	  Delay_us2(1);
	//		  STB_H;
}

void write_29(uchar wr_data0)
{
	uchar i;
	TM1629B_DATAOUTPUT_init();

	STB_H;
	CLK_H;
	dio_set;
	Delay_us2(1);
	STB_L; //����Ч��Ƭѡ�źţ�Ƭѡ�ź��ǵ͵�ƽ��Ч��
	Delay_us2(1);
	for (i = 0; i < 8; i++) //��ʼ����8λ���ݣ�ÿѭ��һ�δ���һλ����
	{
		CLK_L;
		Delay_us2(1);
		if ((wr_data0 & 0x01) == 0x01)
			dio_set; //��λ���ݣ���λ��ǰ!
		else
			dio_crl;
		Delay_us2(1);
		wr_data0 >>= 1;
		Delay_us2(1);
		CLK_H;
		Delay_us2(1);
	}
	dio_set;
	Delay_us2(1);
	STB_H;
}
/*********************************  
���ܣ��������ӳ��� ��y ����1BIT��������
**********************************/
unsigned char TM1629Bread(void)
{
	uchar d, y = 0;

	Delay_us2(1);

	//TM1629B_DATAINPUT_init();
	Delay_us2(1);
	STB_L;
	Delay_us2(1);
	//��DIO�øߣ���Ϊû�а�������ʱ����ֵĬ��Ϊ00H����ʱΪ�����ֵ������
	for (d = 0; d < 8; d++)
	{

		CLK_L;
		y >>= 1;
		Delay_us2(1);

		Delay_us2(1);
		if (dio_rd)
			y |= 0x80;
		else
		{
			y &= 0x7f;
			LED_L1S = 1;
		}
		CLK_H;
		Delay_us2(1);
	}

	// CLK_H;
	//STB_H;

	return (y);
}

void disp1629(void)
{
	uchar h, a = 0x20;
	GET_LED();
	TM1629B_DATAOUTPUT_init();

	/*	for(h = 0;h < 16;h++)smg[h] = 0x00;
	smg[1] = 0x01 * a;
	smg[3] = 0x01 * a;
	smg[5] = 0x01 * a;
	smg[7] = 0x01 * a;
	smg[9] = 0x01 * a;
	smg[11] = 0x01 * a;
	smg[13] = 0x01 * a;
	smg[15] = 0x01 * a;
*/
	STB_H;
	CLK_H;
	dio_set;
	Delay_us2(1);
	STB_L;
	TM1629B_write(0x40); //д���ݵ���ʾ�Ĵ��������õ�ַ�Զ���һ
	STB_H;
	Delay_us2(1);
	STB_L;
	TM1629B_write(0xc0);	 //��ʾ�Ĵ�����00H��Ԫ��ʼ
	for (h = 0; h < 16; h++) //led1629buf[h]
	{
		TM1629B_write(smg[h]); //����ʾ�Ĵ��������ݣ�led1629buf[h]
	}
	STB_H;
	Delay_us2(1);
	STB_L;
	TM1629B_write(0x88); //��ʾ���Ʋ���������������Ļ�����ȿ���ͨ���ı����λ����
	STB_H;
	TM1629B_DATAOUTPUT_init();
}

void get_1629key(void)
{

	uchar cnt;
	TM1629B_DATAOUTPUT_init();
	STB_H;
	CLK_H;
	dio_set;
	Delay_us2(1);
	STB_L;
	Delay_us2(1);
	TM1629B_write(0x42); //�Ͷ��������42H����
	Delay_us2(5);
	CLK_H;
	Delay_us2(5);
	STB_L;
	dio_set;
	Delay_us2(10);
	//  TM1629B_DATAINPUT_init();
	Delay_us2(1);
	for (cnt = 0; cnt < 4; cnt++)
	{
		key1629buf[cnt] = TM1629Bread(); //��4BIT�������ݣ�
		Delay_us2(10);
	}

	Delay_us2(1);
	TM1629B_DATAOUTPUT_init();
	STB_H;
	CLK_H;
	dio_set;
	// if(key1629buf[0])keynub[1] = key1629buf[0];
	// else if(key1629buf[1] == 8)keynub[1] = 512;
	// else keynub[1] = 0;
}

void disp_3wsmg(u8 *p, u8 line)
{
	u8 a, n, p1[3];
	for (a = 0; a < 3; a++)
		p1[a] = p[a];
	for (a = 0; a < 8; a++)
	{

		if (line == 1)
		{
			n = a * 2;
			if (p1[0] & 0x01)
				smg[n] |= 0x01;
			else
				smg[n] &= 0x0fe;
			if (p1[1] & 0x01)
				smg[n] |= 0x02;
			else
				smg[n] &= 0xfd;
			if (p1[2] & 0x01)
				smg[n] |= 0x04;
			else
				smg[n] &= 0xfb;
		}
		else if (line == 2)
		{
			n = a * 2;
			if (p1[0] & 0x01)
				smg[n] |= 0x08;
			else
				smg[n] &= 0x0f7;
			if (p1[1] & 0x01)
				smg[n] |= 0x10;
			else
				smg[n] &= 0xef;
			if (p1[2] & 0x01)
				smg[n] |= 0x20;
			else
				smg[n] &= 0xdf;
		}
		else if (line == 3)
		{
			n = a * 2;
			if (p1[0] & 0x01)
				smg[n] |= 0x40;
			else
				smg[n] &= 0xbf;
			if (p1[1] & 0x01)
				smg[n] |= 0x80;
			else
				smg[n] &= 0x7f;
			if (p1[2] & 0x01)
				smg[n + 1] |= 0x01;
			else
				smg[n + 1] &= 0xfe;
		}
		else if (line == 4)
		{
			n = a * 2 + 1;
			if (p1[0] & 0x01)
				smg[n] |= 0x02;
			else
				smg[n] &= 0x0fd;
			if (p1[1] & 0x01)
				smg[n] |= 0x04;
			else
				smg[n] &= 0x0fb;
			if (p1[2] & 0x01)
				smg[n] |= 0x08;
			else
				smg[n] &= 0x0f7;
		}

		p1[0] >>= 1;
		p1[1] >>= 1;
		p1[2] >>= 1;
	}
}

void disp_set3wsmg(u8 w, u8 *p, u8 line) //disp_set3wsmg(b,w1,sel_bit);
{
	u8 a, n, p1[3];
	for (a = 0; a < 3; a++)
		p1[a] = p[a];

	if (sel_line == line)
	{
		if (f_set && (F500ms == 0))
		{

			if ((w + 0) == sel_bit)
				p1[0] = 0;
			else if ((w + 1) == sel_bit)
				p1[1] = 0;
			else if ((w + 2) == sel_bit)
				p1[2] = 0;
		}
		else if (F500ms == 0)
		{
			p1[0] = 0;
			p1[1] = 0;
			p1[2] = 0;
		}
	}
	for (a = 0; a < 8; a++)
	{

		if (line == 1)
		{
			n = a * 2;
			if (p1[0] & 0x01)
				smg[n] |= 0x01;
			else
				smg[n] &= 0x0fe;
			if (p1[1] & 0x01)
				smg[n] |= 0x02;
			else
				smg[n] &= 0xfd;
			if (p1[2] & 0x01)
				smg[n] |= 0x04;
			else
				smg[n] &= 0xfb;
		}
		else if (line == 2)
		{
			n = a * 2;
			if (p1[0] & 0x01)
				smg[n] |= 0x08;
			else
				smg[n] &= 0x0f7;
			if (p1[1] & 0x01)
				smg[n] |= 0x10;
			else
				smg[n] &= 0xef;
			if (p1[2] & 0x01)
				smg[n] |= 0x20;
			else
				smg[n] &= 0xdf;
		}
		else if (line == 3)
		{
			n = a * 2;
			if (p1[0] & 0x01)
				smg[n] |= 0x40;
			else
				smg[n] &= 0xbf;
			if (p1[1] & 0x01)
				smg[n] |= 0x80;
			else
				smg[n] &= 0x7f;
			if (p1[2] & 0x01)
				smg[n + 1] |= 0x01;
			else
				smg[n + 1] &= 0xfe;
		}
		else if (line == 4)
		{
			n = a * 2 + 1;
			if (p1[0] & 0x01)
				smg[n] |= 0x02;
			else
				smg[n] &= 0x0fd;
			if (p1[1] & 0x01)
				smg[n] |= 0x04;
			else
				smg[n] &= 0x0fb;
			if (p1[2] & 0x01)
				smg[n] |= 0x08;
			else
				smg[n] &= 0x0f7;
		}

		p1[0] >>= 1;
		p1[1] >>= 1;
		p1[2] >>= 1;
	}
}

void CallTm1629()
{
	disp1629();
	get_1629key();
	if (Tm500msCnt < 4)
		Tm500msCnt++;
	else
	{
		Tm500msCnt = 0;
		F500ms = ~F500ms;
	} //
	if (Tm1sCnt < 9)
		Tm1sCnt++;
	else
	{
		Tm1sCnt = 0;
		F1S = ~F1S;
		dis_smgled();
	}
	if (sys_mainx == 4)
		dis_smgled();
	dis_led();
	if (keybackcnt)
		keybackcnt--;
	if (keybackcnt == 1)
	{
		sys_mainx = 1;
		dismod = 1;
	}
	//smg_BtoD(2,(ulong)KEYOFFON.key,4);
}
