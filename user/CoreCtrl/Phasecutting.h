#ifndef _PHASECUTTING_H
#define _PHASECUTTING_H


#include "sys_def.h"

//************************Public global constant definition************************
		//�ɿع�ʼ�չرս� 150�� �Ŵ�10��
		#define SCR_OFF_ANGLE 1500
		//�ɿع�ʼ�յ�ͨ��
		#define SCRONANGLEVALUE 0
		
		#define SCR_PIN      	GPIO_Pin_8
		#define SCR_PORT     	GPIOA	
		
// ************************Public global variable definition************************
   	//1us counter, Refresh in timer2 overflow interruption
    extern UINT32 g_u32Counter1us;
		extern UINT16 g_u16PhaseAngleTcnt;


// ************************Interface function declaration******************************
	//Initate timer0 timing overflow interruption
	void InitT0(uint16_t timPeriod,uint8_t u8Type);
	void InitT1(uint16_t timPeriod,uint8_t u8Type);
	void InitT2(void);

	//Initiate external interruption
	void InitEXINT1(void);
	void InitEXINT2(void);
	void PaSCR_Init(void);
	// ���� �������ڶ�ʱ��T2 ��ǰ��ʱʱ��ֵ ��λ��us
	UINT16 u16GetT2Val(void);

	// ���ݽ������ڡ�������ƽǺ����崥������Ƕȣ�60�ȣ�����
	// ��Թ���㴥����ʼʱ��Ķ�ʱ��0����жϼ���ֵ ��
	// �������ʱ���Ӧ��ʱ1����жϼ���ֵ ��
	// ��������ʱ��
	void vCalPhaseCuttingTime(UINT16 u16PhaseAngle);

#endif


