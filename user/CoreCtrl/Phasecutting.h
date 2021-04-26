#ifndef _PHASECUTTING_H
#define _PHASECUTTING_H


#include "sys_def.h"

//************************Public global constant definition************************
		//可控硅始终关闭角 150度 放大10倍
		#define SCR_OFF_ANGLE 1500
		//可控硅始终导通角
		#define SCRONANGLEVALUE 0
		
		#define SCR_PIN      	GPIO_Pin_8
		#define SCR_PORT     	GPIOA	
		
// ************************Public global variable definition************************
   	//1us counter, Refresh in timer2 overflow interruption
    extern UINT32 g_u32Counter1us;
		extern UINT16 g_u16PhaseAngleTcnt;


// ************************Interface function declaration******************************
	//Initate timer0 timing overflow interruption
	void InitT0(uint16_t timPeriod,uint8_t u8Type);
	void InitT1(uint16_t timPeriod,uint8_t u8Type);
	void InitT2(void);

	//Initiate external interruption
	void InitEXINT1(void);
	void InitEXINT2(void);
	void PaSCR_Init(void);
	// 保存 交流周期定时器T2 当前定时时间值 单位：us
	UINT16 u16GetT2Val(void);

	// 根据交流周期、移相控制角和脉冲触发间隔角度（60度）计算
	// 相对过零点触发开始时间的定时器0溢出中断计数值 与
	// 触发间隔时间对应定时1溢出中断计数值 与
	// 触发脉宽时间
	void vCalPhaseCuttingTime(UINT16 u16PhaseAngle);

#endif


