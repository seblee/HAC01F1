#ifndef __ACFrequency_H__
#define __ACFrequency_H__

#include "sys_def.h"

// ************************Public global constant definition************************
#define NUM 10
enum
{
	SAMPLE_ZERO = 0,
	SAMPLE_CYC ,
	SAMPLE_MAX,
};
// ************************Public global variable definition************************
	/* 交流电压周期，单位uS, 范围 15～22mS（45～65Hz） */
	extern UINT16 g_u16ACCyc;
	// 交流频率状态 BIT0 - 频率异常; BIT1 - 三相交流缺相;
	extern UINT8 g_u8AcFrequencyStatus;
	extern UINT8 g_u8AcZeroFail;
	extern UINT16 g_u16AcZeroDelay[2];
// ************************Interface function declaration******************************

	// 交流周期计时变量更新 在 定时器 溢出定时中断中调用
	void vUpdateAcCyc(UINT16 u16Time);
	// 获取线电压交流周期 在过零点触发的外部中断中调用
	void vGetAcCyc(UINT8 u8Type);
	// 计算交流周期 与 判断交流电压频率是否正常 45~65HZ为正常 是否缺相
	void ChkACFrequency(void);
	uint16_t AVGfilter(uint8_t i8Type,int16_t i16Value);
//	extern UINT16 ABS(INT16 i16Val);
#endif


