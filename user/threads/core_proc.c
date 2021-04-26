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
#include "fanctrl.h"
#include "acfrequency.h"
#include "Lib.h"



/*********************************************************
  * @name   eev_proc
	* @brief  Sample water quality, humidifier current and water level signals
	* @calls  adc_init()
            calc_conductivity()
            calc_humcurrent()
						water_level_sts_get()
						osDelay()            
  * @called main()
  * @param  *argument : versatile pointer, not used
  * @retval None
*********************************************************/
void core_proc(void const *argument)
{
	extern sys_reg_st g_sys;
 	// 上电运行时间参考 延迟启动参考标志
 	static UINT8 s_u8StartDlyTm = 0;
 	static UINT8 s_u8StartFlag = FALSE;
	// 获取当前相对时间基准 单位：秒
	s_u8StartDlyTm = GetCurrSec();
	while(1)
	{			
		ChkACFrequency();
		//FAN AC supply voltage rectification algorithm:
		//Calculate percentage of Vo/Vi and related phase angle
		// 复位3秒后执行显示和风机控制
//		if((GetSecTimeGap(s_u8StartDlyTm) > 3) || s_u8StartFlag)
		{
			s_u8StartFlag = TRUE;		
			FanCtrlAlgorithm();
		}
		osDelay(CORE_PROC_DLY);
	}
}
