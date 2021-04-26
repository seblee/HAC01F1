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
//************************与其他模块接口全局变量************************
   	//1us counter, Refresh in timer0 overflow interruption
    UINT32 g_u32Counter1us = 0;
    UINT32 g_u32Counter1us_TEST = 0;


//************************本模块全局变量************************

//************************Public global constant definition************************

	//Tmax us = 256 * T2DIVIDE / 11.0592
    #define T2DIVIDE8F 		0x02		//小于185us定时的分频
    #define T2DIVIDE32F 	0x03		//小于740us定时的分频
    #define T2DIVIDE64F 	0x04		//小于1481us定时的分频
    #define T2DIVIDE128F 	0x05		//小于2962us定时的分频
    #define T2DIVIDE256F 	0x06		//小于5925us定时的分频
    #define T2DIVIDE1024F 	0x07		//小于23703us定时的分频

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
//	#define TIMER2_CNT_UNIT 6	//定时器2每个计数值对应时间us	64/11.0592

	#define TIMER2_VALUE 	255-1
					#define TIMER2_UNIT 255
	#define TIMER2_CNT_UNIT 1	//定时器2每个计数值对应时间us	48/48
	

// ************************Public global variable definition************************
		#define INT_FLT_TM 50 //过零点去抖定时常量 INT_FLT_TM * TIMER2_UNIT 时间大于10ms
	// 电压过零点产生的外部中断有效性滤波变量 单位：TIMER2_UNIT us
	UCHAR g_u8ExtISRFlt = 0;

	/* 三相可控硅驱动变量: 开启与关断 */
        #define SCRTRIGON  1
        #define SCRTRIGOFF 0
	UCHAR g_u8PaSCR = SCRTRIGOFF;

	// 可控硅触发脉冲宽度设定变量 单位: us
	UINT16 g_u16SCRTrigTm = 0;

	// 可控硅触发脉冲宽度定时变量, 单位: us
	UINT16 g_u16PaTrigWidthTimer = 0;

	// 移相控制角度 对应 定时器计数值
	UINT16 g_u16PhaseAngleTcnt = 0;

	// 脉冲触发间隔角度 60度 对应 定时器计数值
	UINT8 g_u8PulseCycTcnt = 0;

	// 每个周期脉冲触发顺序序号 与 移相角度参考相位 变量
	UCHAR g_u8TrigNo = 0;

	UCHAR g_u8T_CNT[2] = {0};

		#define FAN_PWRON_DELAY_TM 	3 		//风机供电启动时延迟时间 单位 20ms
	// 可控硅启动延迟时间
	UINT8 g_u8ScrStartDelayTm = 0;
		#define SCR_CTRL_GAP 		5		//可控硅斩波角度变化时保持时间 单位20ms
	// 可控硅移相控制角变化控制延迟时间
	UINT8 g_u8ScrAngleChgDelayTm = 0;

// ************************Interface function declaration******************************

		//Tmax us = 256 * T2DIVIDE / 11.0592
	    #define T0DIVIDE8F 		0x02		//小于185us定时的分频
	    #define T0DIVIDE64F 	0x03		//小于1481us定时的分频
	    #define T0DIVIDE256F 	0x04		//小于5925us定时的分频
	    #define T0DIVIDE1024F 	0x05		//小于23703us定时的分频

	    #define T0_DIVIDE_F T0DIVIDE256F

	//T0中断允许
	void EnableT0(void);
	//T0中断禁止
	void DisableT0(void);

	//T1中断允许
	void EnableT1(void);
	//T1中断禁止
	void DisableT1(void);

	//T2中断允许
	void EnableT2(void);
	//T2中断禁止
	void DisableT2(void);
	
	//T3中断允许
	void EnableT3(void);
	//T3中断禁止
	void DisableT3(void);

	//Enable external interruption 1
	void EnableEXINT(void);

	// 更新过零点产生的外部中断去抖滤波计时变量   在 定时器 溢出定时中断中调用
	void vUpdateExtISRFltTm(void);

	// 可控硅驱动模块定义
	void PaSCRON(void);
	void PaSCROFF(void);

	// 开始移相控制角定时，在过零点触发的外部中断服务程序中调用
	void vEnablePhaseAngleTiming(void);

	// 可控硅触发脉冲启动控制输出
	void vSCRTrigOnCntrl(UINT8 u8TrigStatus);

	// 可控硅触发停止控制输出   在 定时器 溢出定时中断中调用
	void vSCRTrigOffCntrl(UINT16 u16TrigTm);

/*=============================================================================*
 * FUNCTION: vCalPhaseCuttingTime()
 * PURPOSE :
 * 			计算 移相控制角 对应 定时器计数值 g_u16PhaseAngleTcnt;
 * 			根据移相控制角设定 斩波脉冲宽度时间 g_u16SCRTrigTm;
 * 			计算 斩波脉冲触发间隔60度角 对应 定时器计数值 g_u8PulseCycTcnt;
 * INPUT:
 *     		UINT16 u16PhaseAngle 相电压过零点对应的移相控制角 单位 0.1度
 *			UINT16 g_u16ACCyc	电压周期 单位 us
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
	// 比例系数：0.1度移相控制角对应定时器1的计数值
	static UINT16 s_u16CoeffCntByAngle = 0;
	// 移相控制角对应定时器1的计数值 和 周期变化参考值
	static UINT16 s_u16Timer1Cnt = 0, s_u16ACCycRef = 0;

	if(u16PhaseAngle >= SCR_OFF_ANGLE)
	{
		g_u16PhaseAngleTcnt = 0;	//停止斩波角度定时
		PaSCROFF();				//可控硅控制关闭状态
		return;
	}
	else if(u16PhaseAngle == 0)
	{
		g_u16PhaseAngleTcnt = 0;	//停止斩波角度定时
		PaSCRON();				//可控硅控制开启状态
		return;
	}
	//比例系数－计数值对应角度  刷新 与 脉冲触发间隔60度角对应计数值 刷新
	//刷新条件：上电初始 或 周期变化大于 100 uS
//	if(ABS((INT16)((INT32)s_u16ACCycRef - (INT32)g_u16ACCyc)) >= 100)
	if( ((s_u16ACCycRef > g_u16ACCyc) && (s_u16ACCycRef - g_u16ACCyc) >= 100) ||
		((s_u16ACCycRef < g_u16ACCyc) && (g_u16ACCyc - s_u16ACCycRef) >= 100) )
	{
		// 计算 每0.1度移相控制角对应定时器1的计数值 放大1000倍
//Atmeg32
//		// g_u16ACCyc * 11059200 * 1000/ (3600 * 8000000)//7.68 = (20000 / 360) / (8 / 11.0592)
//		s_u16CoeffCntByAngle = (UINT16)((UINT32)g_u16ACCyc * 110592 / 288000);
//STM32
		// g_u16ACCyc * 48000000 * 1000/ (3600 * 48000000)
		s_u16CoeffCntByAngle = (UINT16)((UINT32)g_u16ACCyc * 1000 / 3600);//=5556约5.56us

		// 触发脉冲间隔60度角对应定时器计数值
//Atmeg32
//		// 256 - (UCHAR)((UINT32)g_u16ACCyc * 110592 / (60000 * 256))
//		g_u8PulseCycTcnt = 256 - (UCHAR)((UINT32)g_u16ACCyc * 110592 / 15360000);
//STM32
		// g_u16ACCyc * 48000000* 600 / (3600 * 48000000))
		g_u8PulseCycTcnt = (UINT16)((UINT32)g_u16ACCyc * 6 / 36);//=3333.3

		s_u16ACCycRef = g_u16ACCyc;
	}


	// 斩波原理
	//三相三线制交流调压电路的特点：
	//1）每相电路必须通过另一相形成回路；
	//2）负载接线灵活，且不用中性线；
	//3）晶闸管的触发电路必须是双脉冲，或者是宽度大于60度的单脉冲；
	//4）触发脉冲顺序和三相全控桥一样，为T1 ～ T6，依次间隔60°；
	//5）电压过零处定为控制角的起点，角移相范围是0°～150°；
		//0°≤ a <60°：三管导通与两管导通交替，每管导通180°－a 。但a =0°时一直是三管导通
		//60°≤ a <90°：两管导通，每管导通120
		//90°≤ a <150°：两管导通与无晶闸管导通交替，导通角度为300°－2 a
		//晶闸管的触发电路必须是双脉冲，或者是宽度大于60度的单脉冲；

	// 1 Phase 计算可控硅触发脉冲宽度 对应时间 单位uS
	g_u16SCRTrigTm = (UINT16)((UINT32)(1550 - u16PhaseAngle) * (UINT32)g_u16ACCyc / 3600);

	// 1 Phase 根据当前移相控制角 计算 定时器计数目标值
//Atmeg32
//	s_u16Timer1Cnt = 65535 - (UINT16)((UINT32)(u16PhaseAngle) * (UINT32)s_u16CoeffCntByAngle / 1000);
//STM32
	s_u16Timer1Cnt = (UINT16)((UINT32)(u16PhaseAngle) * (UINT32)s_u16CoeffCntByAngle / 1000);
	g_sys.status.Test_Buff[10]=u16PhaseAngle;
	g_sys.status.Test_Buff[11]=g_u16SCRTrigTm;
	g_sys.status.Test_Buff[12]=s_u16Timer1Cnt;
	if(g_u16PhaseAngleTcnt == s_u16Timer1Cnt)
	{//定时器当前计数值与计数目标值相同，不用刷新控制
		return;
	}

	//风机启动时先全速（可控硅全部打开）运行几个周期的时间，避免谐波测试时可控硅两端出现尖峰
	if(g_u16PhaseAngleTcnt == 0)	// 启动状态
	{
		g_u8ScrStartDelayTm = FAN_PWRON_DELAY_TM;

		//风机从 70 度开始逐渐逼近目标值 s_u16Timer1Cnt
		//TIMER1 = 65536 - 700 * s_u16CoeffCntByAngle / 1000
//Atmeg32
//		g_u16PhaseAngleTcnt = 65535 - s_u16CoeffCntByAngle * 7 / 10;	
//STM32		
		g_u16PhaseAngleTcnt =s_u16CoeffCntByAngle * 7 / 10;
		g_u8ScrAngleChgDelayTm =  SCR_CTRL_GAP;
	}

		//避免频繁变化定时器1初值 只有周期变化过多或者第一次使用周期计算定时器值
		//只有角度变化时 脉冲宽度和触发间隔不刷新
	else if((g_u8ScrAngleChgDelayTm == 0) && (g_u8ScrStartDelayTm == 0))
	{//角度变化很大时，通过多次实现控制的可控硅控制中间过程,
		if(g_u16PhaseAngleTcnt > s_u16Timer1Cnt)
		{
			if((g_u16PhaseAngleTcnt - s_u16Timer1Cnt) > 100)	// 100 * 8 / 11.0592 us
			{
//				//每变化 0.1 度对应 计数器值 7.68 = (20000 / 360) / (8 / 11.0592)
				//每变化 0.1 度对应 计数器值 5.56
				g_u16PhaseAngleTcnt -= (g_u16PhaseAngleTcnt - s_u16Timer1Cnt) / 2;
			}
			else
			{
//				//每变化 0.1 度对应 计数器值 7.68 = (20000 / 360) / (8 / 11.0592)
				//每变化 0.1 度对应 计数器值 5.56
				g_u16PhaseAngleTcnt = s_u16Timer1Cnt;
			}
		}
		else
		{
			if((s_u16Timer1Cnt - g_u16PhaseAngleTcnt) > 100)
			{
//				//每变化 0.1 度对应 计数器值 7.68 = (20000 / 360) / (8 / 11.0592)
				//每变化 0.1 度对应 计数器值 5.56
				g_u16PhaseAngleTcnt += (s_u16Timer1Cnt - g_u16PhaseAngleTcnt) / 2;
			}
			else
			{
//				//每变化 0.1 度对应 计数器值 7.68 = (20000 / 360) / (8 / 11.0592)
				//每变化 0.1 度对应 计数器值 5.56
				g_u16PhaseAngleTcnt = s_u16Timer1Cnt;
			}
		}
		// 移相控制角变化后保持时间
		g_u8ScrAngleChgDelayTm =  SCR_CTRL_GAP;
	}
	return;
}


// 开始移相控制角定时，在过零点触发的外部中断服务程序中调用
void vEnablePhaseAngleTiming(void)
{
	UINT16	u16PhaseAngleTcnt;
	// 风机启动状态，全部打开可控硅输出的保持时间刷新
	if(g_u8ScrStartDelayTm)
	{
		PaSCRON();
		g_u8ScrStartDelayTm--;
	}
	// 调压控制状态，定时器计数值不为0时, 执行移相控制调压
	else 
	if(g_u16PhaseAngleTcnt)
	{
		g_sys.status.Test_Buff[8]=g_u16PhaseAngleTcnt;
		u16PhaseAngleTcnt=g_u16PhaseAngleTcnt-g_u16AcZeroDelay[1];//减去过零点偏差
		// 移相控制角定时器溢出定时中断启动
//		TCNT1 = g_u16PhaseAngleTcnt;
////		EnableT1();
		g_u8T_CNT[1]=0;
		InitT1(u16PhaseAngleTcnt,ENABLE);
		//控制角度变化时保持时间
		if(g_u8ScrAngleChgDelayTm)
		{
			g_u8ScrAngleChgDelayTm--;
		}
	}
}

// 可控硅触发脉冲启动控制输出
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

// 可控硅触发停止控制输出   在 定时器 溢出定时中断中调用
void vSCRTrigOffCntrl(UINT16 u16TrigTm)
{
    /* A相可控硅开时 */
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
//定时5ms
void InitT0(uint16_t timPeriod,uint8_t u8Type)
{
	uint16_t PrescalerValue = 0;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* TIM15 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

	/* Compute the prescaler value */
	//SystemCoreClock = 48MHZ
	PrescalerValue = 48 - 1;//48分频,计一个数1us

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

//T0中断允许
void EnableT0(void)
{
	TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
	TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM14, 0);
	TIM_Cmd(TIM14, ENABLE);	
	return;
}

//T0中断禁止
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
	PrescalerValue = 48 - 1;//48分频,计一个数1us

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

//T1中断允许
void EnableT1(void)
{
	TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM15, 0);
	TIM_Cmd(TIM15, ENABLE);	
	return;
}

//T1中断禁止
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
	PrescalerValue = 48 - 1;//48分频,计一个数1us

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

//T2中断允许
void EnableT2(void)
{
	TIM_ClearITPendingBit(TIM16, TIM_IT_Update);
	TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM16, 0);
	TIM_Cmd(TIM16, ENABLE);	
	return;
}

//T2中断禁止
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

		TIM_ClearFlag(TIM14, TIM_FLAG_Update);	     		//清中断标记
		TIM_ClearITPendingBit(TIM14, TIM_IT_Update);		//清除定时器T3溢出中断标志位
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
			// 可控硅触发脉冲控制输出
			vSCRTrigOnCntrl(g_u8TrigNo);
			// 最后一个脉冲触发结束
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

//		// 可控硅触发脉冲控制输出
//		vSCRTrigOnCntrl(g_u8TrigNo);
//		// 最后一个脉冲触发结束
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
		
		TIM_ClearFlag(TIM15, TIM_FLAG_Update);	     		//清中断标记
		TIM_ClearITPendingBit(TIM15, TIM_IT_Update);		//清除定时器T3溢出中断标志位
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
			// 可控硅触发脉冲控制输出
			vSCRTrigOnCntrl(g_u8TrigNo);

			// 启动脉冲触发周期定时器
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
//		// 可控硅触发脉冲控制输出
//		vSCRTrigOnCntrl(g_u8TrigNo);

//		// 启动脉冲触发周期定时器
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
//		__set_PRIMASK(1);	//关闭中断			
		TIM_ClearFlag(TIM16, TIM_FLAG_Update);	     		//清中断标记
		TIM_ClearITPendingBit(TIM16, TIM_IT_Update);		//清除定时器T3溢出中断标志位
	
   	//Refresh 1us counter
	g_u32Counter1us += TIMER2_UNIT;
		g_u32Counter1us_TEST+= 1;
	// 过零点产生的外部中断去抖滤波计时变量更新
	vUpdateExtISRFltTm();

	// 交流周期计时变量更新
	vUpdateAcCyc(TIMER2_UNIT);

	// 可控硅触发脉冲关闭控制
	vSCRTrigOffCntrl(TIMER2_UNIT);

//	__set_PRIMASK(0);	//开启中断
	}
}
//SIGNAL(TIMER2_OVF_vect)
//{
//	UCHAR i;

//    //load counter initial value for timer2 overflow interruption；
//	i = TCNT2;
//	TCNT2 = TIMER2_VALUE + i;

//   	//Refresh 1us counter
//	g_u32Counter1us += TIMER2_UNIT;

//	// 过零点产生的外部中断去抖滤波计时变量更新
//	vUpdateExtISRFltTm();

//	// 交流周期计时变量更新
//	vUpdateAcCyc(TIMER2_UNIT);

//	// 可控硅触发脉冲关闭控制
//	vSCRTrigOffCntrl(TIMER2_UNIT);
//}

// 保存 交流周期定时器 当前定时时间值 单位：us
UINT16 u16GetT2Val(void)
{
	UINT16 i;
    //load counter initial value for timer2 overflow interruption；
	i = (UINT16)TIM16->CNT;
	if(i < TIMER2_VALUE)
	{//产生中断
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
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//拉高
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
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//拉高
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
void EXTI2_3_IRQHandler()//下降沿中断
{
	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		/* Clear the EXTI line 3 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line3);
		
		// 开始移相控制角定时，在过零点触发的外部中断服务程序中调用
		vEnablePhaseAngleTiming();			
		//中断去抖处理
		if(g_u8ExtISRFlt < INT_FLT_TM)
		{
			return;
		}
		//去抖滤波时间初始化
		g_u8ExtISRFlt = 0;
		//下降沿
		g_u8AcZeroFail=TRUE;
		// BC线交流周期获取
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
void EXTI4_15_IRQHandler()//上升沿触发
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line4);
//			// 开始移相控制角定时，在过零点触发的外部中断服务程序中调用
//			vEnablePhaseAngleTiming();
		if(g_u8AcZeroFail==TRUE)
		{
			g_u8AcZeroFail=FALSE;
			//过零点
			vGetAcCyc(SAMPLE_ZERO);
		}
	}
}

// 更新过零点产生的外部中断去抖滤波计时变量   在 定时器 溢出定时中断中调用
void vUpdateExtISRFltTm(void)
{
	// 过零点去抖滤波时间计数
	if(g_u8ExtISRFlt < INT_FLT_TM)
	{
		g_u8ExtISRFlt ++;
	}
}
