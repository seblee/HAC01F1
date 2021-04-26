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
#include "Lib.h"
#include "phasecutting.h"
//************************Public global constant definition************************


//************************Public global variable definition************************
    //System run time by Hours, Minutes, Seconds, and Milliseconds
    UINT16 g_u16Millisecond = 0;
    UINT8 g_u8RunSecond = 0;
    UINT8 g_u8RunMinute = 0;




///*=============================================================================*
// * FUNCTION: WatchDog()
// * PURPOSE : Watchdog trigger and run led trigger
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
// *     TimeTask();
// *
// *============================================================================*/
//void WatchDog(void)
//{
//    PORTC ^= 0x40;
//	return;
//}


//UINT16 ABS(INT16 i16Val)
//{
//	if(i16Val < 0)
//		return (0 - i16Val);
//	else
//		return i16Val;
//}


//void LEDRLYCtrlOn(void)
//{
//	PORTD &= ~_BV(PD4);
//}

//void LEDRLYCtrlOff(void)
//{
//	PORTD |= _BV(PD4);
//}


/*=============================================================================*
 * FUNCTION: TimeTask()
 * PURPOSE : Time related task
 * INPUT:
 *     	g_u32Counter1us
 * OUTPUT
  *   	g_u8RunSecond
 *   	g_u16Millisecond
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
void TimeTask(void)
{
	UINT16 l_u8MsTemp;

    //Refresh system run time START
    if(g_u32Counter1us >= 1000)
    {
    	l_u8MsTemp = g_u32Counter1us / 1000;
			g_u32Counter1us = g_u32Counter1us % 1000;

        g_u16Millisecond = g_u16Millisecond + l_u8MsTemp;

        if(g_u16Millisecond >= 1000)
        {
            g_u16Millisecond = g_u16Millisecond - 1000;
            g_u8RunSecond++;

            if(g_u8RunSecond >= 60)
            {
            	g_u8RunMinute++;
                g_u8RunSecond = g_u8RunSecond - 60;
            }
        }
    }
    //Refresh system run time END
}

// 获取当前时间与参考时间的毫秒级时间间隔 u16MsRef － 开始参考时间
UINT16 GetMsTimeGap(UINT16 u16MsRef)
{
	u16MsRef = (g_u16Millisecond + 1000 - u16MsRef) % 1000 + g_u32Counter1us / 1000;
	return u16MsRef;
}

// 获取当前相对时间基准 单位：毫秒
UINT16 GetCurrMs(void)
{
	return (g_u16Millisecond + g_u32Counter1us / 1000) % 1000;
}

// 获取当前时间与参考时间的秒级时间间隔 u8SecRef － 开始参考时间
UINT8 GetSecTimeGap(UINT8 u8SecRef)
{
	u8SecRef = (g_u8RunSecond + 60 - u8SecRef) % 60 + g_u32Counter1us / 1000000;
	return u8SecRef;
}

// 获取当前相对时间基准 单位：秒
UINT8 GetCurrSec(void)
{
	return (g_u8RunSecond + g_u32Counter1us / 1000000) % 60;
}

// 获取当前时间与参考时间的分钟级时间间隔 u8MinRef － 开始参考时间
UINT8 GetMinTimeGap(UINT8 u8MinRef)
{
	u8MinRef = (g_u8RunMinute + 256 - u8MinRef) % 256 + g_u32Counter1us / 60000000;
	return u8MinRef;
}

// 获取当前相对时间基准 单位：分钟
UINT8 GetCurrMin(void)
{
	return (g_u8RunMinute + g_u32Counter1us / 60000000) % 256;
}
