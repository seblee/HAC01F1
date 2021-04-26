#include "sys_conf.h"
#include "alarms.h"
#include "global_var.h"
#include "event_record.h"
#include "daq.h"
#include "sys_conf.h"
#include "led.h"
#include "acfrequency.h"

//************************与其他模块接口全局变量************************
	/* 告警量寄存器 */
	UCHAR g_u8AlmStatus[MAXALMID];
	
#define ACL_ENMODE_ENABLE	0x0000
#define ACL_ENMODE_SUPPRESS	0x0001//×èè?
#define ACL_ENMODE_DISABLE	0x0002

#define ACL_ENMODE_AUTO_RESET_ALARM     0X0004
#define ACL_ENMODE_HAND_RESET_ALARM     0X0000

typedef struct alarm_acl_td
{
	uint16_t				  id;
	uint16_t 					state;
	uint16_t          alram_value;
	uint16_t					timeout;
	uint16_t 					enable_mask;
	uint16_t 					reset_mask;
	uint8_t           alarm_level;
	uint16_t          dev_type;
	uint16_t (*alarm_proc)(struct alarm_acl_td* str_ptr);	
}alarm_acl_status_st;


typedef enum
{
	SML_MIN_TYPE=0,
	THR_MAX_TYPE,
	OUT_MIN_MAX_TYPE,
	IN_MIN_MAX_TYPE,	
}Compare_type_st;

enum			//òì3￡??àà±í
{
	LO_SUPER_HEAT_EXCEP,
	MOP_PROTECT_EXCEP,
	LOP_PROTECT_EXCEP,
	EEV_MOTOR_EXCEP,
	SUCTION_TEMP_FAULT,
	VAPOR_PRESSURE_FAULT,
	LOW_SUCTION_TEMP_EXCEP,	
	MAX_EXCEP_TYPES
};

typedef struct
{
	alarm_acl_status_st alarm_sts[ACL_TOTAL_NUM];
	
}alarm_st;


volatile uint8_t timeout[MAX_EEVNUM][MAX_EXCEP_TYPES];
static alarm_st alarm_inst;
extern	sys_reg_st		g_sys; 


static void   alarm_status_bitmap_op (uint8_t alarm_id,uint8_t option);


//?ì2aoˉêy
static  uint16_t acl00(alarm_acl_status_st* acl_ptr);						//ACL_ACFREQUENCYALM
static	uint16_t acl01(alarm_acl_status_st* acl_ptr);           //ACL_PSENSORALM
static	uint16_t acl02(alarm_acl_status_st* acl_ptr);           //ACL_COMM
static	uint16_t acl03(alarm_acl_status_st* acl_ptr);           //ACL_EEPROMALM

void exceptionHandle(uint8_t index);
extern uint16_t do_set(int16_t pin_id, BitAction value);

enum
{
		ALARM_ACL_ID_POS=			0,//???ˉDòo?
		ALARM_ACL_EN_MASK_POS		,	//?ìμ??÷????
		ALARM_ACL_RESET_POS     ,//?a3y·?ê?
		AlARM_ACL_LEVEL_POS     ,//???ˉμè??
		ALARM_ACL_DEV_POS      ,//???ˉààDí
		ALARM_ACL_MAX
};


#define ALARM_FSM_INACTIVE			0x0001	
#define ALARM_FSM_PREACTIVE			0x0002
#define ALARM_FSM_ACTIVE				0x0003	
#define ALARM_FSM_POSTACTIVE		0x0004	
#define ALARM_FSM_ERROR					0x0005	

#define ALARM_ACL_TRIGGERED			0x0001
#define ALARM_ACL_CLEARED			  0x0000
#define ALARM_ACL_HOLD          0x0002





//uint16_t alarm_tem_erro,alarm_hum_erro;

static uint16_t (* acl[ACL_TOTAL_NUM])(alarm_acl_status_st*) = 
{
	
//??·?oí?í·?±¨?ˉ(???èoíêa?è)
		acl00,		
		acl01,		
		acl02,	
		acl03,			
};

#define DEV_TYPE_COMPRESSOR 0x0000
#define DEV_TYPE_FAN        0x0400
#define DEV_TYPE_OUT_FAN    0x0800
#define DEV_TYPE_HEATER     0x0c00
#define DEV_TYPE_HUM        0x1000
#define DEV_TYPE_POWER      0x1400
#define DEV_TYPE_TEM_SENSOR 0x1800
#define DEV_TYPE_WATER_PUMP 0x1c00
#define DEV_TYPE_OTHER      0x3c00


const uint16_t ACL_CONF[ACL_TOTAL_NUM][ALARM_ACL_MAX]=
//	id ,en_mask,reless_mask,DEV_type
{		
	    {0,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER},       			//ACL_ACFREQUENCYALM 
			{1,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER},           //ACL_PSENSORALM,	
			{2,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER},           //ACL_COMM,		
			{3,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER},           //ACL_EEPROMALM,		
};
/*
  * @brief  alarm data structure initialization
	* @param  none
  * @retval none
  */

static void init_alarm(alarm_st* alarm_spr) 
{
	uint16_t i;
	//3?ê?ACL 
	for(i=0;i<ACL_TOTAL_NUM;i++)
	{
		alarm_spr->alarm_sts[i].timeout =  0;
		alarm_spr->alarm_sts[i].state = ALARM_FSM_INACTIVE;
		alarm_spr->alarm_sts[i].id = ACL_CONF[i][ALARM_ACL_ID_POS];
		alarm_spr->alarm_sts[i].enable_mask = ACL_CONF[i][ALARM_ACL_EN_MASK_POS];
		alarm_spr->alarm_sts[i].reset_mask = ACL_CONF[i][ALARM_ACL_RESET_POS];
		alarm_spr->alarm_sts[i].alarm_level = ACL_CONF[i][AlARM_ACL_LEVEL_POS];
		alarm_spr->alarm_sts[i].dev_type = ACL_CONF[i][ALARM_ACL_DEV_POS];
		alarm_spr->alarm_sts[i].alarm_proc = acl[i];
		alarm_spr->alarm_sts[i].alram_value=0xffff;
	}
}

void alarm_acl_init(void)
{
//	uint8_t i;
	chain_init();
	init_alarm(&alarm_inst);
	//初始化手动解除报警
//	for(i=0;i<ALARM_TOTAL_WORD;i++)
//	{
			g_sys.config.general.alarm_remove_bitmap = 0;
//	}
} 
 
uint8_t clear_alarm(void)
{
	g_sys.config.general.alarm_remove_bitmap = 0xffff;
	return 1;		
}

static uint8_t get_alarm_remove_bitmap(uint8_t alarm_id)
{
	if(alarm_id < ACL_TOTAL_NUM)
	{
		if((g_sys.config.general.alarm_remove_bitmap >> alarm_id) & 0x0001 )
		{
				return(1);
		}
		else
		{
				return(0);
		}
	}
	else
	{
			return(0);
	}
}

static void clear_alarm_remove_bitmap(uint8_t alarm_id)
{
		if(alarm_id < ACL_TOTAL_NUM)
		{
				g_sys.config.general.alarm_remove_bitmap &=(~(0x0001 << alarm_id));
		}
	
}

//告警处理
static void alarm_arbiration(void)
{
	static uint8_t su8Flash[3]={0};
	if(g_sys.status.alarm_bitmap)
	{
		if((g_sys.status.alarm_bitmap&ACL_ACF)==ACL_ACF)//闪烁1次
		{
			if(++su8Flash[0] >= 5)
			{
				su8Flash[0] = 0;
				led_toggle(LED_ALARM);	
			}			
		}			
		else
		if((g_sys.status.alarm_bitmap&ACL_PSE)==ACL_PSE)//闪烁2次
		{
			if(++su8Flash[1] >= 3)
			{
				su8Flash[1] = 0;
				led_toggle(LED_ALARM);	
			}		
		}	
		else
		if((g_sys.status.alarm_bitmap&ACL_ACOM)==ACL_ACOM)//闪烁5次
		{
				led_toggle(LED_ALARM);	
		}
		else
		{
			led_open(LED_ALARM);						
		}
	}
	else
	{
		led_close(LED_ALARM);			
	}
	
	return;
}
void alarm_acl_exe_process(void)
{	
	static uint8_t num = 0;
	if(++num >= 10)
	{
		num = 0;
		alarm_acl_exe(0);		
	}
	//告警处理
	alarm_arbiration();
	return;
}

void alarm_acl_exe(uint8_t index)
{
	extern	sys_reg_st		g_sys;
	uint16_t acl_trigger_state;	
	uint16_t i;
	uint16_t c_state;
	uint16_t log_id;

    for(i=0;i<ACL_TOTAL_NUM;i++)	
		{
				//if acl disabled, continue loop
			
				if(((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)&&(alarm_inst.alarm_sts[i].state == ALARM_FSM_INACTIVE))
				{
						continue;				
				}
			
				acl_trigger_state = acl[i](&alarm_inst.alarm_sts[i]);
				c_state = alarm_inst.alarm_sts[i].state;	
				log_id = alarm_inst.alarm_sts[i].id|(alarm_inst.alarm_sts[i].alarm_level<<8)|alarm_inst.alarm_sts[i].dev_type;
				switch (c_state)
				{
						case(ALARM_FSM_INACTIVE):
						{
							  if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
										alarm_inst.alarm_sts[i].timeout = g_sys.config.alarm[i].delay;
//										alarm_inst.alarm_sts[i].timeout = Alarm_acl_delay(i);
										alarm_inst.alarm_sts[i].state = ALARM_FSM_PREACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
								}
								else
								{
										;
								}
								
								break;
						}
						case(ALARM_FSM_PREACTIVE):
						{
									 //状态机回到 ALARM_FSM_INACTIVE 状态
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
										alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;		
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
										
									
									 if(alarm_inst.alarm_sts[i].timeout > 0)
										{ 
												alarm_inst.alarm_sts[i].timeout --;
												alarm_inst.alarm_sts[i].state = ALARM_FSM_PREACTIVE;
										}
										else
										{
												alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
												//
												add_alarmlog_fifo(log_id,ALARM_TRIGER,alarm_inst.alarm_sts[i].alram_value);
												//TEST
												alarm_inst.alarm_sts[i].id=i;
												alarm_status_bitmap_op(alarm_inst.alarm_sts[i].id&0x00ff,1);
												node_append(log_id,alarm_inst.alarm_sts[i].alram_value);
											
										}
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;		
								}							
								break;
						}
						case(ALARM_FSM_ACTIVE):
						{
							 //状态机回到 ALARM_FSM_INACTIVE 状态
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
										alarm_inst.alarm_sts[i].timeout = 0;
									  alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;		
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{					
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_CLEARED)
								{	
									//自动解除报警
									if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].reset_mask) == ACL_ENMODE_AUTO_RESET_ALARM)
									{
										alarm_inst.alarm_sts[i].timeout = g_sys.config.alarm[i].delay;
//										alarm_inst.alarm_sts[i].timeout = Alarm_acl_delay(i);
										
										alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;	
									}
									else
									{
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
									}
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
									;
								}
								break;
						}
						case(ALARM_FSM_POSTACTIVE):
						{
								 //状态机回到 ALARM_FSM_INACTIVE 状态
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
											add_alarmlog_fifo(log_id,ALARM_END,alarm_inst.alarm_sts[i].alram_value);												
									     //删除状态节点
											node_delete(log_id);
											alarm_inst.alarm_sts[i].id=i;
											alarm_status_bitmap_op(alarm_inst.alarm_sts[i].id&0x00ff,0);
												
											alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
											alarm_inst.alarm_sts[i].timeout = 0;
								}
								else if(acl_trigger_state == ALARM_ACL_CLEARED)//yxq
								{
										
										if(alarm_inst.alarm_sts[i].timeout > 0)
										{
												alarm_inst.alarm_sts[i].timeout --;
												alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;
										}
										else
										{
												
												add_alarmlog_fifo(log_id,ALARM_END,alarm_inst.alarm_sts[i].alram_value);
//												
//												//删除状态节点
												alarm_status_bitmap_op(i,0);
												node_delete(log_id);
											
												
												alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
										}
												
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
									;
								}
								break;
						}
						default://
							{
								alarm_inst.alarm_sts[i].state=ALARM_FSM_INACTIVE;
								
								break;
							}
				}
		}
//	alarm_arbiration();
	return;
}

//??è?±¨?ˉ??
uint8_t get_alarm_bitmap(uint8_t alarm_id)
{
	if((g_sys.status.alarm_bitmap >>alarm_id) & 0x0001)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

//???ˉ×′ì???
static void alarm_status_bitmap_op (uint8_t alarm_id,uint8_t option)
{
	if(option == 1)
	{
		g_sys.status.alarm_bitmap |= (0x0001<<alarm_id);
	}
	else
	{
		g_sys.status.alarm_bitmap &= ~(0x0001<<alarm_id);
	}
}

static  uint8_t acl_clear(alarm_acl_status_st* acl_ptr)
{

	return (ALARM_ACL_CLEARED);	
}

//ACL_ACFREQUENCYALM
static uint16_t acl00(alarm_acl_status_st* acl_ptr)
{
	if(acl_clear(acl_ptr))
	{
		return(ALARM_ACL_CLEARED);
	}
	//频率异常告警判断
			g_sys.status.Test_Buff[25]=g_u8AcFrequencyStatus;
			g_sys.status.Test_Buff[24]++;
//	if(g_u8AcFrequencyStatus & 0x01)
	if(g_u8AcFrequencyStatus)
	{
		g_u8AlmStatus[ACFREQUENCYALM] = ALARM;
		return (ALARM_ACL_TRIGGERED);
	}
	else
	{
		g_u8AlmStatus[ACFREQUENCYALM] = NORMAL;
		return (ALARM_ACL_CLEARED);
	}
}

//ACL_PSENSORALM//压力传感器异常
static uint16_t acl01(alarm_acl_status_st* acl_ptr)
{
	static uint8_t	su8ASdelay=0;
	if(acl_clear(acl_ptr))
	{
		return(ALARM_ACL_CLEARED);
	}
	if(g_sys.config.g_u8CfgParameter[CFGCOMMODE])
	{
		return(ALARM_ACL_CLEARED);		
	}
	if((g_sys.status.ain[AI_SENSOR1]<=20)||(g_sys.status.ain[AI_SENSOR1]==ABNORMAL_VALUE)||(g_sys.status.ain[AI_SENSOR1]>MAX_PRESS_MEAVAL))
	{
			su8ASdelay=1;
	}
	else
	{
			su8ASdelay=0;
	}
			g_sys.status.Test_Buff[27]=su8ASdelay;
	if(su8ASdelay)
	{
//		su8ASdelay=2;
		g_u8AlmStatus[PSENSORALM] = ALARM;
		return (ALARM_ACL_TRIGGERED);
	}
	else
	{
		g_u8AlmStatus[PSENSORALM] = NORMAL;
		return (ALARM_ACL_CLEARED);
	}
}
//ACL_COMMON
static  uint16_t acl02(alarm_acl_status_st* acl_ptr)
{
	if(acl_clear(acl_ptr))
	{
		return(ALARM_ACL_CLEARED);
	}
	if(g_sys.config.g_u8CfgParameter[CFGCOMMODE])
	{
		if(g_sys.status.Com_error[1]>=g_sys.config.alarm[ACL_COMMON].alarm_param*10)
		{
			return (ALARM_ACL_TRIGGERED);			
		}
		if(g_sys.status.Com_error[1]==0)
		{
			return (ALARM_ACL_CLEARED);			
		}		
	}
	return (ALARM_ACL_CLEARED);
}
//ACL_EEPROMALM
static  uint16_t acl03(alarm_acl_status_st* acl_ptr)
{
	return (ALARM_ACL_CLEARED);
}


























