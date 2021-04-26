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
#include "adc.h"
#include "dac.h"
#include <math.h>
#include "daq.h"
#include "dio.h"
#include "pid.h"
#include "global_var.h"
#include "sys_conf.h"
#include "acfrequency.h"
#include "PhaseCutting.h"
#include "calc.h"


// ************************Public global constant definition************************

		#define AC_CYC_OVERFLOW 25000

//************************Public global variable definition************************

	/* ��ѹ���ڼ�ʱ���� ��λ��uS */
	INT16 g_i16AcCycTimer = AC_CYC_OVERFLOW;

	/* ������ѹ���ڣ���λuS, ��Χ 18��22mS��45��55Hz�� */
	UINT16 g_u16RdACCyc = 0;
	UINT16 g_u16RdACCycSample = 0;

	/* ������ѹ���ڣ���λuS, ��Χ 18��22mS��45��65Hz�� */
	UINT16 g_u16ACCyc = 20000;

	// ����Ƶ��״̬ BIT0 - Ƶ���쳣; BIT1 - ���ཻ��ȱ��;
	UINT8 g_u8AcFrequencyStatus  = 0x00;
// ************************Interface function declaration******************************

	/* α���������ڼ�ʱ���� ��λ��uS */
	INT16 g_i16AcZeroTimer = 0;
	UINT16 g_u16AcZeroDelay[2] = {660,660};  
	UINT8 g_u8AcZeroFail = FALSE;  
	UINT8 g_u8AcFail = FALSE;  
	UINT8 g_u8Freerror = 0;  

/*=============================================================================*
 * FUNCTION: vUpdateAcCyc()
 * PURPOSE : �������ڼ�ʱ��������, ����ʱ������������ʱ��
 * INPUT:
 *     	UINT16 u16Time �����ۼӸ���ʱ��
 *
 * OUTPUT:
 *		g_i16AcCycTimer
 *
 * RETURN:
 *    	NONE
 *
 * CALLS:
 *     	NONE;
 *
 * CALLED BY:
 *     	�� ��ʱ�� �����ʱ�ж��е���
 *
 *============================================================================*/
void vUpdateAcCyc(UINT16 u16Time)
{
	//�������ڼ�ʱ����ˢ��
	if(g_i16AcCycTimer < AC_CYC_OVERFLOW)
	{
		g_i16AcCycTimer += u16Time;
		g_i16AcZeroTimer += u16Time;
	}
}

uint16_t AVGfilter(uint8_t i8Type,int16_t i16Value)
{
		UINT8 i;
		static UINT8 i8Num[SAMPLE_MAX]={0};
		static INT16 i16Value_buf[SAMPLE_MAX][NUM];
		int  sum=0;
		
		if(i8Num[i8Type]<NUM)
		{
			i8Num[i8Type]++;
		}
		else
		{
			i8Num[i8Type]=0;		
		}
		i16Value_buf[i8Type][i8Num[i8Type]] = i16Value;		
		bubble_sort(i16Value_buf[i8Type],NUM);
		for(i=2;i<NUM-2;i++)
		{
			sum += i16Value_buf[i8Type][i];
		}		
		sum/=(NUM-4);
		return (UINT16)(sum);			
}

/*=============================================================================*
 * FUNCTION: vGetAcCyc()
 * PURPOSE : ��ȡ�ߵ�ѹ��������
 * INPUT:
 *
 * OUTPUT:
 *		g_u16RdACCyc	��������
 * RETURN:
 *    	NONE
 *
 * CALLS:
 *     	u16GetT2Val();
 *
 * CALLED BY:
 *     	�������ܻ��߸����ܲ����Ĺ���㴥�����ⲿ�ж��е���
 *
 *============================================================================*/
void vGetAcCyc(UINT8 u8Type)
{
	UINT16 i;

	if(u8Type==SAMPLE_ZERO)
	{		
			i = u16GetT2Val();
			// �����½�����������֮���ʱ�䣬�м����Ϊ�����
			if(g_i16AcZeroTimer < AC_CYC_OVERFLOW)
			{
				g_u16AcZeroDelay[0] = g_i16AcZeroTimer + i;
				g_u16AcZeroDelay[0]=AVGfilter(SAMPLE_ZERO,g_u16AcZeroDelay[0]);
			}
			//ȡ�м��
			g_u16AcZeroDelay[1]=g_u16AcZeroDelay[0]/2;
			g_sys.status.Fan.ACCye[2]=g_u16AcZeroDelay[1];
					
	}
	else
	{
		//�н����ź�
		g_u8Freerror=0;
	// ���� �������ڶ�ʱ�� ��ǰ��ʱʱ��ֵ ��λ��us
		i = u16GetT2Val();
		// �������ڼ�ʱֵ��Ч�����¶�ȡ��������
		if(g_i16AcCycTimer < AC_CYC_OVERFLOW)
		{
			g_u16RdACCycSample = g_i16AcCycTimer + i;		
			g_u16RdACCyc=AVGfilter(SAMPLE_CYC,g_u16RdACCycSample);
		}
//		g_u16RdACCyc=AVGfilter(SAMPLE_CYC,g_u16RdACCycSample);
		g_sys.status.Fan.ACCye[0]=g_u16RdACCyc;
		//��ʼ����ѹ���ڶ�ʱʱ�����
		g_i16AcCycTimer = 0 - (INT16)i;		
		//��ʼ����ѹ���ڶ�ʱʱ�����
		g_i16AcZeroTimer = g_i16AcCycTimer;	
	}	
}

/*=============================================================================*
 * FUNCTION: ChkACFrequency()
 * PURPOSE : ���㽻������ �� �жϽ�����ѹƵ���Ƿ�����
 * INPUT:
 *		g_u16RdACCyc	��������
 *
 * OUTPUT:
 *		g_u16ACCyc	��������
 *		g_u8AcFrequencyStatus	Ƶ�ʸ澯״̬  BIT0 - Ƶ���쳣; BIT1 - ���ཻ��ȱ��;
 *
 * RETURN:
 *    	NONE
 *
 * CALLS:
 *
 *
 * CALLED BY:
 *     	�ڸ澯�����е���
 *
 *============================================================================*/
void ChkACFrequency(void)
{

		#define AC_CYC_UPLMT 22222	// ����������Ч���� ��λ��us
		#define AC_CYC_DNLMT 15384	// ����������Ч���� ��λ��us

	#define AC_ERROR_TM 30 //�����������ʱ�� us

	//������ѹ���� �仯�������ֵ������ȡ����
//		#define AC_CYC_FLT_INIT_TM 120
//		#define AC_CYC_FLT_TM_UP 170
//		#define AC_CYC_FLT_TM_DN 70
		#define AC_CYC_FLT_INIT_TM 12
		#define AC_CYC_FLT_TM_UP 17
		#define AC_CYC_FLT_TM_DN 70
		#define AC_FRE_NUM 5

	static UCHAR s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;

	// ����Ƶ�ʸ澯�����ϵ��ӳ�ʱ��
	static UCHAR s_u8ACFAlmPwrOnDlyTm = 0;

	//������ѹ���ڶ�ȡ���˲�	���ڱ仯���� AC_ERROR_TM us ʱ�����˲�����
	if(g_u16RdACCyc)
	{
		if(ABS((INT16) ((INT32)g_u16RdACCyc - (INT32)g_u16ACCyc)) > AC_ERROR_TM)
		{
			if(g_u16RdACCyc > g_u16ACCyc)
			{	//�����ɱ�С�����ʱ�˲�ʱ���ʼ��
				if(s_u8ACCycFltTm < AC_CYC_FLT_INIT_TM)
				{
					s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;
				}
				else
				{
					s_u8ACCycFltTm++;
				}
				if(s_u8ACCycFltTm > AC_CYC_FLT_TM_UP)	//�˲���������
				{
					s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;
					g_u16ACCyc = g_u16RdACCyc;
				}
			}
			else
			{	//�����ɱ�󵽱�Сʱ�˲�ʱ���ʼ��
				if(s_u8ACCycFltTm > AC_CYC_FLT_INIT_TM)
				{
					s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;
				}
				else
				{
					s_u8ACCycFltTm--;
				}
				if(s_u8ACCycFltTm < AC_CYC_FLT_TM_DN)	//�˲���������
				{
					s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;
					g_u16ACCyc = g_u16RdACCyc;
				}
			}
		}
		else
		{
			s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;
		}

		s_u8ACFAlmPwrOnDlyTm++;
		g_u16RdACCyc = 0;
	}
		g_sys.status.Fan.ACCye[1]=g_u16ACCyc;
		g_u8Freerror++;

	//�ϵ�AC_CYC_FLT_TM_DN �������ں������澯
	if(s_u8ACFAlmPwrOnDlyTm >= AC_CYC_FLT_TM_DN)
	{
		s_u8ACFAlmPwrOnDlyTm = AC_CYC_FLT_TM_DN;

		//Ƶ���쳣�澯����
		if((g_u16ACCyc > AC_CYC_UPLMT) || (g_u16ACCyc < AC_CYC_DNLMT))
		{
			// Ƶ������澯״̬����
			g_u8AcFrequencyStatus |= 0x01;

		}
		else
		{
			// Ƶ������澯״̬��λ
			g_u8AcFrequencyStatus &= ~0x01;
		}
	}
	//��ʱ��δ�ɼ��������źŲ�
	if(g_u8Freerror>=AC_FRE_NUM)
	{
			g_u8Freerror=AC_FRE_NUM;
			g_u8AcFrequencyStatus |= 0x02;		
	}
	else
	{
			g_u8AcFrequencyStatus &= ~ 0x02;				
	}
	
	//���������½���δ�ɼ���
}
