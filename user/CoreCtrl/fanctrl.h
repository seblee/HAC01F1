#ifndef _FANCTRL_H
#define _FANCTRL_H

#include "sys_def.h"
//************************Public global constant definition************************
typedef struct 
{
	uint16_t 	PSET;
	uint16_t	PBAND;
}Refrigerant_st;
enum 
{
	 Ref_R22=0,
	 Ref_R407C,
	 Ref_R410A,
	 Ref_MAX,
 } ;

//************************Public global variable definition************************
	// 风机供电电压，单位为输入电压的百分比, 放大100倍 范围0, 3000～10000
	extern UINT16 g_u16FanPwrVout100;

	// 风机供电可控硅移相控制角(相对于相电压的过零点)，单位为度, 放大10倍 范围 0～1200
	extern UINT16 g_u16FanScrAlfa;

	// 最大压力值，没有传感器时使用8888
	extern INT16 g_i16MaxPressureNow;


//************************Interface function declaration******************************
	void InitFanCtrl(void);

    //FAN AC supply voltage rectification algorithm:
	//       Calculate percentage of Vo/Vi and related phase angle
    void FanCtrlAlgorithm(void);

	//根据输出电压百分比计算控制角度
	void CalculateAngleByVout(void);
	
	void FanCtrlStatus(void);

#endif

