/*********************************************************
  Copyright (C), 2014, Sunrise Group. Co., Ltd.
  File name:      	com_proc.c
  Author: gongping	Version: 0.7       Date:  2014-12-05
  Description:    	Global variables instantation 
  Others:         	n.a
  Function List:  	void g_var_init(void);		//global variables initialization
  Variable List:  	g_var_st g_var_st_inst;		//global variables structure
  Revision History:         
  Date:           Author:          Modification:
	2014-12-05      gongping         file create
*********************************************************/

#include "global_var.h"
#include "fifo.h"
#include "calc.h"
#include "sys_conf.h"
#include "i2c_bsp.h"
#include "cmsis_os.h"  
#include "eev_proc.h"

extern uint8_t AT24CXX_ReadOneByte(uint16_t ReadAddr);

//uint8_t test = 0x33;

uint16_t conf_reg[CONF_REG_MAP_NUM];
uint16_t test_reg[CONF_REG_MAP_NUM];
typedef enum
{
		INIT_LOAD_USR=0,
		INIT_LOAD_FACT,
		INIT_LOAD_DEBUT,
		INIT_LOAD_DEFAULT,
}init_state_em;

#define	EE_FLAG_LOAD_USR			0xdf
#define	EE_FLAG_LOAD_FACT			0x1b
#define	EE_FLAG_LOAD_DFT			0x9b
#define	EE_FLAG_LOAD_DEBUT		    0xff
sys_reg_st					g_sys;

const conf_reg_map_st conf_reg_map_inst[CONF_REG_MAP_NUM]=
{		// 			id			mapped registers						   			min		max		  default		permission	r/w     chk_ptr
		{	0,			&g_sys.config.general.Cool_Type,				   			0,		10,		  Fixed,			0,			1,      NULL    },
//		{	1,			&g_sys.config.general.eevfactId,							0,		0xffff,   0,			2,			1,		NULL	},
//		{	2,			&g_sys.config.general.eevproType,							0,		0xffff,   0,			2,			1,		NULL	},
//		{	3,			&g_sys.config.general.eevHoldTime,							10,		50,   20,			2,			1,		NULL	},
//		{	4,			&g_sys.config.general.excSpeed,								30,		90,   35,			2,			1,		NULL	},
//		{	5, 			&g_sys.config.general.excAllOpenSteps, 					0,		500,      500,			2,			1,		NULL	},
//		{	6,			&g_sys.config.algorithm.eev_ctrl_mode,			   			0,		1,		  0,			1,			1,      NULL   	},		
//    {	7,			&g_sys.config.general.pressType,						   	0,		1,	  0,		3,			1,      NULL   	},
//		{	8,			&g_sys.config.general.pressMAXBar,					   		0,	2000, 340,		3,			1,      NULL   	},
//    {	9,			&g_sys.config.general.pressMINBar,					   		0,   0xffff, 0,		3,			1,      NULL   	},
		{	1,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	2,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	3,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	4,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	5,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	6,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	7,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	8,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
    {	9,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	10,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
    {	11,			&g_sys.config.dev_mask.ain,						   			0,		0x0f,	  0x03,		    3,			1,      NULL   	},
		{	12,			&g_sys.config.dev_mask.din,						   			0,		0x1f,	  0x1f,		3,			1,      NULL   	},
		{	13,			&g_sys.config.dev_mask.dout,					   			0,		0x03,   0x03,		3,			1,      NULL   	},
		{	14,			&g_sys.config.dev_mask.eev,						   			0,		3,	  3,		3,			1,      NULL   	},
		{	FACTORY_RESET,			&g_sys.config.general.restore_factory_setting,  	        0,	0xff,	  0x00,		2,			1,      NULL    },	
		{	16,			&g_sys.config.general.surv_baudrate,			   			0,		5,	  0,			1,			1,      NULL    },
		{	17,			&g_sys.config.general.surv_addr,				   			1,		32,	      1,			1,			1,      NULL   	},
    {	18,			&g_sys.config.general.alarm_bypass_en,			   			0,		1,		  0,			3,			1,      NULL   	},
    {	19,			&g_sys.config.general.Dehumidity_Supperheart,			   	0,		200,	  0,			3,			1,      NULL   	},
    {	20, 		&g_sys.config.general.refrigType,						0,		3, 	      3,			2,			1,		NULL	},
		{	21,			&g_sys.config.general.ntc_cali[AI_NTC_INDEX1],				   			0,		0xffff,	  0,			2,			1,      NULL   	},
		{	22,			&g_sys.config.general.ntc_cali[AI_NTC_INDEX2],				   			0,	    0xffff,	  0,			2,			1,      NULL   	},
		{	23,		  &g_sys.config.general.ai_cali[AI_SENSOR_INDEX1],				   			0,		0xffff,	  0,			2,			1,      NULL    },
		{	24,		  &g_sys.config.general.ai_cali[AI_SENSOR_INDEX2],			       			0,	    0xffff,	  0,			2,          1,      NULL    },
//		{	25,			&g_sys.config.general.super_heat[EEV1],				   			0,		180,	  60,		    1,			1,      NULL    },
//		{	26,			&g_sys.config.general.super_heat[EEV2],				   			0,		180,	  60,		    1,			1,      NULL    },
//		{	27,			&g_sys.config.general.enable_valve_pos_manual,	   			0,		1,		  0,			2,			1,      NULL    },		
//		{	28,			&g_sys.config.general.valve_pos_manual_comm[EEV1],	   			0,		500,		  250,			2,			1,      NULL    },  //@用于传感器或NTC故障	
//		{	29,			&g_sys.config.general.valve_pos_manual_comm[EEV2],	   			0,		500,		  250,			2,			1,      NULL    },	
		{	25,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	26,			&g_sys.config.general.minVol,					   									0,			5000, 		500,			2,			1,      NULL   	},
    {	27,			&g_sys.config.general.maxVol,					   									0,   		15000, 		4500,			2,			1,      NULL   	},
    {	28,			&g_sys.config.general.pressMINBar,					   						0,   		0xffff, 	0,				2,			1,      NULL   	},	
		{	29,			&g_sys.config.general.pressMAXBar,					   						1000,		6000, 		4600,			2,			1,      NULL   	},
		{	30,			&g_sys.config.g_u8CfgParameter[CFGYEAR],			   					0,	    0xFFFF,	  2020,			2,			1,      NULL    },
		{	31,			&g_sys.config.g_u8CfgParameter[CFGMONTH],			   					0,	    0xFFFF,	  8,				2,			1,      NULL    },
		{	32,			&g_sys.config.g_u8CfgParameter[CFGDAY],			   						0,	    0xFFFF,	  1,				2,			1,      NULL    },                                                         
		{ 33,			&g_sys.config.g_u8CfgParameter[CFGPSETID],			   				1000,	  3000,	  	1300,			2,			1,      NULL    },                
		{	34,			&g_sys.config.g_u8CfgParameter[CFGPBANDID],			   				300,	  700,	  	500,			2,			1,      NULL    },                
		{ 35,			&g_sys.config.g_u8CfgParameter[CFGMINVID],			   				3000,	  5000,	  	3000,			2,			1,      NULL    },                
    {	36,			&g_sys.config.g_u8CfgParameter[CFGMAXVID],			   				6000,	  10000,		10000,		2,			1,      NULL    },     
		{	37, 		&g_sys.config.g_u8CfgParameter[CFGFANNUMID],			   			1,	    2,	  		1,				2,			1,      NULL    },     
		{	38, 		&g_sys.config.g_u8CfgParameter[CFGPSENSORTYPEID],			   	1,	    2,	  		1,				2,			1,      NULL    },     
		{	39,			&g_sys.config.g_u8CfgParameter[CFGCOMMODE],			   				0,	    1,	  		0,				2,			1,      NULL    },  
		{	40,			&g_sys.config.g_u8CfgParameter[CFGCOMFANOUT],			   			0,	    100,	  	0,				2,			2,      fan_com_chk    },  
//		{	40,			&g_sys.config.g_u8CfgParameter[CFGCOMFANOUT],			   			0,	    100,	  	0,				1,			2,      NULL    },  
		{	41,			&g_sys.config.alarm[ACL_ACFREQUENCYALM].enable_mode,	   	0,			7,		  	4,				2,			1,      NULL    },
		{	42,			&g_sys.config.alarm[ACL_ACFREQUENCYALM].delay,		   			1,	    1800,	  	2,				2,			1,      NULL    },
		{	43,			&g_sys.config.alarm[ACL_ACFREQUENCYALM].alarm_param,	   	0,	    150,	    10,				2,			1,      NULL    },
    {	44,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	45,			&g_sys.config.alarm[ACL_PSENSORALM].enable_mode,   				0,			7,		  	4,				2,			1,      NULL   	},
		{	46,			&g_sys.config.alarm[ACL_PSENSORALM].delay,		   					1,			1800,	  	2,				2,			1,      NULL   	},
		{	47,			&g_sys.config.alarm[ACL_PSENSORALM].alarm_param,   				0,			150,	  	10,				2,			1,      NULL    },   
		{	48,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	49,			&g_sys.config.alarm[ACL_COMMON].enable_mode,   						0,			7,		  	4,				2,			1,      NULL   	},
		{	50,			&g_sys.config.alarm[ACL_COMMON].delay,		   							0,			1800,	  	0,				2,			1,      NULL   	},
		{	51,			&g_sys.config.alarm[ACL_COMMON].alarm_param,   						0,			900,	  	20,			  2,			1,      NULL    },
		{	52,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	53,			&g_sys.config.alarm[ACL_EEPROMALM].enable_mode,   				0,			7,		  	4,				2,			1,      NULL   	},
		{	54,			&g_sys.config.alarm[ACL_EEPROMALM].delay,		   						1,			1800,	  	2,				2,			1,      NULL   	},
		{	55,			&g_sys.config.alarm[ACL_EEPROMALM].alarm_param,   				0,			900,	  	400,			2,			1,      NULL    },
		{	56,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	57,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	58,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	59,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	60,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	61,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	62,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	63,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	64,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	65,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	66,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	67,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	68,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	69,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	70,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	71,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	72,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	73,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	74,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	75,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	76,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	77,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	78,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	79,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	80,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	81,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	82,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	83,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	84,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	85,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	86,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	87,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	88,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	89,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	90,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	91,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	92,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	93,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	94,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	95,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	96,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	97,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	98,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
		{	99,			NULL,	   																									0,	    0xFFFF,	  0,				2,			1,      NULL    },
};

//status register map declairation
//STATUS_REG_MAP_OFFSET  110
const sts_reg_map_st status_reg_map_inst[STATUS_REG_MAP_NUM]=
{		// 			id			mapped registers														default				
		{			0,			&g_sys.status.sys_info.status_reg_num,			STATUS_REG_MAP_NUM},
		{			1,			&g_sys.status.sys_info.config_reg_num,		  CONF_REG_MAP_NUM},
		{			2,			&g_sys.status.sys_info.software_ver,				SOFTWARE_VER},
		{			3,			&g_sys.status.sys_info.hardware_ver,				HARDWARE_VER},
		{			4,			&g_sys.status.sys_info.serial_no[3],				SERIAL_NO_3},
		{			5,			&g_sys.status.sys_info.serial_no[2],				SERIAL_NO_2},
		{			6,			&g_sys.status.sys_info.serial_no[1],				SERIAL_NO_1},
		{			7,			&g_sys.status.sys_info.serial_no[0],				SERIAL_NO_0},
		{			8,			&g_sys.status.sys_info.man_date[1],					MAN_DATA_1},
		{			9,			&g_sys.status.sys_info.man_date[0],					MAN_DATA_0},
		//{			10,			&g_sys.status.general.running_mode,					    0},
		{			10,			&g_sys.status.work_mode,									0},	    //
		{			11,			&g_sys.status.status_remap,								0},
		{			12,			&g_sys.status.alarm_status_cnt.total_cnt,				0},
		{			13,			&g_sys.status.alarm_bitmap,								0},
		{			14,			NULL,																				0},
		{     15,			NULL,																				0},       
		{			16,			&g_sys.status.ain[AI_SENSOR1],							0},
		{			17,			NULL,																				0},
		{			18,			&g_sys.status.ain[AI_NTC1],									0},
		{			19,			&g_sys.status.din_bitmap[0],										0},
		{			20,			&g_sys.status.din_bitmap[1],										0},
	  {			21,			&g_sys.status.Fan.u16FanVout100,	  								0},  //风机输出
		{		  22,			&g_sys.status.Fan.Fan_out,	  											0},  //风机输出
		{			23,			&g_sys.status.Fan.u16PressuePoint[P_SET_POINT],	  	0},  
		{		  24,			&g_sys.status.Fan.u16PressuePoint[P_OFF_POINT],	  	0},  
		{			25,			&g_sys.status.Fan.u16PressuePoint[P_FLAT_POINT],	  0},  
		{		  26,			&g_sys.status.Fan.u16PressuePoint[P_FLAT2_POINT],	  0},  
		{			27,			&g_sys.status.Fan.u16PressuePoint[P_STEP_POINT],	  0},  
		{			28,			&g_sys.status.Fan.u16PressuePoint[P_STEP_BAND],	  	0}, 
		{			29,			NULL,																				0},
		{			30,			&g_sys.status.Fan.ACCye[0],	  							0},  
		{			31,			&g_sys.status.Fan.ACCye[1],	  							0},  
		{			32,			&g_sys.status.Fan.ACCye[2],	  							0},  
		{			33,			&g_sys.status.Com_error[0],										0},
		{			34,			&g_sys.status.Com_error[1],										0},
		{			35,			&g_sys.status.Test_Buff[0],									0},
		{			36,			&g_sys.status.Test_Buff[1],									0},
		{			37,			&g_sys.status.Test_Buff[2],									0},
		{			38,			&g_sys.status.Test_Buff[3],									0},
		{			39,			&g_sys.status.Test_Buff[4],									0},
		{			40,			&g_sys.status.Test_Buff[5],									0},
		{			41,			&g_sys.status.Test_Buff[6],									0},
		{			42,			&g_sys.status.Test_Buff[7],									0},
		{			43,			&g_sys.status.Test_Buff[8],									0},
		{			44,			&g_sys.status.Test_Buff[9],									0},
		{			45,			&g_sys.status.Test_Buff[10],									0},
		{			46,			&g_sys.status.Test_Buff[11],									0},
		{			47,			&g_sys.status.Test_Buff[12],									0},
		{			48,			&g_sys.status.Test_Buff[13],									0},
		{			49,			&g_sys.status.Test_Buff[14],									0},
		{			50,			&g_sys.status.Test_Buff[15],									0},
		{			51,			&g_sys.status.Test_Buff[16],									0},
		{			52,			&g_sys.status.Test_Buff[17],									0},
		{			53,			&g_sys.status.Test_Buff[18],									0},
		{			54,			&g_sys.status.Test_Buff[19],									0},
		{			55,			&g_sys.status.Test_Buff[20],									0},
		{			56,			&g_sys.status.Test_Buff[21],									0},
		{			57,			&g_sys.status.Test_Buff[22],									0},
		{			58,			&g_sys.status.Test_Buff[23],									0},
		{			59,			&g_sys.status.Test_Buff[24],									0},
		{			60,			&g_sys.status.Test_Buff[25],									0},
		{			61,			&g_sys.status.Test_Buff[26],									0},
		{			62,			&g_sys.status.Test_Buff[27],									0},
		{			63,			&g_sys.status.Test_Buff[28],									0},
		{			64,			&g_sys.status.Test_Buff[29],									0},
		{			65,			&g_sys.status.Test_Buff[30],									0},
		{			66,			&g_sys.status.Test_Buff[31],									0},
		{			67,			&g_sys.status.Test_Buff[32],									0},
		{			68,			&g_sys.status.Test_Buff[33],									0},
		{			69,			&g_sys.status.Test_Buff[34],									0},
};                                                  



/**
  * @brief  get eeprom program status
  * @param  None
  * @retval 
		`EE_FLAG_OK:		configuration data valid in eeprom
		`EE_FLAG_EMPTY:	eeprom empty
  */

static init_state_em get_ee_status(void)
{
		init_state_em em_init_state;
		uint8_t ee_pflag;
												//wait for eeprom power on
		I2C_EE_BufRead(&ee_pflag,STS_EE_ADDR,1);//启动区
//		//TEST
//		ee_pflag =INIT_LOAD_DEBUT;
		switch(ee_pflag)
		{
				case(EE_FLAG_LOAD_USR):
				{
						em_init_state = INIT_LOAD_USR;
						break;
				}
			
				case(EE_FLAG_LOAD_FACT):
				{
						em_init_state = INIT_LOAD_FACT;
						break;
				}		
				case(EE_FLAG_LOAD_DFT):
				{
						em_init_state = INIT_LOAD_DEFAULT;
						break;
				}						
				default:
				{
						em_init_state = INIT_LOAD_DEBUT;
						break;
				}				
		}
		return em_init_state;
} 

/**
  * @brief 	save system configurable variables initialization
	* @param  0:load usr1 eeprom
						1:load usr2 eeprom
						2:load facotry eeprom
						3:load default eeprom
	* @retval err_cnt: mismatch read/write data count
  */

uint16_t set_load_flag(uint8_t ee_load_flag)
{
		uint8_t ee_flag;
		switch (ee_load_flag)
		{
				case (0):
				{
						ee_flag = EE_FLAG_LOAD_USR;
						break;
				}
				case (1):
				{
						ee_flag = EE_FLAG_LOAD_FACT;
						break;
				}
				case (2):
				{
						ee_flag = EE_FLAG_LOAD_DEBUT;
						break;
				}				
				default:
				{
						ee_flag = EE_FLAG_LOAD_DFT;			
						break;
				}
		}
		I2C_EE_BufWrite(&ee_flag,STS_EE_ADDR,1);
		return 1;
}

/**
  * @brief 	save system configurable variables initialization
	* @param  addr_sel:
						`0: save current configuration to usr1 eeprom address
						`1:	save current configuration to usr2 eeprom address
						`2:	save current configuration to facotry eeprom address
	* @retval err_cnt: mismatch read/write data count
  */
uint16_t save_conf_reg(uint8_t addr_sel)
{		
		//uint16_t conf_reg[CONF_REG_MAP_NUM];
		//uint16_t test_reg[CONF_REG_MAP_NUM];
		uint16_t i,j,err_cnt,chk_res;
		uint16_t ee_save_addr;
		uint8_t ee_flag,req;
		
		
		ee_save_addr =0;
		err_cnt = 0;
	
		switch (addr_sel)
		{
				case(0):
				{
						ee_flag = EE_FLAG_LOAD_USR;
						break;
				}
				case(1):
				{
						ee_save_addr = CONF_REG_FACT_ADDR;
						ee_flag = EE_FLAG_LOAD_FACT;
						break;
				}			
				default:
				{
						return 0xff;
				}
		}		
		
		for(i=0;i<CONF_REG_MAP_NUM;i++)																								//set configration reg with default value
		{
				conf_reg[i] = *(conf_reg_map_inst[i].reg_ptr);
		}	
		if(ee_flag == EE_FLAG_LOAD_USR)
		{
				req = 0;
				for(j=0;j<3;j++)
				{
						if(j == 0)
						{
							ee_save_addr=CONF_REG_EE1_ADDR;
						}
						else if(j == 1)
						{
							ee_save_addr=CONF_REG_EE2_ADDR;
						}
						else
						{
							ee_save_addr=CONF_REG_EE3_ADDR;
						}

						I2C_EE_BufWrite((uint8_t *)conf_reg,ee_save_addr,(CONF_REG_MAP_NUM)*2);				//save configuration data to eeprom
						for(i=0;i<10;i++);
						I2C_EE_BufRead((uint8_t *)test_reg,ee_save_addr,(CONF_REG_MAP_NUM)*2);
						for(i=0;i<CONF_REG_MAP_NUM;i++)
						{
								if(conf_reg[i] != test_reg[i])
								{
										err_cnt++;
								}
						}
						if(err_cnt == 0)
						{
								chk_res = checksum_u16(conf_reg,CONF_REG_MAP_NUM);													//set parameter checksum
								I2C_EE_BufWrite((uint8_t*)&chk_res,ee_save_addr+(CONF_REG_MAP_NUM*2),2);

							
								I2C_EE_BufWrite(&ee_flag,STS_EE_ADDR,1);																		//set eeprom program flag
						}
						else
						{
							req++;
						}

						
				}
				if(req<2)
				{
							err_cnt = 0;
				}
				else
				{
							err_cnt = req;
				}
				
		}
		else
		{
				I2C_EE_BufWrite((uint8_t *)conf_reg,ee_save_addr,(CONF_REG_MAP_NUM)*2);				//save configuration data to eeprom
				for(i=0;i<10;i++);
				I2C_EE_BufRead((uint8_t *)test_reg,ee_save_addr,(CONF_REG_MAP_NUM)*2);
				for(i=0;i<CONF_REG_MAP_NUM;i++)
				{
						if(conf_reg[i] != test_reg[i])
						{
								err_cnt++;
						}
				}
				if(err_cnt == 0)
				{
						chk_res = checksum_u16(conf_reg,CONF_REG_MAP_NUM);													//set parameter checksum
						I2C_EE_BufWrite((uint8_t*)&chk_res,ee_save_addr+(CONF_REG_MAP_NUM*2),2);
						I2C_EE_BufWrite(&ee_flag,STS_EE_ADDR,1);																		//set eeprom program flag
				
				}
		}
	
		return err_cnt;
}
uint16_t init_load_default(void)
{
		uint16_t i, ret;
		ret =1;
		for(i=0;i<CONF_REG_MAP_NUM;i++)		//initialize global variable with default values
		{
				if(conf_reg_map_inst[i].reg_ptr != NULL)
				{
						*(conf_reg_map_inst[i].reg_ptr) = conf_reg_map_inst[i].dft;
				}
		}		
		ret = 1;
		g_sys.status.status_remap |= 0x0001;			//?????	
		
		return ret;
}

 
/**
  * @brief  load system configuration data from eeprom
  * @param  None
  * @retval None
  */
  


static uint16_t conf_reg_read_ee(uint16_t addr)
{
		uint16_t reg;
//	    uint16_t err_num = 0;
		uint16_t ee_err,ret;
		reg = 0;
		ee_err = 0;
		ret = 0;
		ee_err = eeprom_compare_read(addr,&reg);
		if((conf_reg_map_inst[addr].reg_ptr != NULL)&&((reg < conf_reg_map_inst[addr].min) || (reg > conf_reg_map_inst[addr].max)))
		{
				*(conf_reg_map_inst[addr].reg_ptr) = (conf_reg_map_inst[addr].dft);				    
				ret = 0;	
		}
		else
		{
				*(conf_reg_map_inst[addr].reg_ptr) = reg;
				ret = 1;
		}
		
		if((ee_err != 0)||(ret == 0))
		{
				ret = 1;
		}
		else
		{
				ret = 0;
		}
		return ret;
}


static uint16_t init_load_user_conf(void)
{		
		uint16_t i,sum,sum_reg;
		sum = 0;
	
		for(i=0;i<CONF_REG_MAP_NUM;i++)
		{			
				sum_reg = sum;
				sum += conf_reg_read_ee(i);
			    
				if(sum != sum_reg)
				{
					//break;
				}
		}
		
		if(sum == 0)
		{
				return(1);
		}
		else
		{
				return(0);
		}
		
}

static uint16_t init_load_factory_conf(void)
{		
		uint16_t buf_reg[CONF_REG_MAP_NUM+1];
		uint16_t i;
		uint16_t chk_res;
		uint16_t ee_load_addr;
		ee_load_addr = CONF_REG_FACT_ADDR;
		
		I2C_EE_BufRead((uint8_t *)buf_reg,ee_load_addr,(CONF_REG_MAP_NUM+1)*2);		//read eeprom data & checksum to data buffer																											//wait for i2c opeartion comletion
		chk_res = checksum_u16(buf_reg,(CONF_REG_MAP_NUM+1));
		if(chk_res != 0)	//eeprom configuration data checksum fail
		{
				for(i=0;i<CONF_REG_MAP_NUM;i++)		//initialize global variable with default values
				{
						if(conf_reg_map_inst[i].reg_ptr != NULL)
						{
								*(conf_reg_map_inst[i].reg_ptr) = conf_reg_map_inst[i].dft;
						}
				}
				return 0;
				
		}
		else
		{
				for(i=0;i<CONF_REG_MAP_NUM;i++)
				{
						*(conf_reg_map_inst[i].reg_ptr) = buf_reg[i];
				}		
				
				return 1;
		}			
}


/**
  * @brief  initialize system status reg data
  * @param  None
  * @retval None
  */
static void init_load_status(void)
{
		uint8_t i;	
		for(i=0;i<STATUS_REG_MAP_NUM;i++)
		{
				if(status_reg_map_inst[i].reg_ptr != NULL)
				{
						*(status_reg_map_inst[i].reg_ptr) = status_reg_map_inst[i].dft;
				}
		}
}

/**
  * @brief  system configurable variables initialization
  * @param  None
  * @retval 
			1:	load default data
			2:	load eeprom data
  */
uint16_t sys_global_var_init(void)
{	
		uint16_t ret;
	    
		init_state_em	em_init_state;
		osDelay(100); 
		init_load_status();
        //I2C_EE_BufWrite(&test,STS_EE_ADDR,1);
		em_init_state = get_ee_status();		//get eeprom init status
		switch(em_init_state)
		{
				case(INIT_LOAD_USR):						//load usr1 data
				{
						ret = init_load_user_conf();
						if(ret == 1)
						{
								g_sys.status.status_remap |= 0x01;         //初始化成功
						}
						else
						{
								g_sys.status.status_remap &= 0xFFFE;		  //初始化失败
						}
						break;
				}
				case(INIT_LOAD_FACT):						//load factory data
				{
						ret = init_load_factory_conf();
						if(ret == 1)
						{		
								save_conf_reg(0);
								set_load_flag(0);
								g_sys.status.status_remap |= 0x01;
						}
						else
						{
								g_sys.status.status_remap &= 0xFFFE;
						}
						break;
				}		
				case(INIT_LOAD_DEBUT):												//resotre default configuration data, include reset password to default values
				{
						ret = init_load_default();
						if(ret == 1)
						{			
								g_sys.status.status_remap |= 0x01;
							    save_conf_reg(0);
							    save_conf_reg(1);
								set_load_flag(0);
						}
						else
						{
								g_sys.status.status_remap &= 0xFFFE;						
						}
						break;
				}	
				default:												//resotre default configuration data, include reset password to default values
				{
						ret = init_load_default();
						if(ret == 1)
						{		
								g_sys.status.status_remap |= 0x01;							
						}
						else
						{
								g_sys.status.status_remap &= 0xFFFE;							
						}
						break;
				}		
		}		
		//测试模式和诊断模式复位。	
		g_sys.config.general.alarm_bypass_en = 0;	
		
		return ret;
} 









static int16_t eeprom_singel_write(uint16_t base_addr,uint16_t reg_offset_addr, uint16_t wr_data,uint16_t rd_data)
{
		int16_t err_no;
		uint16_t wr_data_buf;
		uint16_t cs_data,ee_rd_cheksum;
		wr_data_buf = wr_data;	

		err_no = I2C_EE_BufRead((uint8_t *)&cs_data,base_addr+(CONF_REG_MAP_NUM<<1),2);
		//计算check_sum
		ee_rd_cheksum = cs_data^rd_data^wr_data;
		// 写寄存器		
		err_no += I2C_EE_BufWrite((uint8_t *)&wr_data_buf,base_addr+(reg_offset_addr<<1),2);
		// 写校验
		err_no += I2C_EE_BufWrite((uint8_t *)&ee_rd_cheksum,base_addr+(CONF_REG_MAP_NUM<<1),2);
		
		return err_no;
}


int16_t eeprom_compare_read(uint16_t reg_offset_addr, uint16_t *rd_data)
{
		uint16_t rd_buf0;
		uint16_t rd_buf1;
		uint16_t rd_buf2;
		int16_t ret;
	    
		I2C_EE_BufRead((uint8_t *)&rd_buf0,CONF_REG_EE1_ADDR+(reg_offset_addr<<1),2);			//从用户区的三处读数据
		I2C_EE_BufRead((uint8_t *)&rd_buf1,CONF_REG_EE2_ADDR+(reg_offset_addr<<1),2);
		I2C_EE_BufRead((uint8_t *)&rd_buf2,CONF_REG_EE3_ADDR+(reg_offset_addr<<1),2);

		//normal situation
		if((rd_buf0 == rd_buf1)&&(rd_buf2 == rd_buf1))
		{
				if(rd_buf0 )
				*rd_data = rd_buf0;
				ret = 0;
		}
		else if((rd_buf0 == rd_buf1)||(rd_buf0 == rd_buf2)||(rd_buf1 == rd_buf2))
		{
				*rd_data = rd_buf0;
				if(rd_buf0 == rd_buf1)//buf2!= buf1
				{
						*rd_data = rd_buf0;
						eeprom_singel_write(CONF_REG_EE3_ADDR,reg_offset_addr,rd_buf0,rd_buf2);
						
				}
				else if (rd_buf0 == rd_buf2)//buf2 = buf0, buf1错
				{
						*rd_data = rd_buf2;
						eeprom_singel_write(CONF_REG_EE2_ADDR,reg_offset_addr,rd_buf2,rd_buf1);
				}
				else//(rd_buf1 == rd_buf2)
				{
						*rd_data = rd_buf1;
						eeprom_singel_write(CONF_REG_EE1_ADDR,reg_offset_addr,rd_buf1,rd_buf0);
				}	
				ret = 0;
		}	
		else//三个都错误
		{
				*rd_data = ABNORMAL_VALUE;
				ret = 1;
		}
		return (ret);
}

int16_t eeprom_tripple_write(uint16_t reg_offset_addr,uint16_t wr_data,uint16_t rd_data)
{
		int16_t err_no;
		uint16_t wr_data_buf;
		uint16_t cs_data,ee_rd_cheksum;
		wr_data_buf = wr_data;	

		err_no = eeprom_compare_read(CONF_REG_MAP_NUM, &cs_data);
		if(wr_data == rd_data)//相等，避免重复写入
		{
					return 1;
		}
		else
		{
			if(err_no == 0)
			{
					ee_rd_cheksum = cs_data^rd_data^wr_data;
			}
			else
			{
					return -1;
			}
		}
		
		err_no = 0;
		
		//write data to eeprom
		err_no += I2C_EE_BufWrite((uint8_t *)&wr_data_buf,CONF_REG_EE1_ADDR+(reg_offset_addr<<1),2);
		err_no += I2C_EE_BufWrite((uint8_t *)&wr_data_buf,CONF_REG_EE2_ADDR+(reg_offset_addr<<1),2);
		err_no += I2C_EE_BufWrite((uint8_t *)&wr_data_buf,CONF_REG_EE3_ADDR+(reg_offset_addr<<1),2);			
		
		//write checksum data to eeprom
		err_no += I2C_EE_BufWrite((uint8_t *)&ee_rd_cheksum,CONF_REG_EE1_ADDR+(CONF_REG_MAP_NUM*2),2);
		err_no += I2C_EE_BufWrite((uint8_t *)&ee_rd_cheksum,CONF_REG_EE2_ADDR+(CONF_REG_MAP_NUM*2),2);
		err_no += I2C_EE_BufWrite((uint8_t *)&ee_rd_cheksum,CONF_REG_EE3_ADDR+(CONF_REG_MAP_NUM*2),2);
		return err_no;
}

/**
  * @brief  write register map with constraints.
  * @param  reg_addr: reg map address.
  * @param  wr_data: write data. 
	* @param  permission_flag:  
  *   This parameter can be one of the following values:
  *     @arg PERM_PRIVILEGED: write opertion can be performed dispite permission level
  *     @arg PERM_INSPECTION: write operation could only be performed when pass permission check
  * @retval 
  *   This parameter can be one of the following values:
  *     @arg 1: write operation success
  *     @arg 0: write operation fail
  */
uint16_t reg_map_write(uint16_t reg_addr,uint16_t *wr_data, uint8_t wr_cnt)
{
		uint16_t i;
		uint16_t err_code;
		uint16_t ee_wr_data,ee_rd_data;
		err_code = CPAD_ERR_NOERR;
	    
		if((reg_addr+wr_cnt) > CONF_REG_MAP_NUM)	//address range check
		{
				err_code = CPAD_ERR_ADDR_OR;
				return err_code;
		}
	
		for(i=0;i<wr_cnt;i++)										//writablility check
		{
//			if(conf_reg_map_inst[reg_addr+i].rw != 1)	
			if((conf_reg_map_inst[reg_addr+i].rw != 1)&&(conf_reg_map_inst[reg_addr+i].rw != 2))	
			{
					err_code = CPAD_ERR_WR_OR;
					return err_code;	
			}						
		}		

		for(i=0;i<wr_cnt;i++)										//min_max limit check
		{
				if((*(wr_data+i)>conf_reg_map_inst[reg_addr+i].max)||(*(wr_data+i)<conf_reg_map_inst[reg_addr+i].min))		//min_max limit check
				{
						err_code = CPAD_ERR_DATA_OR;
						return err_code;	
				}
				
				if(conf_reg_map_inst[reg_addr+i].chk_ptr != NULL)
				{
						if(conf_reg_map_inst[reg_addr+i].chk_ptr(*(wr_data+i))==0)
						{
								err_code = CPAD_ERR_CONFLICT_OR;
								return err_code;	
						}
				}
		}
		
		for(i=0;i<wr_cnt;i++)														//data write
		{		
				ee_rd_data = *(conf_reg_map_inst[reg_addr+i].reg_ptr);				//buffer legacy reg data
				ee_wr_data = *(wr_data+i);															//buffer current write data
				
				*(conf_reg_map_inst[reg_addr+i].reg_ptr) = *(wr_data+i);			//write data to designated register			
				if(conf_reg_map_inst[reg_addr+i].rw == 1)//写EEPROM	
				{
			    eeprom_tripple_write(reg_addr+i,ee_wr_data,ee_rd_data);
				}
		}
		return err_code;			
}


uint8_t fan_com_chk(uint16_t pram)
{
	if(((pram <= 100)&&(pram >= 30))||(pram == 0))
	{
			return(1);
	}
	return(0);
}

