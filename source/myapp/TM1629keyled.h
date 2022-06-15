#ifndef __TM1629KEYLED_H
#define __TM1629KEYLED_H

#include <rtthread.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef uint32_t ulong;
typedef uint8_t uchar;

typedef union
{
	u8 Protect;
	struct
	{
		u8 _P1L : 1; //����ʱһ·����
		u8 _P2L : 1; //����ʱ��·����
		u8 _P3L : 1; //����ʱ��·����
		u8 _P4L : 1; //����ʱ��·����
		u8 _P1S : 1; //����ʱһ·����
		u8 _P2S : 1; //����ʱ��·����
		u8 _P3S : 1; //����ʱ��·����
		u8 _P4S : 1; //����ʱ��·����
	} bits;
} _ProtectBits;

extern _ProtectBits ProtectBits;

typedef struct
{
	u16 _P1S10ma;
	u16 _P1Sms;
	u16 _P1L10ma;
	u16 _P1Ls;
	u16 _P2S10ma;
	u16 _P2Sms;
	u16 _P2L10ma;
	u16 _P2Ls;
	u16 _P3S10ma;
	u16 _P3Sms;
	u16 _P3L10ma;
	u16 _P3Ls;
	u16 _P4S10ma;
	u16 _P4Sms;
	u16 _P4L10ma;
	u16 _P4Ls;
	u16 _PNOT10ma;
	u16 _PNOTms;

} _Protect;

extern _Protect Protect, CP_Protect;

#define P1L10ma Protect._P1L10ma
#define P2L10ma Protect._P2L10ma
#define P3L10ma Protect._P3L10ma
#define P4L10ma Protect._P4L10ma
#define PNOT10ma Protect._PNOT10ma
#define P1S10ma Protect._P1S10ma
#define P2S10ma Protect._P2S10ma
#define P3S10ma Protect._P3S10ma
#define P4S10ma Protect._P4S10ma
#define P1Ls Protect._P1Ls
#define P2Ls Protect._P2Ls
#define P3Ls Protect._P3Ls
#define P4Ls Protect._P4Ls
#define P1Sms Protect._P1Sms
#define P2Sms Protect._P2Sms
#define P3Sms Protect._P3Sms
#define P4Sms Protect._P4Sms
#define PNOTms Protect._PNOTms

#define CP_P1L10ma CP_Protect._P1L10ma
#define CP_P2L10ma CP_Protect._P2L10ma
#define CP_P3L10ma CP_Protect._P3L10ma
#define CP_P4L10ma CP_Protect._P4L10ma
#define CP_PNOT10ma CP_Protect._PNOT10ma
#define CP_P1S10ma CP_Protect._P1S10ma
#define CP_P2S10ma CP_Protect._P2S10ma
#define CP_P3S10ma CP_Protect._P3S10ma
#define CP_P4S10ma CP_Protect._P4S10ma
#define CP_P1Ls CP_Protect._P1Ls
#define CP_P2Ls CP_Protect._P2Ls
#define CP_P3Ls CP_Protect._P3Ls
#define CP_P4Ls CP_Protect._P4Ls
#define CP_P1Sms CP_Protect._P1Sms
#define CP_P2Sms CP_Protect._P2Sms
#define CP_P3Sms CP_Protect._P3Sms
#define CP_P4Sms CP_Protect._P4Sms
#define CP_PNOTms CP_Protect._PNOTms

typedef union
{
	u8 key;
	struct
	{
		u8 _JD1OUT : 1; //��λ0x01   //һ·���
		u8 _JD2OUT : 1; //��·���
		u8 _JD3OUT : 1; //��·���
		u8 _JD4OUT : 1; //��·���
		u8 _JD1IN : 1;	//һ·���
		u8 _JD2IN : 1;	//��·���
		u8 _JD3IN : 1;	//��·���
		u8 _JD4IN : 1;	//��·���
	} bits;
} _KEYOFFON;

extern _KEYOFFON KEYOFFON;

#define JD1OUT KEYOFFON.bits._JD1OUT
#define JD2OUT KEYOFFON.bits._JD2OUT
#define JD3OUT KEYOFFON.bits._JD3OUT
#define JD4OUT KEYOFFON.bits._JD4OUT

#define FJD1IN KEYOFFON.bits._JD1IN
#define FJD2IN KEYOFFON.bits._JD2IN
#define FJD3IN KEYOFFON.bits._JD3IN
#define FJD4IN KEYOFFON.bits._JD4IN
extern u8 key_clock[7];

#define Kyear key_clock[0]
#define Kmonth key_clock[1]
#define Kdate key_clock[2]
#define Kweek key_clock[3]
#define Khour key_clock[4]
#define Kmin key_clock[5]
#define Ksec key_clock[6]

#define KEYTNUB 1200
extern ulong SetVal;
extern ulong ABC4_val[4];	//4·����
extern ulong Uval_abc[3];	//3·��ѹ
extern ulong P_abc[4];		//����
extern uint16_t key_cnt[4]; //
extern uchar led_allon;		//
extern u8 phase_mod;		//
extern u16 keybackcnt;
extern u8 f_set;
extern u8 sel_line;

void KeyScan(void);

uint16_t get_keynub(void);
void key_ok(void);
void allledoff(void);
void allledon(void);
void dispclock(u8 ms);
void dis_smgled(void);
void dis_led(void);
void SetValAddDec(u8 nub);
void SaveSet(void);

#endif
