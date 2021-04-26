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
	// ��������ѹ����λΪ�����ѹ�İٷֱ�, �Ŵ�100�� ��Χ0, 3000��10000
	extern UINT16 g_u16FanPwrVout100;

	// �������ɿع�������ƽ�(��������ѹ�Ĺ����)����λΪ��, �Ŵ�10�� ��Χ 0��1200
	extern UINT16 g_u16FanScrAlfa;

	// ���ѹ��ֵ��û�д�����ʱʹ��8888
	extern INT16 g_i16MaxPressureNow;


//************************Interface function declaration******************************
	void InitFanCtrl(void);

    //FAN AC supply voltage rectification algorithm:
	//       Calculate percentage of Vo/Vi and related phase angle
    void FanCtrlAlgorithm(void);

	//���������ѹ�ٷֱȼ�����ƽǶ�
	void CalculateAngleByVout(void);
	
	void FanCtrlStatus(void);

#endif

