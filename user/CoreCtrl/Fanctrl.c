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

//************************������ģ��ӿ�ȫ�ֱ���************************
	/* ��������ѹ����λΪ�����ѹ�İٷֱ�, �Ŵ�100�� ��Χ0, 3000��10000 */
	UINT16 g_u16FanPwrVout100 = 0;
	/* �������ɿع�������ƽ�(��������ѹ�Ĺ����)���Ŵ�10������λΪ��, ��Χ 0��1500 */
	UINT16 g_u16FanScrAlfa = SCR_OFF_ANGLE;

	// ���ѹ��ֵ���������澯ʱʹ��8888
	INT16 g_i16MaxPressureNow;

//************************��ģ��ȫ�ֱ���************************

	// 0�ȡ�140�ȶ�Ӧ��ֵ
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
		3447,3356,3266,3177,3082,2995,2908,2817,2728,2640, //130~139//��С���26.4
		0
	};

//************************Public function declaration******************************
	//����ѹ������Ʋ���
	void CalculatePressurePoint(void);
	//��ȡ���ѹ��
	void GetMaxPressureVal(void);
	//������������ǰ���ѹ����Ӧ�������ѹ
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
	//�ɿع������ʼ��	
	PaSCR_Init();
	
	//��ʱ�жϳ�ʼ��
	InitT0(0,DISABLE);
	InitT1(0,DISABLE);
	InitT2();

	//�ⲿ�жϳ�ʼ��
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
	//����ѹ�����Ƶ����
	CalculatePressurePoint();
	//��ȡ����ѹ��ֵ
	GetMaxPressureVal();

	//���㵱ǰѹ����Ӧ�������ѹ
	CalculateVout();

	//�������������ѹ�ٷֱȼ���ն��������ƽ�
	CalculateAngleByVout();

	//����������ƽǼ���ײ���������
	vCalPhaseCuttingTime(g_u16FanScrAlfa);
}
//LEDָʾ���״̬
void FanCtrlStatus(void)
{
	static uint8_t su8Flash[2]={0};
	
	if(g_sys.status.Fan.Fan_out>10)//�����,��˸5��
	{
			led_toggle(LED_RUN);		
	}			
	else//��˸1��
	{
		if(++su8Flash[0] >= 5)
		{
			su8Flash[0] = 0;
			led_toggle(LED_RUN);	
		}		
	}	
	return;

}

//��ȡ���ѹ��
void GetMaxPressureVal(void)
{
	//ѹ����������Чʱ�ĸ�ֵ
	#define P_INVALID_VAL 8888

//	//��ȡ��ǰ���ѹ��
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
	g_i16Analog[ANALOGID_P1]=g_sys.status.ain[AI_SENSOR1];//�Ŵ�100��
	if(g_u8PSensorOutRangeFlag[ANALOGID_P1] == FALSE)
	{
		g_i16MaxPressureNow = g_i16Analog[ANALOGID_P1];
	}
	else
	{
		g_i16MaxPressureNow = P_INVALID_VAL;
	}
}
//���ʹ�ܿ���
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
//�쳣���
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
		// ѹ���仯���ڷ�Χ 0.1 bar �Ŵ�100�� ��λ100*BAR
	    #define MAXDELTAP1 10
	    #define MAXDELTAP2 50

	/* ѹ���仯�Ĳο�ֵ ��λbar �Ŵ�100�� */
	static INT16 s_i16PDeltaRef;

	// ������ѹ���𵴵���ƽ��ֵ
	static INT16 s_i16PRefAverage = 0;
	// ������ѹ�����ڴ���
	static UINT8 s_u8AdjTimes = 0;
 	static UINT8 s_u8StartPress = 0;
 	static UINT8 s_u8CFGComErr = 0;
	UINT16 u16Step=50;
	UINT16 u16FanPwrVout;
	UINT16 BUFF=0;

	// ͨ�ſ���
	if(g_sys.config.g_u8CfgParameter[CFGCOMMODE])
	{
		s_u8CFGComErr = 0x01;
		g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGCOMFANOUT]*100;		
	}
// ѹ������������ ���� Ƶ���쳣ʱ ��� 100��
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
			NVIC_SystemReset();	//��λ
		}
		// ѹ������������ʱ�����������ڲ�����ʼ��
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
		
		if(FanStartSwitch()==FALSE)//����DIδʹ��-�ر�
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
			//����ѹ��������С�趨ֵ��һ
			else if(g_i16MaxPressureNow < g_sys.status.Fan.u16PressuePoint[P_OFF_POINT])
			{
							BUFF|=0x08;
				g_u16FanPwrVout100 = 0;
			}
			else if(g_i16MaxPressureNow < g_sys.status.Fan.u16PressuePoint[P_SET_POINT])
			{	
				//����ѹ�� С��ѹ���趨���Ӧֵ �� �����趨ֵ��һ
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
			{//������������Χ
					BUFF|=0x40;
				//ѹ���仯���� �趨ֵ �ſ�ʼִ�� �������
				if(ABS(s_i16PDeltaRef - g_i16MaxPressureNow) > MAXDELTAP1)
				{
									BUFF|=0x80;
					// ��ǰѹ������ƽ��ѹ���趨��Χ
					if(ABS(g_i16MaxPressureNow - s_i16PRefAverage) > MAXDELTAP2)
					{//ѹ��ƽ��ֵ���� �ȶ���Χֵ���ȶ���Χѹ���ο�ֵ ˢ�£��ȶ����̵��ڴ�������
						s_i16PRefAverage = g_i16MaxPressureNow;
										BUFF|=0x100;
						if(s_i16PRefAverage > (g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] - MAXDELTAP2))
						{//�ȶ���Χ�ο�ѹ�����ֵΪ����ֵ���ȶ���Χѹ��
							s_i16PRefAverage = g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT] - MAXDELTAP2;
						}

						s_u8AdjTimes = 1;
					}

					if(s_u8AdjTimes <= P_ADJ_TIMES)
					{
										BUFF|=0x200;
						// ѹ���ȶ���Χ�ڵ��ڴ��������趨ֵ����û��ƽ���ֹͣ���ڣ�ֱ������ѹ���ȶ���Χ���ٴε���
						s_u8AdjTimes++;
						//ѹ���仯ƽ��ֵ����
						s_i16PRefAverage = (s_i16PRefAverage + g_i16MaxPressureNow) / 2;

						//������������ǰ���ѹ����Ӧ�������ѹ
						//��������ѹ��Ϊб���ϵ�ǰ���ѹ����Ӧ�����ѹ��
						g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGMINVID] +
								((g_i16MaxPressureNow - g_sys.status.Fan.u16PressuePoint[P_SET_POINT]) *
								((g_sys.config.g_u8CfgParameter[CFGMAXVID] - g_sys.config.g_u8CfgParameter[CFGMINVID]) / g_sys.status.Fan.u16PressuePoint[P_STEP_BAND]));
					}
					//ˢ��ѹ���仯�ο�ֵ
					s_i16PDeltaRef = g_i16MaxPressureNow;
					BUFF|=0x400;
				}
			}
			else if(g_i16MaxPressureNow <= g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT])
			{	//���ѹ������100����ѹ����ز�ʱ����������ѹΪ100������100����Ϊ����趨��ѹ���
				g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGMAXVID];
				BUFF|=0x800;
			}
			else if(g_i16MaxPressureNow <= g_sys.status.Fan.u16PressuePoint[P_STEP_POINT] )
			{		//���ѹ��λ�ڻز��� �������ѹ�趨Ϊ100�� ���� ��� �趨���
				BUFF|=0x1000;
				if(g_u16FanPwrVout100 < g_sys.config.g_u8CfgParameter[CFGMAXVID])
				{
					BUFF|=0x2000;
					g_u16FanPwrVout100 = g_sys.config.g_u8CfgParameter[CFGMAXVID];
				}
			}
			else
			{	//���ѹ�����ڽ�Ծѹ���� �������ѹ�趨Ϊ100���Ľ�������
				BUFF|=0x4000;
				g_u16FanPwrVout100 = 10000;
			}			
		}
//		//TEST
//		g_u16FanPwrVout100 = 50*100;		
	}
	

	//����������
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

//����ģʽѡ��
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
//R22,R407C,R410Aȱʡ
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
				//���ò���û�б仯ʱ���ü����趨��
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

// ��������ٷֱȼ���ն���Ƕ� ����ٷֱȷŴ�100�� ն���ǶȷŴ� 10��
void CalculateAngleByVout(void)
{
//	UINT16 i = 0;
//	do
//	{
//		// ��ֵ�����������ѹ��Ӧ������ƽ�
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
		// ��ֵ�����������ѹ��Ӧ������ƽ�
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


