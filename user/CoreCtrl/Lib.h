#ifndef _LIB_H
#define _LIB_H


#include "sys_def.h"
	//************************Public Constant definition************************
	#ifndef MAX
	#define MAX(x,y)    ( ((x)>=(y)) ? (x) :  (y) )
	#endif

	#define TRUE  	1
	#define FALSE 	0

	#define INVALID 0xff
	#define VALID 0x00
// ************************Interface function declaration******************************
	void WatchDog(void);

//	UINT16 ABS(INT16 i16Val);


	//Time related task
	void TimeTask(void);

	// 获取当前时间与参考时间的毫秒级时间间隔 u16MsRef － 开始参考时间
	UINT16 GetMsTimeGap(UINT16 u16MsRef);
	// 获取当前相对时间基准 单位：毫秒
	UINT16 GetCurrMs(void);
	// 获取当前时间与参考时间的秒级时间间隔 u8SecRef － 开始参考时间
	UINT8 GetSecTimeGap(UINT8 u8SecRef);
	// 获取当前相对时间基准 单位：秒
	UINT8 GetCurrSec(void);
	// 获取当前时间与参考时间的分钟级时间间隔 u8MinRef － 开始参考时间
	UINT8 GetMinTimeGap(UINT8 u8MinRef);
	// 获取当前相对时间基准 单位：分钟
	UINT8 GetCurrMin(void);
#endif /*----------FSC3P08DRV_H-----------------------------------------------------*/

