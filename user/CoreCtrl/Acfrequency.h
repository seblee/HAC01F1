#ifndef __ACFrequency_H__
#define __ACFrequency_H__

#include "sys_def.h"

// ************************Public global constant definition************************
#define NUM 10
enum
{
	SAMPLE_ZERO = 0,
	SAMPLE_CYC ,
	SAMPLE_MAX,
};
// ************************Public global variable definition************************
	/* ������ѹ���ڣ���λuS, ��Χ 15��22mS��45��65Hz�� */
	extern UINT16 g_u16ACCyc;
	// ����Ƶ��״̬ BIT0 - Ƶ���쳣; BIT1 - ���ཻ��ȱ��;
	extern UINT8 g_u8AcFrequencyStatus;
	extern UINT8 g_u8AcZeroFail;
	extern UINT16 g_u16AcZeroDelay[2];
// ************************Interface function declaration******************************

	// �������ڼ�ʱ�������� �� ��ʱ�� �����ʱ�ж��е���
	void vUpdateAcCyc(UINT16 u16Time);
	// ��ȡ�ߵ�ѹ�������� �ڹ���㴥�����ⲿ�ж��е���
	void vGetAcCyc(UINT8 u8Type);
	// ���㽻������ �� �жϽ�����ѹƵ���Ƿ����� 45~65HZΪ���� �Ƿ�ȱ��
	void ChkACFrequency(void);
	uint16_t AVGfilter(uint8_t i8Type,int16_t i16Value);
//	extern UINT16 ABS(INT16 i16Val);
#endif


