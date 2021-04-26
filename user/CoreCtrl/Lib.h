#ifndef _LIB_H
#define _LIB_H


#include "sys_def.h"
	//************************Public Constant definition************************
	#ifndef MAX
	#define MAX(x,y)    ( ((x)>=(y)) ? (x) :  (y) )
	#endif

	#define TRUE  	1
	#define FALSE 	0

	#define INVALID 0xff
	#define VALID 0x00
// ************************Interface function declaration******************************
	void WatchDog(void);

//	UINT16 ABS(INT16 i16Val);


	//Time related task
	void TimeTask(void);

	// ��ȡ��ǰʱ����ο�ʱ��ĺ��뼶ʱ���� u16MsRef �� ��ʼ�ο�ʱ��
	UINT16 GetMsTimeGap(UINT16 u16MsRef);
	// ��ȡ��ǰ���ʱ���׼ ��λ������
	UINT16 GetCurrMs(void);
	// ��ȡ��ǰʱ����ο�ʱ����뼶ʱ���� u8SecRef �� ��ʼ�ο�ʱ��
	UINT8 GetSecTimeGap(UINT8 u8SecRef);
	// ��ȡ��ǰ���ʱ���׼ ��λ����
	UINT8 GetCurrSec(void);
	// ��ȡ��ǰʱ����ο�ʱ��ķ��Ӽ�ʱ���� u8MinRef �� ��ʼ�ο�ʱ��
	UINT8 GetMinTimeGap(UINT8 u8MinRef);
	// ��ȡ��ǰ���ʱ���׼ ��λ������
	UINT8 GetCurrMin(void);
#endif /*----------FSC3P08DRV_H-----------------------------------------------------*/

