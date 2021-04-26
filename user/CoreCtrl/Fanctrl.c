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
#include <stdlib.h>
#include <math.h>
#include "daq.h"
#include "dio.h"
#include "pid.h"
#include "global_var.h"
#include "sys_conf.h"
#include "fanctrl.h"

#include "PhaseCutting.h"
#include "Sample.h"
//#include "fsc1plib.h"

#include "alarms.h"
#include "led.h"


//#include "fsc1peeprom.h"

//************************Public global variable definition************************

//************************与其他模块接口全局变量************************
	/* 风机供电电压，单位为输入电压的百分比, 放大100倍 范围0, 3000～10000 */
	UINT16 g_u16FanPwrVout100 = 0;
	/* 风机供电可控硅移相控制角(相对于相电压的过零点)，放大10倍，单位为度, 范围 0～1500 */
	UINT16 g_u16FanScrAlfa = SCR_OFF_ANGLE;

	// 最大压力值，传感器告警时使用8888
	INT16 g_i16MaxPressureNow;

//************************本模块全局变量************************

	// 0度～140度对应数值
	const UINT16 g_u16VoutByAngleLib[] =
	{
		10000,9961,9961,9961,9961,9961,9961,9961,9961,9961, //0~9
		9954,9954,9954,9954,9954,9954,9935,9935,9935,9935, //10~19
		9913,9913,9913,9888,9888,9865,9850,9838,9825,9811, //20~29
		9799,9785,9770,9752,9735,9718,9699,9682,9662,9640, //30~39
		9622,9599,9575,9549,9525,9495,9470,9440,9410,9381, //40~49
		9350,9322,9290,9259,9219,9180,9136,9099,9054,9009, //50~59
		8963,8918,8868,8818,8768,8717,8661,8609,8550,8484, //60~69
		8424,8363,8304,8238,8172,8104,8040,7979,7912,7840, //70~79
		7768,7693,7620,7550,7477,7406,7329,7254,7176,7095, //80~89
		7021,6940,6859,6776,6692,6614,6526,6444,6363,6272, //90~99
		6186,6100,6013,5931,5840,5750,5659,5568,5477,5386, //100~109
		5295,5204,5113,5022,4931,4840,4750,4653,4562,4465, //110~119
		4375,4284,4190,4095,4000,3904,3818,3726,3635,3544, //120~129
		3447,3356,3266,3177,3082,2995,2908,2817,2728,2640, //130~139//最小输出26.4
		0
	};

//************************Public function declaration******************************
	//计算压力点控制参数
	void CalculatePressurePoint(void);
	//获取最大压力
	void GetMaxPressureVal(void);
	//计算线性区当前最大压力对应的输出电压
	void CalculateVout(void);
/*=============================================================================*
 * FUNCTION: InitFanCtrl()
 * PURPOSE : Initiate Fan Control module
 * INPUT:
 *     NONE
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *
 *
 * CALLED BY:
 *
 *
 *============================================================================*/
void InitFanCtrl(void)
{
	//可控硅输出初始化	
	PaSCR_Init();
	
	//定时中断初始化
	InitT0(0,DISABLE);
	InitT1(0,DISABLE);
	InitT2();

	//外部中断初始化
	InitEXINT1();
	InitEXINT2();
	return;
}


/*=============================================================================*
 * FUNCTION: FanCtrlAlgorithm()
 * PURPOSE : FAN AC supply voltage rectification algorithm:Calculate percentage of Vo/Vi and related phase angle
 * INPUT:
 *     	g_u8CompressorWorkStat
 *		g_sys.status.Fan.u16PressuePoint[]
 *		g_u8AlmStatus[ACFREQUENCYALM]
 *
 * RETURN:
 *     	NONE
 *
 * CALLS:
 *
 *
 * CALLED BY:
 *     	main()
 *
 *============================================================================*/
void FanCtrlAlgorithm(void)
{
	//计算压力控制点参数
	CalculatePressurePoint();
	//获取控制压力值
	GetMaxPressureVal();

	//计算当前压力对应的输出电压
	CalculateVout();

	//根据输入输出电压百分比计算斩波移相控制角
	CalculateAngleByVout();

	//根据移相控制角计算底层驱动参数
	vCalPhaseCuttingTime(g_u16FanScrAlfa);
}
//LED指示风机状态
void FanCtrlStatus(void)
{
	static uint8_t su8Flash[2]={0};
	
	if(g_sys.status.Fan.Fan_out>10)//有输出,闪烁5次
	{
			led_toggle(LED_RUN);		
	}			
	else//闪烁1次
	{
		if(++su8Flash[0] >= 5)
		{
			su8Flash[0] = 0;
			led_toggle(LED_RUN);	
		}		
	}	
	return;

}

//获取最大压力
void GetMaxPressureVal(void)
{
	//压力传感器无效时的赋值
	#define P_INVALID_VAL 8888

//	//获取当前最大压力
//	if((g_u8PSensorOutRangeFlag[ANALOGID_P1] == FALSE) &&
//		(g_u8PSensorOutRangeFlag[ANALOGID_P2] == FALSE))
//	{
//		g_i16MaxPressureNow = MAX(g_i16Analog[ANALOGID_P1], g_i16Analog[ANALOGID_P2]);
//	}
//	else if(g_u8PSensorOutRangeFlag[ANALOGID_P1] == FALSE)
//	{
//		g_i16MaxPressureNow = g_i16Analog[ANALOGID_P1];
//	}
//	else if(g_u8PSensorOutRangeFlag[ANALOGID_P2] == FALSE)
//	{
//		g_i16MaxPressureNow = g_i16Analog[ANALOGID_P2];
//	}
//	else
//	{
//		g_i16MaxPressureNow = P_INVALID_VAL;
//	}
	g_i16Analog[ANALOGID_P1]=g_sys.status.ain[AI_SENSOR1];//放大100倍
	if(g_u8PSensorOutRangeFlag[ANALOGID_P1] == FALSE)
	{
		g_i16MaxPressureNow = g_i16Analog[ANALOGID_P1];
	}
	else
	{
		g_i16MaxPressureNow = P_INVALID_VAL;
	}
}
//风机使能控制
UINT8 FanStartSwitch(void)
{
	UINT16 DI_In=g_sys.status.din_bitmap[1];
	if(DI_In&StartEnable)
	{
		g_sys.status.Test_Buff[21]=DI_In;			
		if((DI_In&FanOn)==0)
		{
				return FALSE;				
		}
		else
		{
				return TRUE;
		}
	}
	return TRUE;
}
//异常输出
UINT8 FanOutError(void)
{
	if(get_alarm_bitmap(ACL_ACFREQUENCYALM) || get_alarm_bitmap(ACL_PSENSORALM)|| get_alarm_bitmap(ACL_COMMON))
	{
		return TRUE;
	}
	return FALSE;
}


void CalculateVout(void)
{
		extern sys_reg_st					g_sys; 				
		#define P_ADJ_TIMES 		250
		#define P_ADJ_ST    		30
		#define P_MIN_Pressure   20
		// 压力变化调节范围 0.1 bar 放大100倍 单位100*BAR
	    #define MAXDELTAP1 10
	    #define MAXDELTAP2 50

	/* 压力变化的参考值 单位bar 放大100倍 */
	static INT16 s_i16PDeltaRef;

	// 线性区压力震荡调节平均值
	static INT16 s_i16PRefAverage = 0;
	// 线性区压力调节次数
	static UINT8 s_u8AdjTimes = 0;
 	static UINT8 s_u8StartPress = 0;
 	static UINT8 s_u8CFGComErr = 0;
	UINT16 u16Step=50;
	UINT16 u16FanPwrVout;
	UINT16 BUFF=0;

	// 通信控制
	if(g_sys.config.g_u8CfgParameter[CFGCOMMODE])
	{
		s_u8CFGComErr = 0x01;
		g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGCOMFANOUT]*100;		
	}
// 压力传感器故障 或者 频率异常时 输出 100％
//		if((g_u8AlmStatus[PSENSORALM] == ALARM) || (g_u8AlmStatus[ACFREQUENCYALM] == ALARM))
	else if(FanOutError()==TRUE)	
	{
		s_u8CFGComErr = 0x02;
		g_u16FanPwrVout100 = 10000;
			BUFF|=0x8000;
	}
	else
	{
		if(s_u8CFGComErr)
		{
			s_u8CFGComErr = 0;
			NVIC_SystemReset();	//复位
		}
		// 压力不在线性区时，线性区调节参数初始化
		if(g_i16MaxPressureNow < g_sys.status.Fan.u16PressuePoint[P_SET_POINT] ||
			g_i16MaxPressureNow >= g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT])
		{
			BUFF|=0x01;
			s_i16PDeltaRef = 0;
			s_i16PRefAverage = 0;
			s_u8AdjTimes = 0;
		}

		g_sys.status.Test_Buff[15]=g_i16MaxPressureNow;
		g_sys.status.Test_Buff[16]=g_sys.status.Fan.u16PressuePoint[P_OFF_POINT];
		g_sys.status.Test_Buff[17]=g_sys.status.Fan.u16PressuePoint[P_SET_POINT];
		g_sys.status.Test_Buff[18]=g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT];
		g_sys.status.Test_Buff[19]=g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT];
		
		g_sys.status.Test_Buff[26]=g_u8AlmStatus[ACFREQUENCYALM];
		g_sys.status.Test_Buff[28]=g_u8AlmStatus[PSENSORALM];
		
		s_u8StartPress++;
		if(s_u8StartPress>=50)
		{
			s_u8StartPress=50;
		}	
		
		if(FanStartSwitch()==FALSE)//开关DI未使能-关闭
		{
			BUFF|=0x02;
			g_u16FanPwrVout100 = 0;		
		}
		else
		{
			if((g_i16MaxPressureNow < P_MIN_Pressure)&&(s_u8StartPress>=50))
			{
							BUFF|=0x04;
				g_u16FanPwrVout100 = 10000;
			}		
			//冷凝压力低于最小设定值减一
			else if(g_i16MaxPressureNow < g_sys.status.Fan.u16PressuePoint[P_OFF_POINT])
			{
							BUFF|=0x08;
				g_u16FanPwrVout100 = 0;
			}
			else if(g_i16MaxPressureNow < g_sys.status.Fan.u16PressuePoint[P_SET_POINT])
			{	
				//冷凝压力 小于压力设定点对应值 且 大于设定值减一
				if(g_u16FanPwrVout100 >= g_sys.config.g_u8CfgParameter[CFGMINVID])
				{
					BUFF|=0x10;
					g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGMINVID];
				}
				else
				{
					BUFF|=0x20;
					g_u16FanPwrVout100 = 0;
				}
			}
			else if(g_i16MaxPressureNow < g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT])
			{//进入线性区范围
					BUFF|=0x40;
				//压力变化超过 设定值 才开始执行 输出调节
				if(ABS(s_i16PDeltaRef - g_i16MaxPressureNow) > MAXDELTAP1)
				{
									BUFF|=0x80;
					// 当前压力超过平均压力设定范围
					if(ABS(g_i16MaxPressureNow - s_i16PRefAverage) > MAXDELTAP2)
					{//压力平均值超过 稳定范围值，稳定范围压力参考值 刷新，稳定过程调节次数清零
						s_i16PRefAverage = g_i16MaxPressureNow;
										BUFF|=0x100;
						if(s_i16PRefAverage > (g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] - MAXDELTAP2))
						{//稳定范围参考压力最大值为上限值减稳定范围压力
							s_i16PRefAverage = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] - MAXDELTAP2;
						}

						s_u8AdjTimes = 1;
					}

					if(s_u8AdjTimes <= P_ADJ_TIMES)
					{
										BUFF|=0x200;
						// 压力稳定范围内调节次数大于设定值，还没有平衡就停止调节，直到超过压力稳定范围后再次调节
						s_u8AdjTimes++;
						//压力变化平均值计算
						s_i16PRefAverage = (s_i16PRefAverage + g_i16MaxPressureNow) / 2;

						//计算线性区当前最大压力对应的输出电压
						//风机供电电压设为斜线上当前最大压力对应输出电压点
						g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGMINVID] +
								((g_i16MaxPressureNow - g_sys.status.Fan.u16PressuePoint[P_SET_POINT]) *
								((g_sys.config.g_u8CfgParameter[CFGMAXVID] - g_sys.config.g_u8CfgParameter[CFGMINVID]) / g_sys.status.Fan.u16PressuePoint[P_STEP_BAND]));
					}
					//刷新压力变化参考值
					s_i16PDeltaRef = g_i16MaxPressureNow;
					BUFF|=0x400;
				}
			}
			else if(g_i16MaxPressureNow <= g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT])
			{	//最高压力低于100％电压输出回差时，如果输出电压为100％，将100％变为最高设定电压输出
				g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGMAXVID];
				BUFF|=0x800;
			}
			else if(g_i16MaxPressureNow <= g_sys.status.Fan.u16PressuePoint[P_STEP_POINT] )
			{		//最高压力位于回差区 ，输出电压设定为100％ 或者 最大 设定输出
				BUFF|=0x1000;
				if(g_u16FanPwrVout100 < g_sys.config.g_u8CfgParameter[CFGMAXVID])
				{
					BUFF|=0x2000;
					g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGMAXVID];
				}
			}
			else
			{	//最高压力大于阶跃压力点 ，输出电压设定为100％的交流输入
				BUFF|=0x4000;
				g_u16FanPwrVout100 = 10000;
			}			
		}
//		//TEST
//		g_u16FanPwrVout100 = 50*100;		
	}
	

	//风机输出步长
	u16FanPwrVout=g_sys.status.Fan.u16FanVout100;
	if(abs(g_u16FanPwrVout100 - u16FanPwrVout) > u16Step)
	{
			if(g_u16FanPwrVout100 > u16FanPwrVout)
			{
					 u16FanPwrVout += u16Step;
			}
			else
			{
						u16FanPwrVout -= u16Step;
			}												
	}
	else
	{
			u16FanPwrVout = g_u16FanPwrVout100;
	}	
//	g_sys.status.Fan.u16FanVout100=	u16FanPwrVout;
	g_sys.status.Fan.u16FanVout100=	g_u16FanPwrVout100;
	g_sys.status.Fan.Fan_out=g_u16FanPwrVout100/100;	
	g_sys.status.Test_Buff[20]=BUFF;	
}

//参数模式选择
UINT8 PraMode(void)
{
	UINT16 DI_In=g_sys.status.din_bitmap[1];
	UINT8 DI_Mode=0;
	if((DI_In&Para_M)==0x00)
	{
		DI_Mode=0;
	}
	else
	if((DI_In&Para_M)==0x01)
	{
		DI_Mode=1;
	}
	else
	if((DI_In&Para_M)==0x02)
	{
		DI_Mode=2;
	}
	else
	if((DI_In&Para_M)==0x03)
	{
		DI_Mode=3;
	}
	return DI_Mode;
}
//R22,R407C,R410A缺省
const UINT16 g_u16CfgDefault[Ref_MAX][2] = 
{
	{1300,500},	//R22
	{1350,450},	//R407C
	{2300,500},	//R410A
};
/*=============================================================================*
 * FUNCTION: CalculatePressurePoint()
 * PURPOSE : Calculate Pressure Control Point Value which is amplified 100 times
 * INPUT:
 *     	g_u8CfgParameter[CFGPSETID]
 *		g_u8CfgParameter[CFGPBANDID]
 * OUTPUT:
 *		g_sys.status.Fan.u16PressuePoint[]
 * RETURN:
 *     	NONE
 *
 * CALLS:
 *     	NONE
 *
 * CALLED BY:
 *     	FanCtrlAlgorithm()
 *
 *============================================================================*/
void CalculatePressurePoint(void)
{
	static UINT8 s_u8Pset = 0, s_u8Pband = 0;
	UINT8 Para_Mode=0;
	
	Para_Mode=PraMode();
	g_sys.status.work_mode=Para_Mode;
	switch(Para_Mode)
	{
		case 0x00:
			{
				g_sys.status.Fan.u16PressuePoint[P_SET_POINT] = g_u16CfgDefault[Ref_R22][0];
				g_sys.status.Fan.u16PressuePoint[P_STEP_BAND] = g_u16CfgDefault[Ref_R22][1];
				g_sys.status.Fan.u16PressuePoint[P_OFF_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] - 100;
				g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] + g_sys.status.Fan.u16PressuePoint[P_STEP_BAND];
				g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 300;
				g_sys.status.Fan.u16PressuePoint[P_STEP_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 500;		
				break;			
			}
		case 0x01:
			{
				g_sys.status.Fan.u16PressuePoint[P_SET_POINT] = g_u16CfgDefault[Ref_R407C][0];
				g_sys.status.Fan.u16PressuePoint[P_STEP_BAND] = g_u16CfgDefault[Ref_R407C][1];
				g_sys.status.Fan.u16PressuePoint[P_OFF_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] - 100;
				g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] + g_sys.status.Fan.u16PressuePoint[P_STEP_BAND];
				g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 300;
				g_sys.status.Fan.u16PressuePoint[P_STEP_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 500;		
				break;			
			}
		case 0x02:
			{
				g_sys.status.Fan.u16PressuePoint[P_SET_POINT] = g_u16CfgDefault[Ref_R410A][0];
				g_sys.status.Fan.u16PressuePoint[P_STEP_BAND] = g_u16CfgDefault[Ref_R410A][1];
				g_sys.status.Fan.u16PressuePoint[P_OFF_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] - 100;
				g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] + g_sys.status.Fan.u16PressuePoint[P_STEP_BAND];
				g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 300;
				g_sys.status.Fan.u16PressuePoint[P_STEP_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 500;					
				break;			
			}
		case 0x03:
			{
				//设置参数没有变化时不用计算设定点
				if( (s_u8Pset != g_sys.config.g_u8CfgParameter[CFGPSETID]) ||
					(s_u8Pband != g_sys.config.g_u8CfgParameter[CFGPBANDID]) )
				{
					s_u8Pset = g_sys.config.g_u8CfgParameter[CFGPSETID];
					s_u8Pband = g_sys.config.g_u8CfgParameter[CFGPBANDID];

					g_sys.status.Fan.u16PressuePoint[P_SET_POINT] = g_sys.config.g_u8CfgParameter[CFGPSETID];
					g_sys.status.Fan.u16PressuePoint[P_STEP_BAND] = g_sys.config.g_u8CfgParameter[CFGPBANDID];					
					g_sys.status.Fan.u16PressuePoint[P_OFF_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] - 100;
					g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] + g_sys.status.Fan.u16PressuePoint[P_STEP_BAND];
					g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 300;
					g_sys.status.Fan.u16PressuePoint[P_STEP_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 500;
				}
				break;			
			}
		default:
			{
				g_sys.status.Fan.u16PressuePoint[P_SET_POINT] = g_u16CfgDefault[Ref_R22][0];
				g_sys.status.Fan.u16PressuePoint[P_STEP_BAND] = g_u16CfgDefault[Ref_R22][1];
				g_sys.status.Fan.u16PressuePoint[P_OFF_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] - 100;
				g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] = g_sys.status.Fan.u16PressuePoint[P_SET_POINT] + g_sys.status.Fan.u16PressuePoint[P_STEP_BAND];
				g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 300;
				g_sys.status.Fan.u16PressuePoint[P_STEP_POINT] = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] + 500;		
				break;			
			}		
	}
	return;
}

// 根据输出百分比计算斩波角度 输出百分比放大100倍 斩波角度放大 10倍
void CalculateAngleByVout(void)
{
//	UINT16 i = 0;
//	do
//	{
//		// 差值法计算输出电压对应移相控制角
//		if(g_u16FanPwrVout100 < g_u16VoutByAngleLib[i])g_sys.status.Fan.u16FanVout100
//		{
//			i++;
//		}
//		else
//		{
//			if(i)
//			{
//				g_u16FanScrAlfa = i * 10 - (g_u16FanPwrVout100 - g_u16VoutByAngleLib[i]) * 10 / (g_u16VoutByAngleLib[i - 1] - g_u16VoutByAngleLib[i]);
//			}
//			else
//			{
//				g_u16FanScrAlfa = 0;
//			}
//			return;
//		}
//	}while(g_u16VoutByAngleLib[i]);

//	g_u16FanScrAlfa = SCR_OFF_ANGLE;
	UINT16 i = 0;
	do
	{
		// 差值法计算输出电压对应移相控制角
		if(g_sys.status.Fan.u16FanVout100 < g_u16VoutByAngleLib[i])
		{
			i++;
		}
		else
		{
			if(i)
			{
				g_u16FanScrAlfa = i * 10 - (g_sys.status.Fan.u16FanVout100 - g_u16VoutByAngleLib[i]) * 10 / (g_u16VoutByAngleLib[i - 1] - g_u16VoutByAngleLib[i]);
			}
			else
			{
				g_u16FanScrAlfa = 0;
			}
			return;
		}
	}while(g_u16VoutByAngleLib[i]);

	g_u16FanScrAlfa = SCR_OFF_ANGLE;
}


