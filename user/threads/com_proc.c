/*********************************************************
  Copyright (C), 2014, Sunrise Group. Co., Ltd.
  File name:      	com_proc.c
  Author: gongping	Version: 0.7       Date:  2014-12-05
  Description:    	Communication handling thread, 
										dealing with RS48 bus activities
										and command resolving
  Others:         	n.a
  Function List:  	frame_detect(void);
	                  cmd_resolve(void);
										framing(uint8_t cmd_type, const uint8_t *tx_buf, uint8_t length);
  Variable List:  	rx_framebuf[32]
	                  tx_framebuf[32]
										tx_buf[16]
  Revision History:         
  Date:           Author:          Modification:
	2014-12-05      gongping         file create
*********************************************************/

#include "cmsis_os.h"  
#include "mb.h"
#include "port.h"
#include "mb_cb.h"
#include "sys_conf.h"
#include "Lib.h"

typedef struct
{
		uint16_t baudrate;
		uint16_t com_addr;
}communication_change_st;

communication_change_st com_change_inst;

static void change_surv_baudrate(void)
{
	extern sys_reg_st					g_sys; 
	uint8_t u8MB=0;
	
	if((com_change_inst.baudrate != g_sys.config.general.surv_baudrate)||(g_sys.config.general.surv_addr != com_change_inst.com_addr ))
	{
		u8MB=1;
	}
	//通讯模式
	if(g_sys.config.g_u8CfgParameter[CFGCOMMODE])
	{
		if(g_sys.status.Com_error[0]>=COMERR_5S*10)
		{
			u8MB=1;	
			g_sys.status.Com_error[0]=0;
		}
	}
	//重新初始化串口
	if(u8MB)
	{
			com_change_inst.baudrate  =  g_sys.config.general.surv_baudrate;
			com_change_inst.com_addr = g_sys.config.general.surv_addr;
		  //eMBDisable();
		  eMBInit( MB_RTU, mb_get_device_addr(), 0, mb_get_baudrate(com_change_inst.baudrate), MB_PAR_NONE );	
			eMBEnable();
	}
	return;
}
/*********************************************************
  * @name   com_proc
	* @brief  communication thread, deal with network protocal and command resolving.
	* @calls  mb_reg_init()
            eMBPoll()
						eMBInit()
						eMBEnable()
            mb_reg_update()
						mb_cmd_resolve()
						osDelay()            
  * @called main()
  * @param  *argument : versatile pointer, not used
  * @retval None
*********************************************************/

//void mb_reg_default_load(void);

void com_proc(void const *argument)
{	
	extern sys_reg_st					g_sys; 				
 	// 上电运行时间参考 延迟启动参考标志
 	static UINT8 s_u8StartDlyTm = 0;
 	static UINT8 s_u8StartFlag = FALSE;
	// 获取当前相对时间基准 单位：秒
	s_u8StartDlyTm = GetCurrSec();																															//modbus protocal stack enable
	
	while(1)
	{	
		// 复位2秒后执行通讯
		if((GetSecTimeGap(s_u8StartDlyTm) > 2) && (s_u8StartFlag == FALSE))
		{
			s_u8StartFlag = TRUE;		
			//modbus hold registers initialization
			com_change_inst.baudrate  =  g_sys.config.general.surv_baudrate;
			com_change_inst.com_addr = g_sys.config.general.surv_addr;
			eMBInit( MB_RTU, mb_get_device_addr(), 0, mb_get_baudrate(com_change_inst.baudrate), MB_PAR_NONE );		//modbus protocal stack initialization
			eMBEnable();		
		}
		if(s_u8StartFlag == TRUE)
		{
			eMBPoll();																																	//modbus slave polling																												
			change_surv_baudrate();
		}
		osDelay(COM_PROC_DLY);
	}
}

