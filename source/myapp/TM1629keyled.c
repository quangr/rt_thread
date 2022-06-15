#include "TM1629keyled.h"
#include "tm1650.h"
#define PROTECT_SETTING_START 24

uchar key_status;
uint16_t key_counter;
uchar copekey_counter;
uchar f_keycontinue;
u8 dismod;
u8 sel_line;
u8 sys_mainx;
uchar sel_bit;
u16 cope_keynub;
u16 keybackcnt;
uint16_t key_cnt[4] = {0, 0, 0, 0};
uchar led_allon; //LED��ȫ��
ulong ABC4_val[4] = {
	923000,
	342,
	20,
	5700,
}; //4·����
ulong Uval_abc[3] = {
	380,
	231,
	179,
}; //3·��ѹ
ulong P_abc[4] = {
	97000,
	430,
	1600,
	67,
};											 //����
_KEYOFFON KEYOFFON;							 //¼ÌµçÆ÷Êä³ö×´Ì¬
u8 phase_mod;								 //���������ģʽ
u8 key_clock[7] = {21, 3, 3, 4, 10, 39, 59}; //��ʾ�Ĵ���ʱ��
_ProtectBits ProtectBits;					 //±£»¤Ñ¹°å
_Protect Protect, CP_Protect;
u8 f_set;
ulong SetVal;

void switch_on(int data);
void switch_off(int data);
extern volatile uint32_t senddata[64];

extern const u8 smgtable[25];

extern u8 smg[16];
extern u16 smg_16h[4];
extern uchar cnt, key1629buf[4];
extern u8 Tm500msCnt; //
extern u8 Tm1sCnt;

u16 nowcnt;
uint16_t setting_changed = 0;

void KeyScan(void)
{
	uint16_t i;
	//Static u8 xdata  nowcnt = 0;
	i = get_keynub(); //uchar key_status;uchar cope_keynub;uint key_counter;uint copekey_counter;

	if (key_status == 0)
	{
		if (i)
		{
			key_status = 1;
			cope_keynub = i;
			key_counter = 1;
		}
		else
		{
			key_counter = 0;
			cope_keynub = 0;
		}
	}
	else if (key_status == 1)
	{

		if (cope_keynub == i) //��ԭ����Ƚ�
		{
			key_status = 2;
			key_counter++; // 2
			copekey_counter = 0;
		}
		else
		{
			cope_keynub = 0;
			key_status = 0;
			key_counter = 0;
			copekey_counter = 0;
		}
	}
	else if (key_status == 2)
	{

		if (cope_keynub == i)
		{
			copekey_counter++; // == 3
			if (copekey_counter++ > 5)
			{
				key_status = 3;
				copekey_counter = 0;
			}
		}

		else
		{
			cope_keynub = 0;
			key_status = 0;
			key_counter = 0;
		}
	}

	else if (key_status == 3)
	{
		key_ok();
	}

	else if (key_status == 5)
	{
		if (i == 0)
		{

			//---------------------���ܼ�����
			//if(cope_keynub != 16 || sys_mainx != 0){gnjbit = 0; dis_cnt[0] = 0;dis_cnt[1] = 0;}
			//else{dis_cnt[0]++;gnjbit = 1;}
			//-------------����
			//if( sys_mainx == 6)
			//{
			//if(cope_keynub != 64 )dis_cnt[2] = 0;
			//else {dis_cnt[2]++;if(dis_cnt[2] == 5)dismod = 5;canshu_out(); }

			// }
			//-------------------ȫ��
			key_cnt[0] = 0; //ȫ������
			led_allon = 0;	//ȫ����־����
			if (sys_mainx == 3)
			{
				sys_mainx = 0;
				led_allon = 0;
				key_cnt[0] = 0;
				allledoff();
			}

			//------------��������-------
			cope_keynub = 0;
			key_status = 0;
			key_counter = 0;
		}
	}
	else
	{
		cope_keynub = 0;
		key_status = 0;
		key_counter = 0;
	}
}

uint16_t get_keynub(void)
{
	//TODO OPTIMIZE
	uint16_t k;
	if ((key1629buf[0] & 0x08) == 0x08)
		k |= 0x01;
	else
		k &= 0xfe;
	if ((key1629buf[0] & 0x80) == 0x80)
		k |= 0x02;
	else
		k &= 0xfd;
	if ((key1629buf[1] & 0x08) == 0x08)
		k |= 0x04;
	else
		k &= 0xfb;
	if ((key1629buf[1] & 0x80) == 0x80)
		k |= 0x08;
	else
		k &= 0xf7;
	if ((key1629buf[2] & 0x08) == 0x08)
		k |= 0x10;
	else
		k &= 0xef;
	if ((key1629buf[2] & 0x80) == 0x80)
		k |= 0x20;
	else
		k &= 0xdf;
	if ((key1629buf[3] & 0x08) == 0x08)
		k |= 0x40;
	else
		k &= 0xbf;
	if ((key1629buf[3] & 0x80) == 0x80)
		k |= 0x80;
	else
		k &= 0x7f;

	return k;
}
extern float p_vv[3];
void protect_set(uint16_t line, uint16_t bound_I, uint16_t time_I, uint16_t bound_II, uint16_t time_II);

void key_ok(void)
{
	u8 k;
	uint16_t i, *p2;
	uint32_t *p;
	i = get_keynub();

	//---------------------------------------------------//
	if ((cope_keynub == i && cope_keynub > 0))
	{
		if (key_counter < 300)
		{
			key_counter++;
			if (f_keycontinue == 2)
			{
				if (key_counter == 299)
				{
					key_status = 2;
					key_counter = 0;
				}
			}
		}
		else if (f_keycontinue == 1)
		{
			f_keycontinue = 2;
			key_counter = 14;
		}
	}
	else
	{
		key_status = 5;
		f_keycontinue = 0;
		nowcnt = 0;
	} //nowcnt����ʱ��
	  //-----------------------������--------------------------//
	if ((cope_keynub == 1) && (key_counter == 4))
	{
		key_counter++;
		keybackcnt = KEYTNUB;
		if (sys_mainx == 1)
		{
			if (dismod < 2)
				dismod++;
			else
				dismod = 1;
			Tm500msCnt = 0;
			F500ms = 1;
			if (dismod == 1)
			{
				LED_Watch = F500ms;
				LED_Set = 0;
			}
			else if (dismod == 2)
			{
				LED_Set = F500ms;
				LED_Watch = 0;
			}
		}
		else if (sys_mainx == 2)
		{
			if (phase_mod == 1)
			{
				if (dismod < 14)
					dismod++;
				else
					dismod = 0;
			}
			else if (phase_mod == 3)
			{
				if (dismod < 9)
					dismod++;
				else
					dismod = 0;
			}
		}
		else if (sys_mainx == 4)
		{
			if (dismod == 100)
			{
				if (sel_line < 2)
					sel_line++;
				else
					sel_line = 1;
			}
			else
			{

				if (sel_line == 0)
				{
					if (phase_mod == 1)
					{
						if (dismod < 8)
							dismod++;
						else
							dismod = 1;
						sel_line = 0;
						sel_bit = 0;
					}
					else if (phase_mod == 3)
					{
						if (dismod < 3)
							dismod++;
						else
							dismod = 1;
						sel_line = 0;
						sel_bit = 0;
					}
				}
				else if (f_set == 0 && sel_line > 0)
				{
					if (phase_mod == 1)
					{
						if (sel_line > 3)
							sel_line--;
						else
							sel_line = 4;
						if (sel_line == 3)
							sel_bit = 0;
						else if (sel_line == 4)
							sel_bit = 2;
					}
					else if (phase_mod == 3)
					{
						if (sel_line > 3)
							sel_line--;
					}
				}
				else if (f_set == 1 && sel_line > 0)
				{
					if (phase_mod == 1)
					{
						if (dismod == 1)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P1L10ma;
								SetValAddDec(1);
								CP_P1L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P1Ls;
								SetValAddDec(1);
								if (SetVal > 600)
									SetVal = 600;
								CP_P1Ls = SetVal;
							}
						}
						else if (dismod == 2)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P2L10ma;
								SetValAddDec(1);
								CP_P2L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P2Ls;
								SetValAddDec(1);
								if (SetVal > 600)
									SetVal = CP_P2Ls;
								CP_P2Ls = SetVal;
							}
						}
						else if (dismod == 3)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P3L10ma;
								SetValAddDec(1);
								CP_P3L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P3Ls;
								SetValAddDec(1);
								if (SetVal > 600)
									SetVal = CP_P3Ls;
								CP_P3Ls = SetVal;
							}
						}
						else if (dismod == 4)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P4L10ma;
								SetValAddDec(1);
								CP_P4L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P4Ls;
								SetValAddDec(1);
								if (SetVal > 600)
									SetVal = CP_P4Ls;
								CP_P4Ls = SetVal;
							}
						}
						else if (dismod == 5)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P1S10ma;
								SetValAddDec(1);
								CP_P1S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P1Sms;
								SetValAddDec(1);
								CP_P1Sms = SetVal;
							}
						}
						else if (dismod == 6)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P2S10ma;
								SetValAddDec(1);
								CP_P2S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P2Sms;
								SetValAddDec(1);
								CP_P2Sms = SetVal;
							}
						}
						else if (dismod == 7)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P3S10ma;
								SetValAddDec(1);
								CP_P3S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P3Sms;
								SetValAddDec(1);
								CP_P3Sms = SetVal;
							}
						}
						else if (dismod == 8)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P4S10ma;
								SetValAddDec(1);
								CP_P4S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P4Sms;
								SetValAddDec(1);
								CP_P4Sms = SetVal;
							}
						}
					}
					if (phase_mod == 3)
					{
						if (dismod == 1)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P1S10ma;
								SetValAddDec(1);
								CP_P1S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P1Sms;
								SetValAddDec(1);
								CP_P1Sms = SetVal;
							}
						}
						else if (dismod == 2)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P1L10ma;
								SetValAddDec(1);
								CP_P1L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P1Ls;
								SetValAddDec(1);
								if (SetVal > 600)
									SetVal = 600;
								CP_P1Ls = SetVal;
							}
						}
						else if (dismod == 3)
						{
							if (sel_line == 3)
							{
								SetVal = CP_PNOT10ma;
								SetValAddDec(1);
								CP_PNOT10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_PNOTms;
								SetValAddDec(1);
								CP_PNOTms = SetVal;
							}
						}
					}
				}
			}
		}
		dis_smgled();
	}
	//-----------------------���һ��--------------------------//
	else if (cope_keynub == 2 && key_counter == 4) //
	{
		key_counter++;
		keybackcnt = KEYTNUB;
		if (sys_mainx == 1)
		{
			if (dismod > 1)
				dismod--;
			else
				dismod = 2;
			Tm500msCnt = 0;
			F500ms = 1;
		}
		else if (sys_mainx == 2)
		{
			if (dismod > 1)
				dismod--;
			else if (phase_mod == 1)
				dismod = 14;
			else if (phase_mod == 3)
				dismod = 9;
		}
		else if (sys_mainx == 4)
		{

			if (dismod == 100)
			{
				if (sel_line > 1)
					sel_line--;
				else
					sel_line = 2;
			}
			else
			{
				if (sel_line == 0)
				{
					if (dismod > 1)
						dismod--;
					else if (phase_mod == 1)
						dismod = 8;
					else if (phase_mod == 3)
						dismod = 3;
					sel_line = 0;
					sel_bit = 0;
				}
				else if (sel_line > 0 && f_set == 0)
				{
					if (sel_line < 4)
						sel_line++;
					else
						sel_line = 3;
					if (sel_line == 3)
						sel_bit = 0;
					else if (sel_line == 4)
					{
						if (dismod < 5)
							sel_bit = 2;
						else
							sel_bit = 0;
					}
				}
				else if (f_set == 1 && sel_line > 0)
				{
					if (phase_mod == 1)
					{
						if (dismod == 1)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P1L10ma;
								SetValAddDec(0);
								CP_P1L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P1Ls;
								SetValAddDec(0);
								if (SetVal > 600)
									SetVal = CP_P1Ls;
								CP_P1Ls = SetVal;
							}
						}
						else if (dismod == 2)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P2L10ma;
								SetValAddDec(0);
								CP_P2L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P2Ls;
								SetValAddDec(0);
								if (SetVal > 600)
									SetVal = CP_P2Ls;
								CP_P2Ls = SetVal;
							}
						}
						else if (dismod == 3)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P3L10ma;
								SetValAddDec(0);
								CP_P3L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P3Ls;
								SetValAddDec(0);
								if (SetVal > 600)
									SetVal = CP_P3Ls;
								CP_P3Ls = SetVal;
							}
						}
						else if (dismod == 4)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P4L10ma;
								SetValAddDec(0);
								CP_P4L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P4Ls;
								SetValAddDec(0);
								if (SetVal > 600)
									SetVal = CP_P4Ls;
								CP_P4Ls = SetVal;
							}
						}
						else if (dismod == 5)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P1S10ma;
								SetValAddDec(0);
								CP_P1S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P1Sms;
								SetValAddDec(0);
								CP_P1Sms = SetVal;
							}
						}
						else if (dismod == 6)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P2S10ma;
								SetValAddDec(0);
								CP_P2S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P2Sms;
								SetValAddDec(0);
								CP_P2Sms = SetVal;
							}
						}
						else if (dismod == 7)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P3S10ma;
								SetValAddDec(0);
								CP_P3S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P3Sms;
								SetValAddDec(0);
								CP_P3Sms = SetVal;
							}
						}
						else if (dismod == 8)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P4S10ma;
								SetValAddDec(0);
								CP_P4S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P4Sms;
								SetValAddDec(0);
								CP_P4Sms = SetVal;
							}
						}
					}
					else
					{
						if (dismod == 1)
						{
							if (sel_line == 3)
							{
								SetVal = CP_P1S10ma;
								SetValAddDec(0);
								CP_P1S10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P1Sms;
								SetValAddDec(0);
								CP_P1Sms = SetVal;
							}
						}
						else if (dismod == 2)
						{

							if (sel_line == 3)
							{
								SetVal = CP_P1L10ma;
								SetValAddDec(0);
								CP_P1L10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_P1Ls;
								SetValAddDec(0);
								if (SetVal > 600)
									SetVal = CP_P1Ls;
								CP_P1Ls = SetVal;
							}
						}
						else if (dismod == 3)
						{
							if (sel_line == 3)
							{
								SetVal = CP_PNOT10ma;
								SetValAddDec(0);
								CP_PNOT10ma = SetVal;
							}
							else if (sel_line == 4)
							{
								SetVal = CP_PNOTms;
								SetValAddDec(0);
								CP_PNOTms = SetVal;
							}
						}
					}
				}
			}
		}
		dis_smgled();
	}
	//-----------------------ȷ��--------------------------//
	else if (cope_keynub == 4 && key_counter == 4) //
	{
		key_counter++;
		keybackcnt = KEYTNUB;
		if (sys_mainx == 1)
		{
			if (dismod == 1)
				sys_mainx = 2;
			else if (dismod == 2)
			{
				if (phase_mod == 1)
				{
					f_set = 0;
					sys_mainx = 4;
					dismod = 1;
					sel_line = 0;
					sel_bit = 0;
					p = &senddata[PROTECT_SETTING_START];
					p2 = &CP_Protect._P1S10ma;
					for (k = 0; k < 16; k++)
					{
						if (k % 4 == 1)
						{
							*p2 = (*p) * 2;
						}
						else
						{
							*p2 = *p;
						}
						p++;
						p2++;
					} //���Ƶ�����
				}
				if (phase_mod == 3)
				{
					f_set = 0;
					sys_mainx = 4;
					dismod = 1;
					sel_line = 0;
					sel_bit = 0;
					CP_Protect._P1S10ma = senddata[PROTECT_SETTING_START] ;
					CP_Protect._P1Sms = senddata[PROTECT_SETTING_START + 1] * 2;
					CP_Protect._P1L10ma = senddata[PROTECT_SETTING_START + 2];
					CP_Protect._P1Ls = senddata[PROTECT_SETTING_START + 3];
					CP_Protect._PNOT10ma = senddata[PROTECT_SETTING_START + 4];
					CP_Protect._PNOTms = senddata[PROTECT_SETTING_START + 5] * 2;
				}
			}
			dismod = 1;
		}
		else if (sys_mainx == 4)
		{
			if (dismod == 100)
			{
				if (sel_line == 1)
				{
					if (phase_mod == 1)
					{

						p = &senddata[PROTECT_SETTING_START];
						p2 = &CP_Protect._P1S10ma;
						for (size_t ttt = 0; ttt < 4; ttt++)
						{
						if ((*(p + ttt * 4) != *(p2 + ttt * 4)) || (*(p + ttt * 4 + 1) != (*(p2 + ttt * 4 + 1) / 20)) || (*(p + ttt * 4 + 2) != *(p2 + ttt * 4 + 2)) || (*(p + ttt * 4 + 3) != *(p2 + ttt * 4 + 3)))
							{
								protect_set(ttt, (*(p2 + ttt * 4)), *(p2 + ttt * 4 + 1) / 2, *(p2 + ttt * 4 + 2), *(p2 + ttt * 4 + 3));
								setting_changed = 1;
							}
						}
					}
					else
					{
						if (senddata[PROTECT_SETTING_START]/10 != CP_Protect._P1S10ma || senddata[PROTECT_SETTING_START + 1] != CP_Protect._P1Sms / 20 || senddata[PROTECT_SETTING_START + 2] != CP_Protect._P1L10ma || senddata[PROTECT_SETTING_START + 3] != CP_Protect._P1Ls)
						{
							protect_set(0, CP_Protect._P1S10ma, CP_Protect._P1Sms / 2, CP_Protect._P1L10ma, CP_Protect._P1Ls);
							setting_changed = 1;
						}
						if (senddata[PROTECT_SETTING_START + 4] != CP_Protect._PNOT10ma || senddata[PROTECT_SETTING_START + 5] != CP_Protect._PNOTms)
						{
							protect_set(1, CP_Protect._PNOT10ma, CP_Protect._PNOTms, 0, 0);
							setting_changed = 1;
						}
					}
					SaveSet();
					sys_mainx = 1;
					dismod = 1;
					sel_line = 0;
					sel_bit = 0;
				}
				else
				{
					sys_mainx = 1;
					dismod = 1;
					sel_line = 0;
					sel_bit = 0;
				}
			}
			else
			{

				if (sel_line == 0)
				{
					sel_line = 3;
					f_set = 0;
				}
				else if (sel_line > 0)
				{
					if (f_set == 0)
						f_set = 1;
					else
					{
						if (sel_bit < 4)
							sel_bit++;
						else
						{
							if (sel_line == 4 && dismod < 5)
								sel_bit = 2;
							else
								sel_bit = 0;
						}
					}
				}
			}
		}

		dis_smgled();
	}

	//-----------------------ȡ��--------------------------//

	else if (cope_keynub == 8 && key_counter == 4)
	{
		if (sys_mainx == 2)
		{
			sys_mainx = 1;
			dismod = 1;
			sel_line = 0;
		}
		if (sys_mainx == 4)
		{
			if (sel_line > 0)
			{
				if (f_set == 1)
					f_set = 0;
				else if (f_set == 0)
				{
					sel_line = 0;
				}
			}
			else if (sel_line == 0)
			{
				if (dismod < 100)
				{
					dismod = 100;
					sel_line = 1;
				}
				else if (dismod == 100)
				{
					sys_mainx = 1;
					dismod = 1;
					sel_line = 0;
					sel_bit = 0;
				}
			}
		}

		dis_smgled();
	}

	else if (cope_keynub == 0x10 && key_counter == 4) //����1
	{
		rt_kprintf("swtich_toggle senddata[0]=%d \n", senddata[0]);
		rt_enter_critical();
		if (senddata[0] & 0x1)
		{
			switch_off(0x1);
		}
		else
		{
			switch_on(0x1);
		}
		setting_changed = 1;
		rt_exit_critical();
	}

	else if (cope_keynub == 0x20 && key_counter == 4) //����2
	{
		rt_enter_critical();
		if (senddata[0] & 0x2)
		{
			switch_off(0x2);
		}
		else
		{
			switch_on(0x2);
		}
		setting_changed = 1;

		rt_exit_critical();
	}

	else if (cope_keynub == 0x40 && key_counter == 4) //����3
	{
		rt_enter_critical();
		if (senddata[0] & 0x4)
		{
			switch_off(0x4);
		}
		else
		{
			switch_on(0x4);
		}
		setting_changed = 1;

		rt_exit_critical();
	}

	else if (cope_keynub == 0x80 && key_counter == 4) //����3
	{
		rt_enter_critical();
		if (senddata[0] & 0x8)
		{
			switch_off(0x8);
		}
		else
		{
			switch_on(0x8);
		}
		setting_changed = 1;

		rt_exit_critical();
	}
	dis_smgled();
}

void allledoff(void)
{
	u8 a = 0;
	for (a = 0; a < 16; a++)
		smg[a] = 0;
}

void allledon(void)
{
	u8 a = 0;
	for (a = 0; a < 16; a++)
		smg[a] = 0xff;
}

void dis_smgled(void)
{
	ulong l;
	u8 a;
	if (sys_mainx == 1)
	{
		if (phase_mod == 1) //����ģʽ
		{
			//-----------------------��һ·��ʾ-----------------------------
			if ((senddata[1] < 10) && ((senddata[0] & 0x1) == 0))
				smg_BtoD(10, 0, 1); //��ʾ�����ѷ�բ
			else
			{
				l = senddata[1] / 10;
				smg_BtoD(2, l, 1);
			}
			//-----------------------�ڶ�·��ʾ-----------------------------
			if ((senddata[2] < 10) && ((senddata[0] & 0x2) == 0))
				smg_BtoD(10, 0, 2); //��ʾ�����ѷ�բ
			else
			{
				l = senddata[2] / 10;
				smg_BtoD(2, l, 2);
			}
			//-----------------------����·��ʾ-----------------------------
			if ((senddata[3] < 10) && ((senddata[0] & 0x4) == 0))
				smg_BtoD(10, 0, 3); //��ʾ�����ѷ�բ
			else
			{
				l = senddata[3] / 10;
				smg_BtoD(2, l, 3);
			}
			//-----------------------����·��ʾ-----------------------------
			if ((senddata[4] < 10) && ((senddata[0] & 0x8) == 0))
				smg_BtoD(10, 0, 4); //��ʾ�����ѷ�բ
			else
			{
				l = senddata[4] / 10;
				smg_BtoD(2, l, 4);
			}
		}
		else if (phase_mod == 3) //����ģʽ
		{
			//-----------------------��һ·��ʾ-----------------------------
			if ((senddata[1] < 10) && (senddata[2] < 10) && (senddata[3] < 10) && ((senddata[0] & 0x1) == 0))
			{
				smg_BtoD(10, 0, 1); //��ʾ�����ѷ�բ
				smg_BtoD(12, 0, 2); //��ʾ�����ѷ�բ
				smg_BtoD(12, 0, 3); //��ʾ�����ѷ�բ
			}
			else
			{
				l = senddata[1] / 10;
				smg_BtoD(2, l, 1);
				l = senddata[2] / 10;
				smg_BtoD(2, l, 2);
				l = senddata[3] / 10;
				smg_BtoD(2, l, 3);
			}
			if ((senddata[4] < 10) && ((senddata[0] & 0x8) == 0))
				smg_BtoD(10, 0, 4); //��ʾ�����ѷ�բ
			else
			{
				l = senddata[4] / 10;
				smg_BtoD(2, l, 4);
			}
		}
	}

	if (sys_mainx == 2)
	{
		if (phase_mod == 1) //����ģʽ
		{
			if (dismod == 1)
			{
				if ((senddata[0] & 0x1) == 0)
					smg_BtoD(10, 0, 1); //��ʾ�����ѷ�բ
				else
					smg_BtoD(11, 0, 1); //��ʾ�����ѷ�բ

				if ((senddata[0] & 0x2) == 0)
					smg_BtoD(10, 0, 2); //��ʾ�����ѷ�բ
				else
					smg_BtoD(11, 0, 2); //��ʾ�����ѷ�բ

				if ((senddata[0] & 0x4) == 0)
					smg_BtoD(10, 0, 3); //��ʾ�����ѷ�բ
				else
					smg_BtoD(11, 0, 3); //��ʾ�����ѷ�բ

				if ((senddata[0] & 0x8) == 0)
					smg_BtoD(10, 0, 4); //��ʾ�����ѷ�բ
				else
					smg_BtoD(11, 0, 4); //��ʾ�����ѷ�բ
			}

			else if (dismod == 2) //����
			{
				l = senddata[1] / 10;
				smg_BtoD(2, l, 1);
				l = senddata[2] / 10;
				smg_BtoD(2, l, 2);
				l = senddata[3] / 10;
				smg_BtoD(2, l, 3);
				l = senddata[4] / 10;
				smg_BtoD(2, l, 4);
			}
			else if (dismod == 3) //��ѹ
			{
				l = senddata[5];
				smg_BtoD(0, l, 1);
				smg_BtoD(12, l, 2);
				smg_BtoD(12, l, 3);
				smg_BtoD(12, l, 4);
			}
			else if (dismod == 4) //P_abc[4]����
			{
				l = ((int32_t)((p_vv[0] >= 0) ? (p_vv[0])/10 : -(p_vv[0])/10)) ;
				smg_BtoD(2, l, 1);
				l = ((int32_t)((p_vv[1] >= 0) ? (p_vv[1])/10 : -(p_vv[1])/10)) ;
				smg_BtoD(2, l, 2);
				l = ((int32_t)((p_vv[2] >= 0) ? (p_vv[2])/10 : -(p_vv[2])/10));
				smg_BtoD(2, l, 3);
				smg_BtoD(12, l, 4);
				// l = P_abc[3] / 10;
				// smg_BtoD(2, l, 4);
			}
			else if (dismod == 5) //P_abc[4]
			{
				dispclock(0); //����
			}
			else if (dismod == 6) //P_abc[4]
			{
				dispclock(1); //ʱ��
			}
			else if (dismod > 6 && dismod < 11) //P_abc[4]
			{
				smg_BtoD(13, 0, 1);
				a = dismod - 6 + 20;
				smg_BtoD(a, 0, 2);
				if (dismod == 7)
				{
					l = senddata[PROTECT_SETTING_START + 0 * 4 + 2];
					if (l == 0)
					{

						smg_BtoD(10, l, 3);
						smg_BtoD(12, l, 4);
					}
					else
					{
						smg_BtoD(2, l, 3);
						l = senddata[PROTECT_SETTING_START + 0 * 4 + 3];
						smg_BtoD(0, l, 4);
					}
				}
				else if (dismod == 8)
				{
					l = senddata[PROTECT_SETTING_START + 1 * 4 + 2];
					if (l == 0)
					{

						smg_BtoD(10, l, 3);
						smg_BtoD(12, l, 4);
					}
					else
					{
						smg_BtoD(2, l, 3);
						l = senddata[PROTECT_SETTING_START + 1 * 4 + 3];
						smg_BtoD(0, l, 4);
					}
				}
				else if (dismod == 9)
				{
					l = senddata[PROTECT_SETTING_START + 2 * 4 + 2];
					if (l == 0)
					{

						smg_BtoD(10, l, 3);
						smg_BtoD(12, l, 4);
					}
					else
					{
						smg_BtoD(2, l, 3);
						l = senddata[PROTECT_SETTING_START + 2 * 4 + 3];
						smg_BtoD(0, l, 4);
					}
				}
				else if (dismod == 10)
				{
					l = senddata[PROTECT_SETTING_START + 3 * 4 + 2];
					if (l == 0)
					{

						smg_BtoD(10, l, 3);
						smg_BtoD(12, l, 4);
					}
					else
					{
						smg_BtoD(2, l, 3);
						l = senddata[PROTECT_SETTING_START + 3 * 4 + 3];
						smg_BtoD(0, l, 4);
					}
				}
			}
			else if (dismod > 10 && dismod < 15) //P_abc[4]
			{
				smg_BtoD(13, 0, 1);
				a = dismod - 10 + 30;
				smg_BtoD(a, 0, 2);
				if (dismod == 11)
				{
					l = senddata[PROTECT_SETTING_START + 0 * 4] ;
					if (l == 0)
					{

						smg_BtoD(10, l, 3);
						smg_BtoD(12, l, 4);
					}
					else
					{
						smg_BtoD(2, l, 3);
						l = senddata[PROTECT_SETTING_START + 0 * 4 + 1] * 2;
						smg_BtoD(2, l, 4);
					}
				}
				else if (dismod == 12)
				{
					l = senddata[PROTECT_SETTING_START + 1 * 4] ;
					if (l == 0)
					{

						smg_BtoD(10, l, 3);
						smg_BtoD(12, l, 4);
					}
					else
					{
						smg_BtoD(2, l, 3);
						l = senddata[PROTECT_SETTING_START + 1 * 4 + 1] * 2;
						smg_BtoD(2, l, 4);
					}
				}
				else if (dismod == 13)
				{
					l = senddata[PROTECT_SETTING_START + 2 * 4] ;
					if (l == 0)
					{

						smg_BtoD(10, l, 3);
						smg_BtoD(12, l, 4);
					}
					else
					{
						smg_BtoD(2, l, 3);
						l = senddata[PROTECT_SETTING_START + 2 * 4 + 1] * 2;
						smg_BtoD(2, l, 4);
					}
				}
				else if (dismod == 14)
				{
					l = senddata[PROTECT_SETTING_START + 3 * 4] ;
					if (l == 0)
					{

						smg_BtoD(10, l, 3);
						smg_BtoD(12, l, 4);
					}
					else
					{
						smg_BtoD(2, l, 3);
						l = senddata[PROTECT_SETTING_START + 3 * 4 + 1] * 2;
						smg_BtoD(2, l, 4);
					}
				}
			}
		}
		else if (phase_mod == 3) //����ģʽ
		{
			if (dismod == 1)
			{
				if ((senddata[0] & 0x1) == 0)
					smg_BtoD(10, 0, 1); //��ʾ�����ѷ�բ
				else
					smg_BtoD(11, 0, 1); //��ʾ�����ѷ�բ

				if ((senddata[0] & 0x2) == 0)
					smg_BtoD(10, 0, 2); //��ʾ�����ѷ�բ
				else
					smg_BtoD(11, 0, 2); //��ʾ�����ѷ�բ

				if ((senddata[0] & 0x4) == 0)
					smg_BtoD(10, 0, 3); //��ʾ�����ѷ�բ
				else
					smg_BtoD(11, 0, 3); //��ʾ�����ѷ�բ

				if ((senddata[0] & 0x8) == 0)
					smg_BtoD(10, 0, 4); //��ʾ�����ѷ�բ
				else
					smg_BtoD(11, 0, 4); //��ʾ�����ѷ�բ
			}

			else if (dismod == 2)
			{
				l = senddata[1] / 10;
				smg_BtoD(2, l, 1); //A�����
				l = senddata[2] / 10;
				smg_BtoD(2, l, 2); //B�����
				l = senddata[3] / 10;
				smg_BtoD(2, l, 3); //C�����
				l = senddata[4] / 10;
				smg_BtoD(2, l, 4); //��ƽ�⣨���򣩵���
			}
			else if (dismod == 3)
			{
				l = senddata[5];
				smg_BtoD(0, l, 1); //A���ѹ
				l = senddata[6];
				smg_BtoD(0, l, 2); //B���ѹ
				l = senddata[7];
				smg_BtoD(0, l, 3); //C���ѹ
				smg_BtoD(12, l, 4);
			}
			else if (dismod == 4) //P_abc[4]
			{
				l = ((int32_t)((p_vv[0] >= 0) ? (p_vv[0])/10 : -(p_vv[0])/10)) ;
				smg_BtoD(2, l, 1);
				l = ((int32_t)((p_vv[1] >= 0) ? (p_vv[1])/10 : -(p_vv[1])/10)) ;
				smg_BtoD(2, l, 2);
				l = ((int32_t)((p_vv[2] >= 0) ? (p_vv[2])/10 : -(p_vv[2])/10));
				smg_BtoD(2, l, 3);
				l = (int32_t)((p_vv[2] + p_vv[1] + p_vv[0]) >= 0 ? (int32_t)((p_vv[2] + p_vv[1] + p_vv[0])/10) : -(int32_t)(((p_vv[2] + p_vv[1] + p_vv[0])/10)));
				smg_BtoD(2, l, 4); //�ܹ���
			}
			else if (dismod == 5) //P_abc[4]
			{
				dispclock(0);
			}
			else if (dismod == 6) //P_abc[4]
			{
				dispclock(1);
			}
			else if (dismod == 7) //P_abc[4]
			{
				smg_BtoD(13, 0, 1);
				smg_BtoD(20, 0, 2);
				l = senddata[PROTECT_SETTING_START + 2];
				if (l == 0)
				{

					smg_BtoD(10, l, 3);
					smg_BtoD(12, l, 4);
				}
				else
				{
					smg_BtoD(2, l, 3);
					l = senddata[PROTECT_SETTING_START + 3];
					smg_BtoD(0, l, 4);
				}
			}
			else if (dismod == 8) //P_abc[4]
			{
				smg_BtoD(13, 0, 1);
				smg_BtoD(30, 0, 2);
				l = senddata[PROTECT_SETTING_START + 0 * 4] ;
				if (l == 0)
				{

					smg_BtoD(10, l, 3);
					smg_BtoD(12, l, 4);
				}
				else
				{
					smg_BtoD(2, l, 3);
					l = senddata[PROTECT_SETTING_START + 0 * 4 + 1] * 2;
					smg_BtoD(2, l, 4);
				}
			}
			else if (dismod == 9) //P_abc[4]
			{
				smg_BtoD(13, 0, 1);
				smg_BtoD(35, 0, 2);
				l = senddata[PROTECT_SETTING_START + 1 * 4];
				if (l == 0)
				{

					smg_BtoD(10, l, 3);
					smg_BtoD(12, l, 4);
				}
				else
				{
					smg_BtoD(2, l, 3);
					l = senddata[PROTECT_SETTING_START + 1 * 4 + 1] * 2;
					smg_BtoD(2, l, 4);
				}
			}
		}
	}

	if (sys_mainx == 4)
	{
		if (phase_mod == 1) //����ģʽ
		{
			if (dismod > 0 && dismod < 5) //P_abc[4]
			{
				smg_BtoD(13, 0, 1);
				a = dismod - 0 + 20;
				smg_BtoD(a, 0, 2);
				if (dismod == 1)
				{
					l = CP_P1L10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P1Ls;
					Disp_SetSmg(0, l, 4);
				} //Disp_SetSmg(u8 d,ulong val,u8  line)
				else if (dismod == 2)
				{
					l = CP_P2L10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P2Ls;
					Disp_SetSmg(0, l, 4);
				}
				else if (dismod == 3)
				{
					l = CP_P3L10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P3Ls;
					Disp_SetSmg(0, l, 4);
				}
				else if (dismod == 4)
				{
					l = CP_P4L10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P4Ls;
					Disp_SetSmg(0, l, 4);
				}
			}
			else if (dismod > 4 && dismod < 9) //P_abc[4]
			{
				smg_BtoD(13, 0, 1);
				a = dismod - 4 + 30;
				smg_BtoD(a, 0, 2);
				if (dismod == 5)
				{
					l = CP_P1S10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P1Sms;
					Disp_SetSmg(2, l, 4);
				}
				else if (dismod == 6)
				{
					l = CP_P2S10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P2Sms;
					Disp_SetSmg(2, l, 4);
				}
				else if (dismod == 7)
				{
					l = CP_P3S10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P3Sms;
					Disp_SetSmg(2, l, 4);
				}
				else if (dismod == 8)
				{
					l = CP_P4S10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P4Sms;
					Disp_SetSmg(2, l, 4);
				}
			}
			else if (dismod == 100) //P_abc[4]
			{
				if (sel_line == 1)
				{
					if (F500ms == 0)
						smg_BtoD(12, 0, 1);
					else
						smg_BtoD(14, 0, 1);
				}
				else
				{
					smg_BtoD(14, 0, 1);
				}

				if (sel_line == 2)
				{
					if (F500ms == 0)
						smg_BtoD(12, 0, 2);
					else
						smg_BtoD(15, 0, 2);
				}
				else
				{
					smg_BtoD(15, 0, 2);
				}
				smg_BtoD(12, sel_line, 3);
				smg_BtoD(12, l, 4);
			}
		}
		else if (phase_mod == 3) //����ģʽ
		{
			if (dismod > 0 && dismod < 5) //P_abc[4]
			{
				if (dismod == 1)
				{
					smg_BtoD(13, 0, 1);
					a = 5 - 4 + 30;
					smg_BtoD(a, 0, 2);
					l = CP_P1S10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P1Sms;
					Disp_SetSmg(2, l, 4);
				}
				if (dismod == 2)
				{
					smg_BtoD(13, 0, 1);
					a = 1 - 0 + 20;
					smg_BtoD(a, 0, 2);
					l = CP_P1L10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_P1Ls;
					Disp_SetSmg(0, l, 4);
				}
				if (dismod == 3)
				{
					smg_BtoD(13, 0, 1);
					smg_BtoD(35, 0, 2);
					l = CP_PNOT10ma;
					Disp_SetSmg(2, l, 3);
					l = CP_PNOTms;
					Disp_SetSmg(0, l, 4);
				}
			}
			else if (dismod == 100) //P_abc[4]
			{
				if (sel_line == 1)
				{
					if (F500ms == 0)
						smg_BtoD(12, 0, 1);
					else
						smg_BtoD(14, 0, 1);
				}
				else
				{
					smg_BtoD(14, 0, 1);
				}

				if (sel_line == 2)
				{
					if (F500ms == 0)
						smg_BtoD(12, 0, 2);
					else
						smg_BtoD(15, 0, 2);
				}
				else
				{
					smg_BtoD(15, 0, 2);
				}
				smg_BtoD(12, sel_line, 3);
				smg_BtoD(12, l, 4);
			}
		}
	}
}

void dispclock(u8 ms)
{
	u8 w[3];
	if (ms == 0)
	{
		if (Kyear > 99)
			Kyear = 99;
		w[1] = smgtable[Kyear / 10];
		w[2] = smgtable[Kyear % 10];
		w[0] = smgtable[24];
		disp_3wsmg(w, 1);
		if (Kmonth > 12 || Kmonth < 1)
			Kmonth = 1;
		w[1] = smgtable[Kmonth / 10];
		w[2] = smgtable[Kmonth % 10];
		w[0] = smgtable[18];
		disp_3wsmg(w, 2);

		if (Kdate > 31 || Kdate < 1)
			Kdate = 1;
		w[1] = smgtable[Kdate / 10];
		w[2] = smgtable[Kdate % 10];
		w[0] = smgtable[18];
		disp_3wsmg(w, 3);

		if (Kweek > 6)
			Kweek = 0;
		w[2] = smgtable[Kweek];
		w[1] = smgtable[24];
		w[0] = smgtable[24];
		disp_3wsmg(w, 4);
	}
	if (ms == 1)
	{
		if (Khour > 23)
			Khour = 0;
		w[1] = smgtable[Khour / 10];
		w[2] = smgtable[Khour % 10];
		w[0] = smgtable[24];
		disp_3wsmg(w, 1);
		if (Kmin > 59)
			Kmin = 0;
		w[1] = smgtable[Kmin / 10];
		w[2] = smgtable[Kmin % 10];
		w[0] = smgtable[18];
		disp_3wsmg(w, 2);

		if (Ksec > 59)
			Ksec = 0;
		w[1] = smgtable[Ksec / 10];
		w[2] = smgtable[Ksec % 10];
		w[0] = smgtable[18];
		disp_3wsmg(w, 3);

		w[1] = smgtable[24];
		w[2] = smgtable[24];
		w[0] = smgtable[24];
		disp_3wsmg(w, 4);
	}
}

void dis_led(void)
{
	if (sys_mainx == 1)
	{
		if (dismod == 1)
		{
			LED_Watch = F500ms;
			LED_Set = 0;
		}
		else if (dismod == 2)
		{
			LED_Set = F500ms;
			LED_Watch = 0;
		}
		LEDLIGHT.LED &= 0xf00f;
		if ((senddata[0] & 0x1))
		{
			LEDLIGHT.LED |= 0x0010;
		}
		if ((senddata[0] & 0x2))
		{
			LEDLIGHT.LED |= 0x0040;
		}
		if ((senddata[0] & 0x4))
		{
			LEDLIGHT.LED |= 0x0100;
		}
		if ((senddata[0] & 0x8))
		{
			LEDLIGHT.LED |= 0x0400;
		}
	}
	if (sys_mainx == 2)
	{
		LEDLIGHT.LED &= 0xf007;
		LED_Watch = 1;
		LED_Set = 0;
		if (phase_mod == 1)
		{
			if (dismod == 2)
			{
				LEDLIGHT.LED |= 0x0550;
			}
			else if (dismod == 3)
			{
				LEDLIGHT.LED |= 0x0aa0;
			}
			else if (dismod == 4)
			{
				LEDLIGHT.LED |= 0x0ff0;
			}
			else if (dismod == 5 || dismod == 6)
			{
				LEDLIGHT.LED |= 0x0008;
			}
			else if (dismod > 6 && dismod < 15)
			{
				LEDLIGHT.LED |= 0x0900;
			}
		}
	}
}

void SetValAddDec(u8 nub)
{
	u8 a, n[5];
	ulong c;
	if (SetVal > 60000)
		SetVal = 60000;
	n[0] = SetVal / 10000;
	c = SetVal % 10000;
	n[1] = c / 1000;
	c %= 1000;
	n[2] = c / 100;
	c %= 100;
	n[3] = c / 10;
	n[4] = c % 10;
	if (nub == 1)
	{
		n[sel_bit]++;
		if (sel_bit > 0)
		{
			if (n[sel_bit] > 9)
				n[sel_bit] = 0;
		}
		else
		{
			if (n[sel_bit] > 5)
				n[sel_bit] = 0;
		}
	}
	else if (nub == 0)
	{
		if (n[sel_bit] > 0)
			n[sel_bit]--;
		else
		{
			if (sel_bit)
				n[sel_bit] = 9;
			else
				n[sel_bit] = 5;
		}
	}
	SetVal = 10000 * n[0] + 1000 * n[1] + 100 * n[2] + 10 * n[3] + n[4];
}

void SaveSet(void)
{
}
