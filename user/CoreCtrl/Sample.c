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
#include "Sample.h"
#include "Lib.h"

//************************Public global variable definition************************

//************************与其他模块接口全局变量************************
// Analog value array 压力放大100倍
INT16 g_i16Analog[ANALOGMAXNUM];

//压力传感器超范围标志
UINT8 g_u8PSensorOutRangeFlag[ANALOGMAXNUM];

//************************本模块全局变量************************
/* 每个模拟量连续采集次数变量 */
UINT8 g_u8ADCSampleTimes = 0;
/* Continuous ADC sample times for every analog */
#define ADCMAXTIMES 8
/* ADC转换值暂存变量*/
UINT16 g_u16ADCTemp[ADCMAXTIMES];

typedef struct
{  //温度传感器表格数据结构
    INT16 i16Temprature;
    UINT16 u16Resistance;
} Struct_TempratureTable;

#define NTC_TABLE_LEN 155  //温度表格长度

const Struct_TempratureTable tagNTCTable[NTC_TABLE_LEN] = {  //温度放大10倍，电阻值放大100倍
    {-490, 62800}, {-480, 59300}, {-470, 56000}, {-460, 52600}, {-450, 49200}, {-440, 46000}, {-430, 42900},
    {-420, 40100}, {-410, 37900},  //通过拟合得到了这10组数据
    {-400, 35403}, {-390, 33076}, {-380, 30920}, {-370, 28922}, {-360, 27069}, {-350, 25348}, {-340, 23750},
    {-330, 22264}, {-320, 20882}, {-310, 19595}, {-300, 18397}, {-290, 17280}, {-280, 16238}, {-270, 15266},
    {-260, 14358}, {-250, 13510}, {-240, 12717}, {-230, 11976}, {-220, 11282}, {-210, 10632}, {-200, 10024},
    {-190, 9454},  {-180, 8919},  {-170, 8417},  {-160, 7947},  {-150, 7505},  {-140, 7090},  {-130, 6700},
    {-120, 6333},  {-110, 5988},  {-100, 5664},  {-90, 5358},   {-80, 5071},   {-70, 4801},   {-60, 4547},
    {-50, 4380},   {-40, 4084},   {-30, 3872},   {-20, 3673},   {-10, 3485},   {00, 3308},    {10, 3142},
    {20, 2984},    {30, 2836},    {40, 2696},    {50, 2563},    {60, 2438},    {70, 2320},    {80, 2208},
    {90, 2103},    {100, 2003},   {110, 1908},   {120, 1819},   {130, 1734},   {140, 1654},   {150, 1578},
    {160, 1505},   {170, 1437},   {180, 1372},   {190, 1310},   {200, 1252},   {210, 1196},   {220, 1143},
    {230, 1093},   {240, 1046},   {250, 1000},   {260, 957},    {270, 916},    {280, 877},    {290, 839},
    {300, 804},    {310, 770},    {320, 738},    {330, 707},    {340, 678},    {350, 650},    {360, 624},
    {370, 598},    {380, 574},    {390, 551},    {400, 529},    {410, 508},    {420, 488},    {430, 469},
    {440, 451},    {450, 433},    {460, 417},    {470, 401},    {480, 385},    {490, 371},    {500, 357},
    {510, 344},    {520, 331},    {530, 319},    {540, 307},    {550, 296},    {560, 285},    {570, 275},
    {580, 265},    {590, 256},    {600, 247},    {610, 238},    {620, 230},    {630, 222},    {640, 214},
    {650, 207},    {660, 200},    {670, 193},    {680, 186},    {690, 180},    {700, 174},    {710, 168},
    {720, 163},    {730, 158},    {740, 152},    {750, 147},    {760, 143},    {770, 138},    {780, 134},
    {790, 130},    {800, 125},    {810, 122},    {820, 118},    {830, 114},    {840, 111},    {850, 107},
    {860, 104},    {870, 101},    {880, 98},     {890, 95},     {900, 92},     {910, 89},     {920, 86},
    {930, 84},     {940, 81},     {950, 79},     {960, 77},     {970, 75},     {980, 72},     {990, 70},
    {1000, 68},    {1010, 66},    {1020, 64},    {1030, 63},    {1040, 61},    {1050, 59}};

// 参考电压：4.656V 放大1000倍
#define VREF1000 4656

//************************Public function declaration******************************
// 计算AD转换结果 保存到g_u16ADCTemp[0]
void CalculateADCResult(void);
// 根据AD结果计算实际物理值
void CalculatePhysicalValue(UINT8 u8AnalogID);
// 启动一次ADC转换
void StartADConvert(void);
//计算温度值
void CalculateTemperature(UINT16 u16TempSampleV, UCHAR u8TempIDIndex);

///*=============================================================================*
// * FUNCTION: SIGNAL(ADC_vect)
// * PURPOSE : ADC interruption; read result to temporary array.
// * INPUT:
// *     g_u8ADCSampleTimes
// *
// * RETURN:
// *     g_u16ADCTemp[]
// *
// * CALLS:
// *     NONE;
// *
// * CALLED BY:
// *     NONE;
// *
// *============================================================================*/
// SIGNAL(ADC_vect)
//{
//	//Save sample result
//	g_u16ADCTemp[g_u8ADCSampleTimes] = ADC & 0x3FF;
//	//Refresh continous sample times
//	g_u8ADCSampleTimes++;
//}

///*=============================================================================*
// * FUNCTION: InitAnalog()
// * PURPOSE : Initiate Atmega32 ADC module
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
// void InitAnalog( void )
//{
//	UINT8 i;

//    //多工选择寄存器－ADMUX
//    //    Bits 4:0 (MUX4:0)－ 模拟通道与增益选择位 初始化为选择单端输入 ADC0
//    //    Bit 5 (ADLAR)    － ADLAR置位时转换结果为左对齐，否则为右对齐 初始化为右对齐
//    //    Bit 7:6 (REFS1:0)－ 参考电压选择 初始化选择外部电压参考
//    //                 0 0 AREF，内部Vref 关闭
//    //                 0 1 AVCC，AREF 引脚外加滤波电容
//    //                 1 0 保留
//    //                 1 1 2.56V 的片内基准电压源， AREF 引脚外加滤波电容
//	ADMUX = 0x00;
//    //ADC 控制和状态寄存器A － ADCSRA
//    //    Bits 2:0 (ADPS2:0)－ ADC 预分频器选择位,确定XTAL与ADC输入时钟之间的分频因子
//    //                         0－2分频； 1－2分频； 2－4分频； 3－8分频
//    //                         4－16分频；5－32分频；6－64分频；7－128分频  使用32分频（提高转换精度）
//    //    Bit 3 (ADIE)      － ADC中断使能；若ADIE及SREG的位I置位， ADC转换结束中断即被使能
//    //    Bit 4 (ADIF)      － ADC 中断标志；在ADC转换结束，且数据寄存器被更新后，ADIF 置位。
//    //    Bit 5 (ADATE)     － ADC 自动触发使能；ADATE置位将启动ADC自动触发功能。
//    //    Bit 6 (ADSC)      － ADC 开始转换；在单次转换模式下，ADSC 置位将启动一次ADC 转换。
//    //                         在连续转换模式下，ADSC 置位将启动首次转换。
//    //    Bit 7 (ADEN)      － ADC 使能；ADEN置位即启动ADC，否则ADC功能关闭。
//	// ADC使能启动，写1到ADIF清除中断标志，开放AD中断
//	ADCSRA	= 0x9d;

//	for(i = 0; i < ANALOGMAXNUM; i++)
//	{
//		g_u8PSensorOutRangeFlag[i] = FALSE;
//	}
//}

/*=============================================================================*
 * FUNCTION: StartADConvert()
 * PURPOSE : Start AD convert module
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
 *     ;
 *
 *============================================================================*/
void StartADConvert(void)
{
    //    ADCSRA |= _BV(ADSC);				//启动AD转换
    return;
}

/*=============================================================================*
 * FUNCTION: AnalogDeal()
 * PURPOSE : Analog sample and deal with
 * INPUT:
 *		g_u8ADCSampleTimes
 *
 * OUTPUT:
 *		g_i16Analog[]
 *
 * RETURN:
 *     NONE
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     main();
 *
 *============================================================================*/
void AnalogDeal(void)
{
    /* 模拟量采样通道序号常量定义 */
    typedef enum _ADC_PORT_NUM_
    {
        ADC_P1,
        ADC_P2,
        ADC_NTC1,
        ADC_NTC2,

        ADC_PORT_NUM
    } ADC_PORT_NUMBER;
    /* 模拟量采样通道序号： 0～3 */
    static UINT8 s_u8AnalogPort = ADC_P1, s_u8AnalogID;

    /* 模拟量采样过程状态常量 */
    typedef enum _ADC_PROCESS_NUM_
    {
        ADC_PORT,  // 0	采样通道切换
        ADC_HOLD,  // 1	滤波保持
        ADC_EXE,   // 2	AD转换
        ADC_DEAL,  // 3	AD结果计算
        ADC_PROCESS_NUM
    } ADC_PROCESS_DEFINITION;
    /* 模拟量采样过程状态：0－采样通道切换 1－滤波保持；2－AD转换；3－AD结果计算 */
    static UINT8 s_u8ADCProcess = ADC_PORT;

    static UINT16 s_u16ADCHoldingTimer = 0;
/* AD转换通道切换时滤波保持时间，单位mS */
#define ADCHOLDTIME 10
    s_u8AnalogPort       = s_u8AnalogPort;
    s_u16ADCHoldingTimer = s_u16ADCHoldingTimer;
    // 根据采样过程状态进行相应处理
    switch (s_u8ADCProcess)
    {
            //		case ADC_PORT:	//模拟量采样 通道切换状态
            //			// Change the channel to the next one
            //			s_u8AnalogPort = (s_u8AnalogPort + 1) % ADC_PORT_NUM;
            //		    //Set the selected channel
            ////            ADMUX = s_u8AnalogPort;

            //			// 采样条件初始化: 采样次数、模拟量ID、通道保持滤波时间初始化
            //			g_u8ADCSampleTimes = 0;
            //			// 模拟量ID 与通道号相同
            //			s_u8AnalogID = s_u8AnalogPort;
            //			s_u16ADCHoldingTimer = GetCurrMs();
            //		break;

            //        case ADC_HOLD:	//模拟量采样 滤波保持状态
            //        	if(GetMsTimeGap(s_u16ADCHoldingTimer) < ADCHOLDTIME)
            //			{
            //				return;
            //			}

            //		break;

            //        case ADC_EXE:	//模拟量采样 AD转换状态
            //		{
            //		    if(g_u8ADCSampleTimes < ADCMAXTIMES)
            //			{
            //				StartADConvert();
            //				return;
            //			}
            //		}
            //		break;

        case ADC_DEAL:  //模拟量采样 AD结果计算状态
        {
            CalculateADCResult();

            CalculatePhysicalValue(s_u8AnalogID);
        }
        break;

        default:
            break;
    }

    // 采样过程状态刷新
    s_u8ADCProcess = (s_u8ADCProcess + 1) % ADC_PROCESS_NUM;
}

/*=============================================================================*
 * FUNCTION: CalculateADCResult()
 * PURPOSE : Calculate ADC sample result. Give up the first value and give up the
 *           max and min of other 7 values, calculate the mean value of the left samples
 * INPUT:
 *     	g_u16ADCTemp[]
 * OUTPUT:
 *		g_u16ADCTemp[0]
 * RETURN:
 *     	NONE
 *
 * CALLS:
 *     	NONE;
 *
 * CALLED BY:
 *     	AnalogDeal();
 *
 *============================================================================*/
void CalculateADCResult(void)
{
    UINT8 i;
    UINT8 l_u8MAXid, l_u8MINid, l_u8MAXvalue, l_u8MINvalue;

    // Throw away the first ADC value and also use it to save the ADC result
    g_u16ADCTemp[0] = 0;
    // Accumulate all other 7 ADC value and calculate the mean value
    for (i = 1; i < 8; i++)
    {
        g_u16ADCTemp[0] += g_u16ADCTemp[i];
    }
    g_u16ADCTemp[0] /= 7;

    // Find the index of max value and min value in array 找到最大和最小值索引
    l_u8MAXid = l_u8MINid = 1;
    l_u8MAXvalue = l_u8MINvalue = 0;
    for (i = 1; i < 8; i++)
    {
        if (g_u16ADCTemp[i] > g_u16ADCTemp[0])
        {
            if (g_u16ADCTemp[i] - g_u16ADCTemp[0] > l_u8MAXvalue)
            {
                l_u8MAXvalue = g_u16ADCTemp[i] - g_u16ADCTemp[0];
                l_u8MAXid    = i;
            }
        }
        else
        {
            if (g_u16ADCTemp[0] - g_u16ADCTemp[i] > l_u8MINvalue)
            {
                l_u8MINvalue = g_u16ADCTemp[0] - g_u16ADCTemp[i];
                l_u8MINid    = i;
            }
        }
    }

    // Calculate the mean value without the max and min one 去掉第一个和最大最小值后的平均值
    g_u16ADCTemp[0] = 0;
    for (i = 1; i < 8; i++)
    {
        if ((i != l_u8MAXid) && (i != l_u8MINid))
        {
            g_u16ADCTemp[0] += g_u16ADCTemp[i];
        }
    }
    if (l_u8MAXid != l_u8MINid)
    {
        g_u16ADCTemp[0] /= 5;
    }
    else
    {
        g_u16ADCTemp[0] /= 6;
    }
    return;
}

/*=============================================================================*
 * FUNCTION:  CalculatePhysicalValue()
 * PURPOSE :  Calculate the analog physical value
 *
 * INPUT:
 *     	u8AnalogID,
 *     	g_u16ADCTemp[0],
 * OUTPUT
 *		g_i16Analog[]
 *		g_u8EnvT1OutRangeFlag
 *		g_u8SCRT2OutRangeFlag
 *
 * RETURN:
 *     	NONE
 *
 * CALLS:
 *     	NONE
 *
 * CALLED BY:
 *     	AnalogDeal();
 *
 *============================================================================*/
void CalculatePhysicalValue(UINT8 u8AnalogID)
{
//电压型压力传感器参数 0.5V~4.5V 对应 0～34BAR 放大1000倍
#define PVTYPE_VL 500
#define PVTYPE_INVALD_VL 200
#define PVTYPE_VH 4500
#define PVTYPE_INVALD_VH 4800

#define PVTYPE_PL 0
#define PVTYPE_PH 3400
    UINT8 l_u8Tmp;
    UINT16 l_u16Temp;

#define P_DELTA_ERROR 5          //变化0.05bar
#define P_ERROR_FLT_INIT_TM 120  //压力滤波次数初始值
#define P_FLT_UP_TM 130          //压力滤波持续增加次数
#define P_FLT_DN_TM 110          //压力滤波持续降低次数
    static UINT8 s_u8PFltTimes[2] = {P_ERROR_FLT_INIT_TM, P_ERROR_FLT_INIT_TM};

    // Calculate the ADC port input voltage 放大100倍，单位V
    if (g_u16ADCTemp[0] >= 1023)
    {
        l_u16Temp = VREF1000;
    }
    else
    {
        l_u16Temp = (UINT16)((UINT32)g_u16ADCTemp[0] * VREF1000 / 1023);
    }

    switch (u8AnalogID)
    {
        case ANALOGID_P1:
        case ANALOGID_P2:
            //传感器初始化为无效
            g_u8PSensorOutRangeFlag[u8AnalogID] = TRUE;

            //电压型压力传感器计算
            if (l_u16Temp < PVTYPE_VL)
            {
                //测量压力的电压信号超过范围<=0.2V 时认为传感器无效
                if (l_u16Temp > PVTYPE_INVALD_VL)
                {  //测量压力的电压信号范围 > 0.2V 时认为传感器有效
                    g_u8PSensorOutRangeFlag[u8AnalogID] = FALSE;
                }
                // 电压信号小于下限时，压力值赋值为下限值 0 BAR
                l_u16Temp = PVTYPE_PL;
            }
            else if (l_u16Temp > PVTYPE_VH)
            {
                //测量压力的电压信号超过范围 >= 4.8V 时认为传感器无效
                if (l_u16Temp < PVTYPE_INVALD_VH)
                {  //测量压力的电压信号范围 < 4.8V 时认为传感器有效
                    g_u8PSensorOutRangeFlag[u8AnalogID] = FALSE;
                }
                // 电压信号大于上限时，压力值赋值为上限值 34 BAR
                l_u16Temp = PVTYPE_PH;
            }
            else
            {  // P = (Vp - Vl) * (Ph - Pl) / (Vh - Vl)
                g_u8PSensorOutRangeFlag[u8AnalogID] = FALSE;
                l_u16Temp =
                    (UINT16)((UINT32)(l_u16Temp - PVTYPE_VL) * (PVTYPE_PH - PVTYPE_PL) / (PVTYPE_VH - PVTYPE_VL));
            }

            l_u8Tmp = u8AnalogID - ANALOGID_P1;
            //压力滤波处理 跳变时需要滤波，否则平滑处理，跳变值 P_DELTA_ERROR
            if (ABS((INT16)((INT32)l_u16Temp - (INT32)g_i16Analog[u8AnalogID])) > P_DELTA_ERROR)
            {
                // 正向跳变 滤波
                if (l_u16Temp > g_i16Analog[u8AnalogID])
                {
                    if (s_u8PFltTimes[l_u8Tmp] < P_ERROR_FLT_INIT_TM)
                    {
                        s_u8PFltTimes[l_u8Tmp] = P_ERROR_FLT_INIT_TM;
                    }
                    else
                    {
                        s_u8PFltTimes[l_u8Tmp]++;
                    }
                }
                // 反向跳变 滤波
                else
                {
                    if (s_u8PFltTimes[l_u8Tmp] > P_ERROR_FLT_INIT_TM)
                    {
                        s_u8PFltTimes[l_u8Tmp] = P_ERROR_FLT_INIT_TM;
                    }
                    else
                    {
                        s_u8PFltTimes[l_u8Tmp]--;
                    }
                }

                if ((s_u8PFltTimes[l_u8Tmp] > P_FLT_UP_TM) || (s_u8PFltTimes[l_u8Tmp] < P_FLT_DN_TM))
                {
                    s_u8PFltTimes[l_u8Tmp]  = P_ERROR_FLT_INIT_TM;
                    g_i16Analog[u8AnalogID] = l_u16Temp;
                }
            }
            else  // 平滑变化
            {
                s_u8PFltTimes[l_u8Tmp]  = P_ERROR_FLT_INIT_TM;
                g_i16Analog[u8AnalogID] = l_u16Temp;
            }
            break;

        case ANALOGID_NTC1:
        case ANALOGID_NTC2:
            if (l_u16Temp == VREF1000)
            {
                g_u8PSensorOutRangeFlag[u8AnalogID] = TRUE;
                g_i16Analog[u8AnalogID]             = -500;
            }
            else
            {
                CalculateTemperature(l_u16Temp, u8AnalogID);
            }
            break;

        default:
            break;
    }
    return;
}

/*=============================================================================*
 * FUNCTION: CalculateTemperature()
 * PURPOSE : Calculate temperature value from sample voltage and temperature table
 * INPUT:
 *     	u16TempSampleV
 *		u8TempIDIndex
 *
 * RETURN:
 *     Temerature value
 *
 * CALLS:
 *     NONE;
 *
 * CALLED BY:
 *     main();
 *
 *============================================================================*/
void CalculateTemperature(UINT16 u16TempSampleV, UCHAR u8TempIDIndex)
{
    UINT8 i, l_u8Temp;
    INT16 l_i16Temp;
    UINT32 l_u32Temp;
    static UINT16 s_u16TempFltTimer[2];
#define T_DELTA_ERROR 10  //变化1度

    i         = i;
    l_u8Temp  = l_u8Temp;
    l_u32Temp = l_u32Temp;
    //由采样电压计算温度传感器电阻(单位为Kohm) (4.656  - V) / (10) = 4.656  / (10.1 + Res)
    //电阻值(单位为Kohm)放大100倍后强制为长整形 电压放大1000倍
    l_u32Temp = (UINT32)(VREF1000)*1000 / (VREF1000 - u16TempSampleV) - 1010;

    l_u8Temp = NTC_TABLE_LEN;

    g_u8PSensorOutRangeFlag[u8TempIDIndex] = TRUE;

    //    if(l_u32Temp >= pgm_read_word(&tagNTCTable[0].u16Resistance))
    //    {//超出温度下限
    //        l_i16Temp = pgm_read_word(&tagNTCTable[0].i16Temprature) - 1;
    //    }
    //    else if(l_u32Temp < pgm_read_word(&tagNTCTable[l_u8Temp - 1].u16Resistance))
    //    {//超出温度上限
    //        l_i16Temp = pgm_read_word(&tagNTCTable[l_u8Temp - 1].i16Temprature) + 1;
    //    }
    //    else
    //    {//正常
    //		for(i = 0; i < l_u8Temp; i++)
    // 	    {
    // 	       if(l_u32Temp >= pgm_read_word(&tagNTCTable[i].u16Resistance))
    //   	       {
    //    	        break;
    //           }
    //    	}
    //        l_i16Temp = pgm_read_word(&tagNTCTable[i].i16Temprature);
    //        //插值计算
    //        l_i16Temp = l_i16Temp - ((UINT16)l_u32Temp - pgm_read_word(&tagNTCTable[i].u16Resistance)) * 10
    //                      / (pgm_read_word(&tagNTCTable[i-1].u16Resistance) -
    //                      pgm_read_word(&tagNTCTable[i].u16Resistance));

    //		g_u8PSensorOutRangeFlag[u8TempIDIndex] = FALSE;
    //	}

    //温度滤波处理
    if (ABS(l_i16Temp - g_i16Analog[u8TempIDIndex]) >= T_DELTA_ERROR)
    {
        // 温度跳变1度持续500ms后认为有效
        if (GetMsTimeGap(s_u16TempFltTimer[u8TempIDIndex - ANALOGID_NTC1]) > 500)
        {
            g_i16Analog[u8TempIDIndex]                       = l_i16Temp;
            s_u16TempFltTimer[u8TempIDIndex - ANALOGID_NTC1] = GetCurrMs();
        }
    }
    else
    {
        g_i16Analog[u8TempIDIndex]                       = l_i16Temp;
        s_u16TempFltTimer[u8TempIDIndex - ANALOGID_NTC1] = GetCurrMs();
    }

    return;
}
