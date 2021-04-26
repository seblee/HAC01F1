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

	/* 电压周期计时变量 单位：uS */
	INT16 g_i16AcCycTimer = AC_CYC_OVERFLOW;

	/* 交流电压周期，单位uS, 范围 18～22mS（45～55Hz） */
	UINT16 g_u16RdACCyc = 0;
	UINT16 g_u16RdACCycSample = 0;

	/* 交流电压周期，单位uS, 范围 18～22mS（45～65Hz） */
	UINT16 g_u16ACCyc = 20000;

	// 交流频率状态 BIT0 - 频率异常; BIT1 - 三相交流缺相;
	UINT8 g_u8AcFrequencyStatus  = 0x00;
// ************************Interface function declaration******************************

	/* 伪过零间隔周期计时变量 单位：uS */
	INT16 g_i16AcZeroTimer = 0;
	UINT16 g_u16AcZeroDelay[2] = {660,660};  
	UINT8 g_u8AcZeroFail = FALSE;  
	UINT8 g_u8AcFail = FALSE;  
	UINT8 g_u8Freerror = 0;  

/*=============================================================================*
 * FUNCTION: vUpdateAcCyc()
 * PURPOSE : 交流周期计时变量更新, 将计时变量加上增加时间
 * INPUT:
 *     	UINT16 u16Time 周期累加更新时间
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
 *     	在 定时器 溢出定时中断中调用
 *
 *============================================================================*/
void vUpdateAcCyc(UINT16 u16Time)
{
	//交流周期计时变量刷新
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
 * PURPOSE : 获取线电压交流周期
 * INPUT:
 *
 * OUTPUT:
 *		g_u16RdACCyc	交流周期
 * RETURN:
 *    	NONE
 *
 * CALLS:
 *     	u16GetT2Val();
 *
 * CALLED BY:
 *     	在正半周或者负半周产生的过零点触发的外部中断中调用
 *
 *============================================================================*/
void vGetAcCyc(UINT8 u8Type)
{
	UINT16 i;

	if(u8Type==SAMPLE_ZERO)
	{		
			i = u16GetT2Val();
			// 交流下降沿与上升沿之间的时间，中间点则为过零点
			if(g_i16AcZeroTimer < AC_CYC_OVERFLOW)
			{
				g_u16AcZeroDelay[0] = g_i16AcZeroTimer + i;
				g_u16AcZeroDelay[0]=AVGfilter(SAMPLE_ZERO,g_u16AcZeroDelay[0]);
			}
			//取中间点
			g_u16AcZeroDelay[1]=g_u16AcZeroDelay[0]/2;
			g_sys.status.Fan.ACCye[2]=g_u16AcZeroDelay[1];
					
	}
	else
	{
		//有交流信号
		g_u8Freerror=0;
	// 保存 交流周期定时器 当前定时时间值 单位：us
		i = u16GetT2Val();
		// 交流周期计时值有效条件下读取交流周期
		if(g_i16AcCycTimer < AC_CYC_OVERFLOW)
		{
			g_u16RdACCycSample = g_i16AcCycTimer + i;		
			g_u16RdACCyc=AVGfilter(SAMPLE_CYC,g_u16RdACCycSample);
		}
//		g_u16RdACCyc=AVGfilter(SAMPLE_CYC,g_u16RdACCycSample);
		g_sys.status.Fan.ACCye[0]=g_u16RdACCyc;
		//初始化电压周期定时时间变量
		g_i16AcCycTimer = 0 - (INT16)i;		
		//初始化电压周期定时时间变量
		g_i16AcZeroTimer = g_i16AcCycTimer;	
	}	
}

/*=============================================================================*
 * FUNCTION: ChkACFrequency()
 * PURPOSE : 计算交流周期 与 判断交流电压频率是否正常
 * INPUT:
 *		g_u16RdACCyc	交流周期
 *
 * OUTPUT:
 *		g_u16ACCyc	交流周期
 *		g_u8AcFrequencyStatus	频率告警状态  BIT0 - 频率异常; BIT1 - 三相交流缺相;
 *
 * RETURN:
 *    	NONE
 *
 * CALLS:
 *
 *
 * CALLED BY:
 *     	在告警处理中调用
 *
 *============================================================================*/
void ChkACFrequency(void)
{

		#define AC_CYC_UPLMT 22222	// 交流周期有效上限 单位：us
		#define AC_CYC_DNLMT 15384	// 交流周期有效下限 单位：us

	#define AC_ERROR_TM 30 //交流周期误差时间 us

	//交流电压周期 变化超过误差值连续读取次数
//		#define AC_CYC_FLT_INIT_TM 120
//		#define AC_CYC_FLT_TM_UP 170
//		#define AC_CYC_FLT_TM_DN 70
		#define AC_CYC_FLT_INIT_TM 12
		#define AC_CYC_FLT_TM_UP 17
		#define AC_CYC_FLT_TM_DN 70
		#define AC_FRE_NUM 5

	static UCHAR s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;

	// 交流频率告警处理上电延迟时间
	static UCHAR s_u8ACFAlmPwrOnDlyTm = 0;

	//交流电压周期读取与滤波	周期变化超过 AC_ERROR_TM us 时进行滤波处理
	if(g_u16RdACCyc)
	{
		if(ABS((INT16) ((INT32)g_u16RdACCyc - (INT32)g_u16ACCyc)) > AC_ERROR_TM)
		{
			if(g_u16RdACCyc > g_u16ACCyc)
			{	//周期由变小到变大时滤波时间初始化
				if(s_u8ACCycFltTm < AC_CYC_FLT_INIT_TM)
				{
					s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;
				}
				else
				{
					s_u8ACCycFltTm++;
				}
				if(s_u8ACCycFltTm > AC_CYC_FLT_TM_UP)	//滤波次数结束
				{
					s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;
					g_u16ACCyc = g_u16RdACCyc;
				}
			}
			else
			{	//周期由变大到变小时滤波时间初始化
				if(s_u8ACCycFltTm > AC_CYC_FLT_INIT_TM)
				{
					s_u8ACCycFltTm = AC_CYC_FLT_INIT_TM;
				}
				else
				{
					s_u8ACCycFltTm--;
				}
				if(s_u8ACCycFltTm < AC_CYC_FLT_TM_DN)	//滤波次数结束
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

	//上电AC_CYC_FLT_TM_DN 交流周期后处理交流告警
	if(s_u8ACFAlmPwrOnDlyTm >= AC_CYC_FLT_TM_DN)
	{
		s_u8ACFAlmPwrOnDlyTm = AC_CYC_FLT_TM_DN;

		//频率异常告警处理
		if((g_u16ACCyc > AC_CYC_UPLMT) || (g_u16ACCyc < AC_CYC_DNLMT))
		{
			// 频率溢出告警状态设置
			g_u8AcFrequencyStatus |= 0x01;

		}
		else
		{
			// 频率溢出告警状态复位
			g_u8AcFrequencyStatus &= ~0x01;
		}
	}
	//长时间未采集到交流信号波
	if(g_u8Freerror>=AC_FRE_NUM)
	{
			g_u8Freerror=AC_FRE_NUM;
			g_u8AcFrequencyStatus |= 0x02;		
	}
	else
	{
			g_u8AcFrequencyStatus &= ~ 0x02;				
	}
	
	//交流周期下降沿未采集到
}
