/*********************************************************
  Copyright (C), 2014, Sunrise Group. Co., Ltd.
  File name:      	com_proc.c
  Author: gongping	Version: 0.7       Date:  2014-12-05
  Description:    	Main entry, system threads initialization
  Others:         	n.a
  Function List:  	n.a
  Variable List:  	n.a
  Revision History:         
  Date:           Author:          Modification:
	2014-12-05      gongping         file create
*********************************************************/


#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "threads.h" 
#include "sys_conf.h"
#include "dio.h"
#include "i2c_bsp.h"
#include "global_var.h"
#include "fanctrl.h"
/**/
static void hw_drivers_init(void);
extern uint16_t init_load_default(void);

osThreadId tid_daq;
osThreadId tid_com;
osThreadId tid_bkg;

osThreadDef(core_proc, osPriorityNormal, 1, 0);
osThreadDef(com_proc, osPriorityAboveNormal, 1, 0);
osThreadDef(bkg_proc, osPriorityNormal, 1, 0);

/*********************************************************
  * @name   main
	* @brief  Main function, global initialization and user thread creation
	* @calls  g_var_init()
            osThreadCreate()
						osDelay()            
  * @called main()
  * @param  None
  * @retval None
*********************************************************/
int main(void)   
{   
	hw_drivers_init();			//��ʼ������
	sys_global_var_init();
//	eev1_timerInit(g_sys.config.general.excSpeed);			//@EEV1
//	osDelay(10); 
//	eev2_timerInit(g_sys.config.general.excSpeed);			//@EEV2
	tid_daq = osThreadCreate(osThread(core_proc), NULL);					//data aquization process initialization
  tid_com = osThreadCreate(osThread(com_proc), NULL);					//communication process initialization
	tid_bkg = osThreadCreate(osThread(bkg_proc), NULL);					//background process initialization
	osDelay(osWaitForever); 	
	while(1)
	{;}			
}

static void hw_drivers_init(void)
{
//	eev_init();			//�������ͷ�
 	drv_dio_init();	//
	drv_adc_dma_init();	//��·ѹ����NTC
	drv_i2c_init();		//AT24C256 ģ��IIC��ʼ��
	// ������ٿ���ģ���ʼ��
	InitFanCtrl();
}




