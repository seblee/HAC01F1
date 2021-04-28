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

//************************������ģ��ӿ�ȫ�ֱ���************************
// Analog value array ѹ���Ŵ�100��
INT16 g_i16Analog[ANALOGMAXNUM];

//ѹ������������Χ��־
UINT8 g_u8PSensorOutRangeFlag[ANALOGMAXNUM];

//************************��ģ��ȫ�ֱ���************************
/* ÿ��ģ���������ɼ��������� */
UINT8 g_u8ADCSampleTimes = 0;
/* Continuous ADC sample times for every analog */
#define ADCMAXTIMES 8
/* ADCת��ֵ�ݴ����*/
UINT16 g_u16ADCTemp[ADCMAXTIMES];

typedef struct
{  //�¶ȴ�����������ݽṹ
    INT16 i16Temprature;
    UINT16 u16Resistance;
} Struct_TempratureTable;

#define NTC_TABLE_LEN 155  //�¶ȱ�񳤶�

const Struct_TempratureTable tagNTCTable[NTC_TABLE_LEN] = {  //�¶ȷŴ�10��������ֵ�Ŵ�100��
    {-490, 62800}, {-480, 59300}, {-470, 56000}, {-460, 52600}, {-450, 49200}, {-440, 46000}, {-430, 42900},
    {-420, 40100}, {-410, 37900},  //ͨ����ϵõ�����10������
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

// �ο���ѹ��4.656V �Ŵ�1000��
#define VREF1000 4656

//************************Public function declaration******************************
// ����ADת����� ���浽g_u16ADCTemp[0]
void CalculateADCResult(void);
// ����AD�������ʵ������ֵ
void CalculatePhysicalValue(UINT8 u8AnalogID);
// ����һ��ADCת��
void StartADConvert(void);
//�����¶�ֵ
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

//    //�๤ѡ��Ĵ�����ADMUX
//    //    Bits 4:0 (MUX4:0)�� ģ��ͨ��������ѡ��λ ��ʼ��Ϊѡ�񵥶����� ADC0
//    //    Bit 5 (ADLAR)    �� ADLAR��λʱת�����Ϊ����룬����Ϊ�Ҷ��� ��ʼ��Ϊ�Ҷ���
//    //    Bit 7:6 (REFS1:0)�� �ο���ѹѡ�� ��ʼ��ѡ���ⲿ��ѹ�ο�
//    //                 0 0 AREF���ڲ�Vref �ر�
//    //                 0 1 AVCC��AREF ��������˲�����
//    //                 1 0 ����
//    //                 1 1 2.56V ��Ƭ�ڻ�׼��ѹԴ�� AREF ��������˲�����
//	ADMUX = 0x00;
//    //ADC ���ƺ�״̬�Ĵ���A �� ADCSRA
//    //    Bits 2:0 (ADPS2:0)�� ADC Ԥ��Ƶ��ѡ��λ,ȷ��XTAL��ADC����ʱ��֮��ķ�Ƶ����
//    //                         0��2��Ƶ�� 1��2��Ƶ�� 2��4��Ƶ�� 3��8��Ƶ
//    //                         4��16��Ƶ��5��32��Ƶ��6��64��Ƶ��7��128��Ƶ  ʹ��32��Ƶ�����ת�����ȣ�
//    //    Bit 3 (ADIE)      �� ADC�ж�ʹ�ܣ���ADIE��SREG��λI��λ�� ADCת�������жϼ���ʹ��
//    //    Bit 4 (ADIF)      �� ADC �жϱ�־����ADCת�������������ݼĴ��������º�ADIF ��λ��
//    //    Bit 5 (ADATE)     �� ADC �Զ�����ʹ�ܣ�ADATE��λ������ADC�Զ��������ܡ�
//    //    Bit 6 (ADSC)      �� ADC ��ʼת�����ڵ���ת��ģʽ�£�ADSC ��λ������һ��ADC ת����
//    //                         ������ת��ģʽ�£�ADSC ��λ�������״�ת����
//    //    Bit 7 (ADEN)      �� ADC ʹ�ܣ�ADEN��λ������ADC������ADC���ܹرա�
//	// ADCʹ��������д1��ADIF����жϱ�־������AD�ж�
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
    //    ADCSRA |= _BV(ADSC);				//����ADת��
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
    /* ģ��������ͨ����ų������� */
    typedef enum _ADC_PORT_NUM_
    {
        ADC_P1,
        ADC_P2,
        ADC_NTC1,
        ADC_NTC2,

        ADC_PORT_NUM
    } ADC_PORT_NUMBER;
    /* ģ��������ͨ����ţ� 0��3 */
    static UINT8 s_u8AnalogPort = ADC_P1, s_u8AnalogID;

    /* ģ������������״̬���� */
    typedef enum _ADC_PROCESS_NUM_
    {
        ADC_PORT,  // 0	����ͨ���л�
        ADC_HOLD,  // 1	�˲�����
        ADC_EXE,   // 2	ADת��
        ADC_DEAL,  // 3	AD�������
        ADC_PROCESS_NUM
    } ADC_PROCESS_DEFINITION;
    /* ģ������������״̬��0������ͨ���л� 1���˲����֣�2��ADת����3��AD������� */
    static UINT8 s_u8ADCProcess = ADC_PORT;

    static UINT16 s_u16ADCHoldingTimer = 0;
/* ADת��ͨ���л�ʱ�˲�����ʱ�䣬��λmS */
#define ADCHOLDTIME 10
    s_u8AnalogPort       = s_u8AnalogPort;
    s_u16ADCHoldingTimer = s_u16ADCHoldingTimer;
    // ���ݲ�������״̬������Ӧ����
    switch (s_u8ADCProcess)
    {
            //		case ADC_PORT:	//ģ�������� ͨ���л�״̬
            //			// Change the channel to the next one
            //			s_u8AnalogPort = (s_u8AnalogPort + 1) % ADC_PORT_NUM;
            //		    //Set the selected channel
            ////            ADMUX = s_u8AnalogPort;

            //			// ����������ʼ��: ����������ģ����ID��ͨ�������˲�ʱ���ʼ��
            //			g_u8ADCSampleTimes = 0;
            //			// ģ����ID ��ͨ������ͬ
            //			s_u8AnalogID = s_u8AnalogPort;
            //			s_u16ADCHoldingTimer = GetCurrMs();
            //		break;

            //        case ADC_HOLD:	//ģ�������� �˲�����״̬
            //        	if(GetMsTimeGap(s_u16ADCHoldingTimer) < ADCHOLDTIME)
            //			{
            //				return;
            //			}

            //		break;

            //        case ADC_EXE:	//ģ�������� ADת��״̬
            //		{
            //		    if(g_u8ADCSampleTimes < ADCMAXTIMES)
            //			{
            //				StartADConvert();
            //				return;
            //			}
            //		}
            //		break;

        case ADC_DEAL:  //ģ�������� AD�������״̬
        {
            CalculateADCResult();

            CalculatePhysicalValue(s_u8AnalogID);
        }
        break;

        default:
            break;
    }

    // ��������״̬ˢ��
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

    // Find the index of max value and min value in array �ҵ�������Сֵ����
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

    // Calculate the mean value without the max and min one ȥ����һ���������Сֵ���ƽ��ֵ
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
//��ѹ��ѹ������������ 0.5V~4.5V ��Ӧ 0��34BAR �Ŵ�1000��
#define PVTYPE_VL 500
#define PVTYPE_INVALD_VL 200
#define PVTYPE_VH 4500
#define PVTYPE_INVALD_VH 4800

#define PVTYPE_PL 0
#define PVTYPE_PH 3400
    UINT8 l_u8Tmp;
    UINT16 l_u16Temp;

#define P_DELTA_ERROR 5          //�仯0.05bar
#define P_ERROR_FLT_INIT_TM 120  //ѹ���˲�������ʼֵ
#define P_FLT_UP_TM 130          //ѹ���˲��������Ӵ���
#define P_FLT_DN_TM 110          //ѹ���˲��������ʹ���
    static UINT8 s_u8PFltTimes[2] = {P_ERROR_FLT_INIT_TM, P_ERROR_FLT_INIT_TM};

    // Calculate the ADC port input voltage �Ŵ�100������λV
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
            //��������ʼ��Ϊ��Ч
            g_u8PSensorOutRangeFlag[u8AnalogID] = TRUE;

            //��ѹ��ѹ������������
            if (l_u16Temp < PVTYPE_VL)
            {
                //����ѹ���ĵ�ѹ�źų�����Χ<=0.2V ʱ��Ϊ��������Ч
                if (l_u16Temp > PVTYPE_INVALD_VL)
                {  //����ѹ���ĵ�ѹ�źŷ�Χ > 0.2V ʱ��Ϊ��������Ч
                    g_u8PSensorOutRangeFlag[u8AnalogID] = FALSE;
                }
                // ��ѹ�ź�С������ʱ��ѹ��ֵ��ֵΪ����ֵ 0 BAR
                l_u16Temp = PVTYPE_PL;
            }
            else if (l_u16Temp > PVTYPE_VH)
            {
                //����ѹ���ĵ�ѹ�źų�����Χ >= 4.8V ʱ��Ϊ��������Ч
                if (l_u16Temp < PVTYPE_INVALD_VH)
                {  //����ѹ���ĵ�ѹ�źŷ�Χ < 4.8V ʱ��Ϊ��������Ч
                    g_u8PSensorOutRangeFlag[u8AnalogID] = FALSE;
                }
                // ��ѹ�źŴ�������ʱ��ѹ��ֵ��ֵΪ����ֵ 34 BAR
                l_u16Temp = PVTYPE_PH;
            }
            else
            {  // P = (Vp - Vl) * (Ph - Pl) / (Vh - Vl)
                g_u8PSensorOutRangeFlag[u8AnalogID] = FALSE;
                l_u16Temp =
                    (UINT16)((UINT32)(l_u16Temp - PVTYPE_VL) * (PVTYPE_PH - PVTYPE_PL) / (PVTYPE_VH - PVTYPE_VL));
            }

            l_u8Tmp = u8AnalogID - ANALOGID_P1;
            //ѹ���˲����� ����ʱ��Ҫ�˲�������ƽ����������ֵ P_DELTA_ERROR
            if (ABS((INT16)((INT32)l_u16Temp - (INT32)g_i16Analog[u8AnalogID])) > P_DELTA_ERROR)
            {
                // �������� �˲�
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
                // �������� �˲�
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
            else  // ƽ���仯
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
#define T_DELTA_ERROR 10  //�仯1��

    i         = i;
    l_u8Temp  = l_u8Temp;
    l_u32Temp = l_u32Temp;
    //�ɲ�����ѹ�����¶ȴ���������(��λΪKohm) (4.656  - V) / (10) = 4.656  / (10.1 + Res)
    //����ֵ(��λΪKohm)�Ŵ�100����ǿ��Ϊ������ ��ѹ�Ŵ�1000��
    l_u32Temp = (UINT32)(VREF1000)*1000 / (VREF1000 - u16TempSampleV) - 1010;

    l_u8Temp = NTC_TABLE_LEN;

    g_u8PSensorOutRangeFlag[u8TempIDIndex] = TRUE;

    //    if(l_u32Temp >= pgm_read_word(&tagNTCTable[0].u16Resistance))
    //    {//�����¶�����
    //        l_i16Temp = pgm_read_word(&tagNTCTable[0].i16Temprature) - 1;
    //    }
    //    else if(l_u32Temp < pgm_read_word(&tagNTCTable[l_u8Temp - 1].u16Resistance))
    //    {//�����¶�����
    //        l_i16Temp = pgm_read_word(&tagNTCTable[l_u8Temp - 1].i16Temprature) + 1;
    //    }
    //    else
    //    {//����
    //		for(i = 0; i < l_u8Temp; i++)
    // 	    {
    // 	       if(l_u32Temp >= pgm_read_word(&tagNTCTable[i].u16Resistance))
    //   	       {
    //    	        break;
    //           }
    //    	}
    //        l_i16Temp = pgm_read_word(&tagNTCTable[i].i16Temprature);
    //        //��ֵ����
    //        l_i16Temp = l_i16Temp - ((UINT16)l_u32Temp - pgm_read_word(&tagNTCTable[i].u16Resistance)) * 10
    //                      / (pgm_read_word(&tagNTCTable[i-1].u16Resistance) -
    //                      pgm_read_word(&tagNTCTable[i].u16Resistance));

    //		g_u8PSensorOutRangeFlag[u8TempIDIndex] = FALSE;
    //	}

    //�¶��˲�����
    if (ABS(l_i16Temp - g_i16Analog[u8TempIDIndex]) >= T_DELTA_ERROR)
    {
        // �¶�����1�ȳ���500ms����Ϊ��Ч
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
