/*********************************************************
  Copyright (C), 2020, Alivi Co., Ltd.
  File name:      	core_proc.c
  Author: Alair	Version: 0.7       Date:  2020-07-08
  Description:    	Main entry, system threads initialization
  Others:         	n.a
  Function List:  	core_proc(void const *argument)

  Variable List:  	n.a
  Revision History:         
  Date:           Author:          Modification:
	2020-07-08      Alair         file create
*********************************************************/
#include "cmsis_os.h"  
#include "stm32f0xx.h"
#include "adc.h"
#include "dac.h"
#include <math.h>
#include "daq.h"
#include "dio.h"
#include "pid.h"
#include "global_var.h"
#include "sys_conf.h"
//#include "fanctrl.h"
#include "phasecutting.h"
#include "acfrequency.h"
//************************������ģ��ӿ�ȫ�ֱ���************************
   	//1us counter, Refresh in timer0 overflow interruption
    UINT32 g_u32Counter1us = 0;
    UINT32 g_u32Counter1us_TEST = 0;


//************************��ģ��ȫ�ֱ���************************

//************************Public global constant definition************************

	//Tmax us = 256 * T2DIVIDE / 11.0592
    #define T2DIVIDE8F 		0x02		//С��185us��ʱ�ķ�Ƶ
    #define T2DIVIDE32F 	0x03		//С��740us��ʱ�ķ�Ƶ
    #define T2DIVIDE64F 	0x04		//С��1481us��ʱ�ķ�Ƶ
    #define T2DIVIDE128F 	0x05		//С��2962us��ʱ�ķ�Ƶ
    #define T2DIVIDE256F 	0x06		//С��5925us��ʱ�ķ�Ƶ
    #define T2DIVIDE1024F 	0x07		//С��23703us��ʱ�ķ�Ƶ

	#define T2_DIVIDE_F T2DIVIDE64F

	#define TIMER2_150US 0x30
	#define TIMER2_100US 0x76
	#define TIMER2_50US  0xba
	#define TIMER2_40US  0xc8
	#define TIMER2_30US  0xd6
	#define TIMER2_20US  0xe4

	#define TIMER2_200US 0xdd
	#define TIMER2_255US 0xd4
	#define TIMER2_300US 0xcc
	#define TIMER2_400US 0xbb
	#define TIMER2_500US 0xaa
	// TIMER2_VALUE = 256 - T * 11059200 / (1000000 * T2_DIVIDE_F)
	// T = T2_DIVIDE_F * (256 - TIMER2_VALUE) * 1000000 / 11059200
//	#define TIMER2_VALUE 	TIMER2_255US
//					#define TIMER2_UNIT 255
//	#define TIMER2_CNT_UNIT 6	//��ʱ��2ÿ������ֵ��Ӧʱ��us	64/11.0592

	#define TIMER2_VALUE 	255-1
					#define TIMER2_UNIT 255
	#define TIMER2_CNT_UNIT 1	//��ʱ��2ÿ������ֵ��Ӧʱ��us	48/48
	

// ************************Public global variable definition************************
		#define INT_FLT_TM 50 //�����ȥ����ʱ���� INT_FLT_TM * TIMER2_UNIT ʱ�����10ms
	// ��ѹ�����������ⲿ�ж���Ч���˲����� ��λ��TIMER2_UNIT us
	UCHAR g_u8ExtISRFlt = 0;

	/* ����ɿع���������: ������ض� */
        #define SCRTRIGON  1
        #define SCRTRIGOFF 0
	UCHAR g_u8PaSCR = SCRTRIGOFF;

	// �ɿع败���������趨���� ��λ: us
	UINT16 g_u16SCRTrigTm = 0;

	// �ɿع败�������ȶ�ʱ����, ��λ: us
	UINT16 g_u16PaTrigWidthTimer = 0;

	// ������ƽǶ� ��Ӧ ��ʱ������ֵ
	UINT16 g_u16PhaseAngleTcnt = 0;

	// ���崥������Ƕ� 60�� ��Ӧ ��ʱ������ֵ
	UINT8 g_u8PulseCycTcnt = 0;

	// ÿ���������崥��˳����� �� ����ǶȲο���λ ����
	UCHAR g_u8TrigNo = 0;

	UCHAR g_u8T_CNT[2] = {0};

		#define FAN_PWRON_DELAY_TM 	3 		//�����������ʱ�ӳ�ʱ�� ��λ 20ms
	// �ɿع������ӳ�ʱ��
	UINT8 g_u8ScrStartDelayTm = 0;
		#define SCR_CTRL_GAP 		5		//�ɿع�ն���Ƕȱ仯ʱ����ʱ�� ��λ20ms
	// �ɿع�������ƽǱ仯�����ӳ�ʱ��
	UINT8 g_u8ScrAngleChgDelayTm = 0;

// ************************Interface function declaration******************************

		//Tmax us = 256 * T2DIVIDE / 11.0592
	    #define T0DIVIDE8F 		0x02		//С��185us��ʱ�ķ�Ƶ
	    #define T0DIVIDE64F 	0x03		//С��1481us��ʱ�ķ�Ƶ
	    #define T0DIVIDE256F 	0x04		//С��5925us��ʱ�ķ�Ƶ
	    #define T0DIVIDE1024F 	0x05		//С��23703us��ʱ�ķ�Ƶ

	    #define T0_DIVIDE_F T0DIVIDE256F

	//T0�ж�����
	void EnableT0(void);
	//T0�жϽ�ֹ
	void DisableT0(void);

	//T1�ж�����
	void EnableT1(void);
	//T1�жϽ�ֹ
	void DisableT1(void);

	//T2�ж�����
	void EnableT2(void);
	//T2�жϽ�ֹ
	void DisableT2(void);
	
	//T3�ж�����
	void EnableT3(void);
	//T3�жϽ�ֹ
	void DisableT3(void);

	//Enable external interruption 1
	void EnableEXINT(void);

	// ���¹����������ⲿ�ж�ȥ���˲���ʱ����   �� ��ʱ�� �����ʱ�ж��е���
	void vUpdateExtISRFltTm(void);

	// �ɿع�����ģ�鶨��
	void PaSCRON(void);
	void PaSCROFF(void);

	// ��ʼ������ƽǶ�ʱ���ڹ���㴥�����ⲿ�жϷ�������е���
	void vEnablePhaseAngleTiming(void);

	// �ɿع败�����������������
	void vSCRTrigOnCntrl(UINT8 u8TrigStatus);

	// �ɿع败��ֹͣ�������   �� ��ʱ�� �����ʱ�ж��е���
	void vSCRTrigOffCntrl(UINT16 u16TrigTm);

/*=============================================================================*
 * FUNCTION: vCalPhaseCuttingTime()
 * PURPOSE :
 * 			���� ������ƽ� ��Ӧ ��ʱ������ֵ g_u16PhaseAngleTcnt;
 * 			����������ƽ��趨 ն��������ʱ�� g_u16SCRTrigTm;
 * 			���� ն�����崥�����60�Ƚ� ��Ӧ ��ʱ������ֵ g_u8PulseCycTcnt;
 * INPUT:
 *     		UINT16 u16PhaseAngle ���ѹ������Ӧ��������ƽ� ��λ 0.1��
 *			UINT16 g_u16ACCyc	��ѹ���� ��λ us
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     	EepromAVRReadBlock()
 *		EepromAVRWriteBlock()
 *		CharStringCompare()
 *		DefaultCfgSet()
 *		DownloadRamCfg()
 *		ExceptionEepromWeFailue()
 * CALLED BY:
 *     	main()
 *
 *============================================================================*/
void vCalPhaseCuttingTime(UINT16 u16PhaseAngle)
{
	// ����ϵ����0.1��������ƽǶ�Ӧ��ʱ��1�ļ���ֵ
	static UINT16 s_u16CoeffCntByAngle = 0;
	// ������ƽǶ�Ӧ��ʱ��1�ļ���ֵ �� ���ڱ仯�ο�ֵ
	static UINT16 s_u16Timer1Cnt = 0, s_u16ACCycRef = 0;

	if(u16PhaseAngle >= SCR_OFF_ANGLE)
	{
		g_u16PhaseAngleTcnt = 0;	//ֹͣն���Ƕȶ�ʱ
		PaSCROFF();				//�ɿع���ƹر�״̬
		return;
	}
	else if(u16PhaseAngle == 0)
	{
		g_u16PhaseAngleTcnt = 0;	//ֹͣն���Ƕȶ�ʱ
		PaSCRON();				//�ɿع���ƿ���״̬
		return;
	}
	//����ϵ��������ֵ��Ӧ�Ƕ�  ˢ�� �� ���崥�����60�ȽǶ�Ӧ����ֵ ˢ��
	//ˢ���������ϵ��ʼ �� ���ڱ仯���� 100 uS
//	if(ABS((INT16)((INT32)s_u16ACCycRef - (INT32)g_u16ACCyc)) >= 100)
	if( ((s_u16ACCycRef > g_u16ACCyc) && (s_u16ACCycRef - g_u16ACCyc) >= 100) ||
		((s_u16ACCycRef < g_u16ACCyc) && (g_u16ACCyc - s_u16ACCycRef) >= 100) )
	{
		// ���� ÿ0.1��������ƽǶ�Ӧ��ʱ��1�ļ���ֵ �Ŵ�1000��
//Atmeg32
//		// g_u16ACCyc * 11059200 * 1000/ (3600 * 8000000)//7.68 = (20000 / 360) / (8 / 11.0592)
//		s_u16CoeffCntByAngle = (UINT16)((UINT32)g_u16ACCyc * 110592 / 288000);
//STM32
		// g_u16ACCyc * 48000000 * 1000/ (3600 * 48000000)
		s_u16CoeffCntByAngle = (UINT16)((UINT32)g_u16ACCyc * 1000 / 3600);//=5556Լ5.56us

		// ����������60�ȽǶ�Ӧ��ʱ������ֵ
//Atmeg32
//		// 256 - (UCHAR)((UINT32)g_u16ACCyc * 110592 / (60000 * 256))
//		g_u8PulseCycTcnt = 256 - (UCHAR)((UINT32)g_u16ACCyc * 110592 / 15360000);
//STM32
		// g_u16ACCyc * 48000000* 600 / (3600 * 48000000))
		g_u8PulseCycTcnt = (UINT16)((UINT32)g_u16ACCyc * 6 / 36);//=3333.3

		s_u16ACCycRef = g_u16ACCyc;
	}


	// ն��ԭ��
	//���������ƽ�����ѹ��·���ص㣺
	//1��ÿ���·����ͨ����һ���γɻ�·��
	//2�����ؽ������Ҳ��������ߣ�
	//3����բ�ܵĴ�����·������˫���壬�����ǿ�ȴ���60�ȵĵ����壻
	//4����������˳�������ȫ����һ����ΪT1 �� T6�����μ��60�㣻
	//5����ѹ���㴦��Ϊ���ƽǵ���㣬�����෶Χ��0�㡫150�㣻
		//0��� a <60�㣺���ܵ�ͨ�����ܵ�ͨ���棬ÿ�ܵ�ͨ180�㣭a ����a =0��ʱһֱ�����ܵ�ͨ
		//60��� a <90�㣺���ܵ�ͨ��ÿ�ܵ�ͨ120
		//90��� a <150�㣺���ܵ�ͨ���޾�բ�ܵ�ͨ���棬��ͨ�Ƕ�Ϊ300�㣭2 a
		//��բ�ܵĴ�����·������˫���壬�����ǿ�ȴ���60�ȵĵ����壻

	// 1 Phase ����ɿع败�������� ��Ӧʱ�� ��λuS
	g_u16SCRTrigTm = (UINT16)((UINT32)(1550 - u16PhaseAngle) * (UINT32)g_u16ACCyc / 3600);

	// 1 Phase ���ݵ�ǰ������ƽ� ���� ��ʱ������Ŀ��ֵ
//Atmeg32
//	s_u16Timer1Cnt = 65535 - (UINT16)((UINT32)(u16PhaseAngle) * (UINT32)s_u16CoeffCntByAngle / 1000);
//STM32
	s_u16Timer1Cnt = (UINT16)((UINT32)(u16PhaseAngle) * (UINT32)s_u16CoeffCntByAngle / 1000);
	g_sys.status.Test_Buff[10]=u16PhaseAngle;
	g_sys.status.Test_Buff[11]=g_u16SCRTrigTm;
	g_sys.status.Test_Buff[12]=s_u16Timer1Cnt;
	if(g_u16PhaseAngleTcnt == s_u16Timer1Cnt)
	{//��ʱ����ǰ����ֵ�����Ŀ��ֵ��ͬ������ˢ�¿���
		return;
	}

	//�������ʱ��ȫ�٣��ɿع�ȫ���򿪣����м������ڵ�ʱ�䣬����г������ʱ�ɿع����˳��ּ��
	if(g_u16PhaseAngleTcnt == 0)	// ����״̬
	{
		g_u8ScrStartDelayTm = FAN_PWRON_DELAY_TM;

		//����� 70 �ȿ�ʼ�𽥱ƽ�Ŀ��ֵ s_u16Timer1Cnt
		//TIMER1 = 65536 - 700 * s_u16CoeffCntByAngle / 1000
//Atmeg32
//		g_u16PhaseAngleTcnt = 65535 - s_u16CoeffCntByAngle * 7 / 10;	
//STM32		
		g_u16PhaseAngleTcnt =s_u16CoeffCntByAngle * 7 / 10;
		g_u8ScrAngleChgDelayTm =  SCR_CTRL_GAP;
	}

		//����Ƶ���仯��ʱ��1��ֵ ֻ�����ڱ仯������ߵ�һ��ʹ�����ڼ��㶨ʱ��ֵ
		//ֻ�нǶȱ仯ʱ �����Ⱥʹ��������ˢ��
	else if((g_u8ScrAngleChgDelayTm == 0) && (g_u8ScrStartDelayTm == 0))
	{//�Ƕȱ仯�ܴ�ʱ��ͨ�����ʵ�ֿ��ƵĿɿع�����м����,
		if(g_u16PhaseAngleTcnt > s_u16Timer1Cnt)
		{
			if((g_u16PhaseAngleTcnt - s_u16Timer1Cnt) > 100)	// 100 * 8 / 11.0592 us
			{
//				//ÿ�仯 0.1 �ȶ�Ӧ ������ֵ 7.68 = (20000 / 360) / (8 / 11.0592)
				//ÿ�仯 0.1 �ȶ�Ӧ ������ֵ 5.56
				g_u16PhaseAngleTcnt -= (g_u16PhaseAngleTcnt - s_u16Timer1Cnt) / 2;
			}
			else
			{
//				//ÿ�仯 0.1 �ȶ�Ӧ ������ֵ 7.68 = (20000 / 360) / (8 / 11.0592)
				//ÿ�仯 0.1 �ȶ�Ӧ ������ֵ 5.56
				g_u16PhaseAngleTcnt = s_u16Timer1Cnt;
			}
		}
		else
		{
			if((s_u16Timer1Cnt - g_u16PhaseAngleTcnt) > 100)
			{
//				//ÿ�仯 0.1 �ȶ�Ӧ ������ֵ 7.68 = (20000 / 360) / (8 / 11.0592)
				//ÿ�仯 0.1 �ȶ�Ӧ ������ֵ 5.56
				g_u16PhaseAngleTcnt += (s_u16Timer1Cnt - g_u16PhaseAngleTcnt) / 2;
			}
			else
			{
//				//ÿ�仯 0.1 �ȶ�Ӧ ������ֵ 7.68 = (20000 / 360) / (8 / 11.0592)
				//ÿ�仯 0.1 �ȶ�Ӧ ������ֵ 5.56
				g_u16PhaseAngleTcnt = s_u16Timer1Cnt;
			}
		}
		// ������ƽǱ仯�󱣳�ʱ��
		g_u8ScrAngleChgDelayTm =  SCR_CTRL_GAP;
	}
	return;
}


// ��ʼ������ƽǶ�ʱ���ڹ���㴥�����ⲿ�жϷ�������е���
void vEnablePhaseAngleTiming(void)
{
	UINT16	u16PhaseAngleTcnt;
	// �������״̬��ȫ���򿪿ɿع�����ı���ʱ��ˢ��
	if(g_u8ScrStartDelayTm)
	{
		PaSCRON();
		g_u8ScrStartDelayTm--;
	}
	// ��ѹ����״̬����ʱ������ֵ��Ϊ0ʱ, ִ��������Ƶ�ѹ
	else 
	if(g_u16PhaseAngleTcnt)
	{
		g_sys.status.Test_Buff[8]=g_u16PhaseAngleTcnt;
		u16PhaseAngleTcnt=g_u16PhaseAngleTcnt-g_u16AcZeroDelay[1];//��ȥ�����ƫ��
		// ������ƽǶ�ʱ�������ʱ�ж�����
//		TCNT1 = g_u16PhaseAngleTcnt;
////		EnableT1();
		g_u8T_CNT[1]=0;
		InitT1(u16PhaseAngleTcnt,ENABLE);
		//���ƽǶȱ仯ʱ����ʱ��
		if(g_u8ScrAngleChgDelayTm)
		{
			g_u8ScrAngleChgDelayTm--;
		}
	}
}

// �ɿع败�����������������
void vSCRTrigOnCntrl(UINT8 u8TrigStatus)
{
	u8TrigStatus = (u8TrigStatus % 6);

	if((u8TrigStatus == 0) || (u8TrigStatus == 3))
	{
		PaSCRON();
		g_u8PaSCR = SCRTRIGON;
		g_u16PaTrigWidthTimer = 0;
	}

	g_u8TrigNo++;
}

// �ɿع败��ֹͣ�������   �� ��ʱ�� �����ʱ�ж��е���
void vSCRTrigOffCntrl(UINT16 u16TrigTm)
{
    /* A��ɿع迪ʱ */
  if(g_u8PaSCR == SCRTRIGON)
	{
		g_u16PaTrigWidthTimer += u16TrigTm;
		if(g_u16PaTrigWidthTimer >= g_u16SCRTrigTm)
		{
			g_u8PaSCR = SCRTRIGOFF;
			PaSCROFF();
		}
	}
}

/*********************************************************
  * @name   led_init
	* @brief  led gpio and port bank clock initilization
	* @calls  gpio dirvers
  * @called bkg_proc()
  * @param  None
  * @retval None
*********************************************************/
void PaSCR_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Pin = SCR_PIN ;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed =GPIO_Speed_Level_3;
	GPIO_Init(SCR_PORT, &GPIO_InitStruct);
	GPIO_SetBits(SCR_PORT, SCR_PIN);
//	GPIO_ResetBits(SCR_PORT, SCR_PIN);
}

void PaSCROFF(void)
{
//	PORTD |= _BV(PD7);
	GPIO_SetBits(SCR_PORT, SCR_PIN);
//	GPIO_ResetBits(SCR_PORT, SCR_PIN);
}

void PaSCRON(void)
{
//	PORTD &=~ _BV(PD7);
	GPIO_ResetBits(SCR_PORT, SCR_PIN);
//	GPIO_SetBits(SCR_PORT, SCR_PIN);
}

///*=============================================================================*
// * FUNCTION: InitT0()
// * PURPOSE : Initiate STM32F051 timer1 module
// * INPUT:
// *     NONE
// *
// * RETURN:
// *     NONE
// *
// * CALLS:
// *     NONE;
// *
// * CALLED BY:
// *     InitMCU();
// *
// *============================================================================*/
//��ʱ5ms
void InitT0(uint16_t timPeriod,uint8_t u8Type)
{
	uint16_t PrescalerValue = 0;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* TIM15 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

	/* Compute the prescaler value */
	//SystemCoreClock = 48MHZ
	PrescalerValue = 48 - 1;//48��Ƶ,��һ����1us

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_Period = timPeriod;//1us*255=255us
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
	TIM_TimeBaseInit(TIM14,&TIM_TimeBaseStructure);
	TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);
	if(u8Type==ENABLE)
	{
		TIM_Cmd(TIM14, ENABLE);
		
		NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	else
	{
		DisableT0();		
	}
	
	return;		
}

//T0�ж�����
void EnableT0(void)
{
	TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
	TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM14, 0);
	TIM_Cmd(TIM14, ENABLE);	
	return;
}

//T0�жϽ�ֹ
void DisableT0(void)
{
	TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
	TIM_ITConfig(TIM14, TIM_IT_Update, DISABLE);
	TIM_SetCounter(TIM14, 0);
	TIM_Cmd(TIM14, DISABLE);
	return;
}


///*=============================================================================*
// * FUNCTION: InitT1()
// * PURPOSE : Initiate Atmega32 timer1 module
// * INPUT:
// *     NONE
// *
// * RETURN:
// *     NONE
// *
// * CALLS:
// *     NONE;
// *
// * CALLED BY:
// *     InitMCU();
// *
// *============================================================================*/
void InitT1(uint16_t timPeriod,uint8_t u8Type)
{
	uint16_t PrescalerValue = 0;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* TIM15 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);

	/* Compute the prescaler value */
	//SystemCoreClock = 48MHZ
	PrescalerValue = 48 - 1;//48��Ƶ,��һ����1us

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_Period = timPeriod;//1us*255=255us
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	TIM_TimeBaseInit(TIM15,&TIM_TimeBaseStructure);
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
	if(u8Type==ENABLE)
	{
		TIM_Cmd(TIM15, ENABLE);
		
		NVIC_InitStructure.NVIC_IRQChannel = TIM15_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	else
	{
		DisableT1();		
	}
	return;		
}

//T1�ж�����
void EnableT1(void)
{
	TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM15, 0);
	TIM_Cmd(TIM15, ENABLE);	
	return;
}

//T1�жϽ�ֹ
void DisableT1(void)
{
	TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
	TIM_SetCounter(TIM15, 0);
	TIM_Cmd(TIM15, DISABLE);
	return;
}

///*=============================================================================*
// * FUNCTION: InitT2()
// * PURPOSE : Initiate Atmega32 timer2 module
// * INPUT:
// *     NONE
// *
// * RETURN:
// *     NONE
// *
// * CALLS:
// *     NONE;
// *
// * CALLED BY:
// *     InitMCU();
// *
// *============================================================================*/
void InitT2(void)
{		
	uint16_t timPeriod = TIMER2_VALUE;
	uint16_t PrescalerValue = 0;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* TIM15 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);

	/* Compute the prescaler value */
	//SystemCoreClock = 48MHZ
	PrescalerValue = 48 - 1;//48��Ƶ,��һ����1us

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_Period = timPeriod;//1us*255=255us
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_ClearITPendingBit(TIM16, TIM_IT_Update);
	TIM_TimeBaseInit(TIM16,&TIM_TimeBaseStructure);
	TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM16, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM16_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	return;		
}

//T2�ж�����
void EnableT2(void)
{
	TIM_ClearITPendingBit(TIM16, TIM_IT_Update);
	TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM16, 0);
	TIM_Cmd(TIM16, ENABLE);	
	return;
}

//T2�жϽ�ֹ
void DisableT2(void)
{
	TIM_ClearITPendingBit(TIM16, TIM_IT_Update);
	TIM_ITConfig(TIM16, TIM_IT_Update, DISABLE);
	TIM_SetCounter(TIM16, 0);
	TIM_Cmd(TIM16, DISABLE);
	return;
}

/*=============================================================================*
 * FUNCTION: SIGNAL(TIMER0_OVF_vect)
 * PURPOSE : Timer0 overflow ISR
 * INPUT:
 *     NONE
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     NONE;
 *
 *============================================================================*/
void TIM14_IRQHandler(void)
{
	UINT16 i;
	UINT16 u16TimPeriod;
	if (TIM_GetITStatus(TIM14, TIM_IT_Update) != RESET)
	{

		TIM_ClearFlag(TIM14, TIM_FLAG_Update);	     		//���жϱ��
		TIM_ClearITPendingBit(TIM14, TIM_IT_Update);		//�����ʱ��T3����жϱ�־λ
		g_u8T_CNT[0]++;
		if(g_u8T_CNT[0]<=1)
		{
			return;
		}
		i = TIM14->CNT;
		if(i < g_u8PulseCycTcnt)
		{
//			TCNT0 = g_u8PulseCycTcnt + i;
			u16TimPeriod=g_u8PulseCycTcnt + i;
			InitT0(u16TimPeriod,ENABLE);
			// �ɿع败������������
			vSCRTrigOnCntrl(g_u8TrigNo);
			// ���һ�����崥������
			if(g_u8TrigNo >= 6)
			{
				DisableT0();
			}
		}

		return;
		}
}
//SIGNAL(TIMER0_OVF_vect)
//{
//	UCHAR i;

//	i = TCNT0;
//	if(i < g_u8PulseCycTcnt)
//	{
//		TCNT0 = g_u8PulseCycTcnt + i;

//		// �ɿع败������������
//		vSCRTrigOnCntrl(g_u8TrigNo);
//		// ���һ�����崥������
//		if(g_u8TrigNo >= 6)
//		{
//			DisableT0();
//		}
//	}

//	return;
//}

/*=============================================================================*
 * FUNCTION: SIGNAL(TIMER1_OVF_vect)
 * PURPOSE : Timer1 overflow ISR
 * INPUT:
 *     NONE
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     NONE;
 *
 *============================================================================*/
 void TIM15_IRQHandler(void)
{
	UINT16 i;
	UINT16 u16TimPeriod;
	if (TIM_GetITStatus(TIM15, TIM_IT_Update) != RESET)
	{
		
		TIM_ClearFlag(TIM15, TIM_FLAG_Update);	     		//���жϱ��
		TIM_ClearITPendingBit(TIM15, TIM_IT_Update);		//�����ʱ��T3����жϱ�־λ
		g_u8T_CNT[1]++;
		if(g_u8T_CNT[1]<=1)
		{
			return;
		}
//		i = TCNT1;
		i = TIM15->CNT;

		if(i < g_u16PhaseAngleTcnt)
		{
			g_u8TrigNo = 0;
			// �ɿع败������������
			vSCRTrigOnCntrl(g_u8TrigNo);

			// �������崥�����ڶ�ʱ��
//			TCNT0 = g_u8PulseCycTcnt;
			u16TimPeriod= g_u8PulseCycTcnt;
			g_u8T_CNT[0]=0;
			InitT0(u16TimPeriod,ENABLE);

			DisableT1();
		}
		return;
	}
}
//SIGNAL(TIMER1_OVF_vect)
//{
//	UINT16 i;


//	i = TCNT1;

//	if(i < g_u16PhaseAngleTcnt)
//	{
//		g_u8TrigNo = 0;
//		// �ɿع败������������
//		vSCRTrigOnCntrl(g_u8TrigNo);

//		// �������崥�����ڶ�ʱ��
//		TCNT0 = g_u8PulseCycTcnt;

//		EnableT0();

//		DisableT1();
//	}
//}

/*=============================================================================*
 * FUNCTION: SIGNAL(TIMER2_OVF_vect)
 * PURPOSE : Timer2 overflow ISR
 * INPUT:
 *     NONE
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     NONE;
 *
 *============================================================================*/
void TIM16_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM16, TIM_IT_Update) != RESET)
	{
//		__set_PRIMASK(1);	//�ر��ж�			
		TIM_ClearFlag(TIM16, TIM_FLAG_Update);	     		//���жϱ��
		TIM_ClearITPendingBit(TIM16, TIM_IT_Update);		//�����ʱ��T3����жϱ�־λ
	
   	//Refresh 1us counter
	g_u32Counter1us += TIMER2_UNIT;
		g_u32Counter1us_TEST+= 1;
	// �����������ⲿ�ж�ȥ���˲���ʱ��������
	vUpdateExtISRFltTm();

	// �������ڼ�ʱ��������
	vUpdateAcCyc(TIMER2_UNIT);

	// �ɿع败������رտ���
	vSCRTrigOffCntrl(TIMER2_UNIT);

//	__set_PRIMASK(0);	//�����ж�
	}
}
//SIGNAL(TIMER2_OVF_vect)
//{
//	UCHAR i;

//    //load counter initial value for timer2 overflow interruption��
//	i = TCNT2;
//	TCNT2 = TIMER2_VALUE + i;

//   	//Refresh 1us counter
//	g_u32Counter1us += TIMER2_UNIT;

//	// �����������ⲿ�ж�ȥ���˲���ʱ��������
//	vUpdateExtISRFltTm();

//	// �������ڼ�ʱ��������
//	vUpdateAcCyc(TIMER2_UNIT);

//	// �ɿع败������رտ���
//	vSCRTrigOffCntrl(TIMER2_UNIT);
//}

// ���� �������ڶ�ʱ�� ��ǰ��ʱʱ��ֵ ��λ��us
UINT16 u16GetT2Val(void)
{
	UINT16 i;
    //load counter initial value for timer2 overflow interruption��
	i = (UINT16)TIM16->CNT;
	if(i < TIMER2_VALUE)
	{//�����ж�
		return (TIMER2_UNIT + i * TIMER2_CNT_UNIT);
	}
	else
	{
		return TIMER2_CNT_UNIT * (i - TIMER2_VALUE);
	}
}


/*=============================================================================*
 * FUNCTION: InitEXInt1()
 * PURPOSE : Initiate Atmega32 external interruption 0, 1, and 2 module
 * INPUT:
 *     NONE
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     InitMCU();
 *
 *============================================================================*/
void InitEXINT1(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable GPIOA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	/* Configure PB3 pin as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Connect EXTI4 Line to PA4 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource3);

	/* Configure EXTI4&EXTI5 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI4&EXTI5 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_Init(&NVIC_InitStructure);

}

/*=============================================================================*
 * FUNCTION: InitEXInt2()
 * PURPOSE : Initiate STM32F051 external interruption 4 module
 * INPUT:
 *     NONE
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     InitMCU();
 *
 *============================================================================*/
void InitEXINT2(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable GPIOA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	/* Configure PB3 pin as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Connect EXTI4 Line to PB4 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);

	/* Configure EXTI4&EXTI5 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI4&EXTI5 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_Init(&NVIC_InitStructure);

}


/*=============================================================================*
 * FUNCTION: EXTI2_3_IRQHandler()
 * PURPOSE : External interruption 2_3
 * INPUT:
 *     NONE
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     NONE;
 *
 *============================================================================*/
void EXTI2_3_IRQHandler()//�½����ж�
{
	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		/* Clear the EXTI line 3 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line3);
		
		// ��ʼ������ƽǶ�ʱ���ڹ���㴥�����ⲿ�жϷ�������е���
		vEnablePhaseAngleTiming();			
		//�ж�ȥ������
		if(g_u8ExtISRFlt < INT_FLT_TM)
		{
			return;
		}
		//ȥ���˲�ʱ���ʼ��
		g_u8ExtISRFlt = 0;
		//�½���
		g_u8AcZeroFail=TRUE;
		// BC�߽������ڻ�ȡ
		vGetAcCyc(SAMPLE_CYC);
	}
}
/*=============================================================================*
 * FUNCTION: EXTI2_4_IRQHandler()
 * PURPOSE : External interruption 2_3
 * INPUT:
 *     NONE
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     NONE;
 *
 *============================================================================*/
void EXTI4_15_IRQHandler()//�����ش���
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line4);
//			// ��ʼ������ƽǶ�ʱ���ڹ���㴥�����ⲿ�жϷ�������е���
//			vEnablePhaseAngleTiming();
		if(g_u8AcZeroFail==TRUE)
		{
			g_u8AcZeroFail=FALSE;
			//�����
			vGetAcCyc(SAMPLE_ZERO);
		}
	}
}

// ���¹����������ⲿ�ж�ȥ���˲���ʱ����   �� ��ʱ�� �����ʱ�ж��е���
void vUpdateExtISRFltTm(void)
{
	// �����ȥ���˲�ʱ�����
	if(g_u8ExtISRFlt < INT_FLT_TM)
	{
		g_u8ExtISRFlt ++;
	}
}
