#ifndef __SYS_CONF
#define	__SYS_CONF

#include "sys_def.h"
#include "alarms.h"
#include "string.h"
#include "adc.h"
#include <stm32f0xx.h>

#define CORE_PROC_DLY		100u
#define COM_PROC_DLY    10u
//#define BKG_PROC_DLY		500u
#define BKG_PROC_DLY		100u

//cpad err code
#define CPAD_ERR_NOERR					0
#define CPAD_ERR_ADDR_OR				1
#define CPAD_ERR_DATA_OR				2
#define CPAD_ERR_PERM_OR				3
#define CPAD_ERR_WR_OR					4
#define CPAD_ERR_CONFLICT_OR    5

#define TRUE            1
#define FALSE           0

enum
{
	AI_SENSOR_INDEX1 = 0,
	AI_SENSOR_INDEX2,
	AI_SENSOR_NUM
};

enum
{
	AI_NTC_INDEX1 = 0,
	AI_NTC_INDEX2,
	AI_NTC_NUM
};

enum
{
	AUTO_MODE = 0,
	MAN_MODE,//手动模式
};

enum
{
	SYNC_MODE = 0,//同步
	ALONE_MODE,		//独立
};

enum
{
	BIT0 = 0x01,//手动模式
	BIT1 = 0x02,//同步
};

enum
{
	PID_ALOGORITHM = 0,
	STEP_ALOGORITHM
};

enum		//控制状态
{
	STATUS_INIT = 0,		     //初始化
	STATUS_CLOSE,        //关闭
	STATUS_OFF,         //待机
	STATUS_POS,         //定位
	STATUS_WAIT,        //等待
    STATUS_PRE_ADJUST,  //预调节
	STATUS_ON,			//运行
	STATUS_HOLD
};

//采集取样
enum
{
	GET_ZERO=0,
	GET_ONE,
	GET_TWO,
	GET_AVG
};

enum
{
	EEV1 = 0,
	EEV2,
	MAX_EEVNUM
};

enum
{
	VALVE_OFF = 0,
	VALVE_ON
};

enum
{
	NO_ACTION = 0,
	FORCE_CLOSE_VALVE,    		//强制关闭阀门
	VALVE_OPEN_FIXED_POS		//阀开启至固定位置
};    //@2017-08-21 

#define ALARM_TOTAL_WORD 1
typedef struct
{		
	uint16_t		Cool_Type;				//制冷类型
	uint16_t    refrigType;			    //制冷剂类型
	
	uint16_t		alarm_bypass_en;		//取消告警使能
	uint16_t		Dehumidity_Supperheart;		//除湿模式过热度
	uint16_t		alarm_remove_bitmap;	//告警移除
	
	uint16_t		ntc_cali[AI_NTC_NUM];	//NTC校准值
	uint16_t		ai_cali[AI_SENSOR_NUM];  //传感器校准值
	
	uint16_t 		surv_baudrate;			//
	uint16_t 		surv_addr;				//
	
//   uint16_t        super_heat[MAX_EEVNUM];	//过热度的设定值
//   uint16_t        valve_action_steps_alarm;			    //报警时开/关的阀开度(5%)
//    uint16_t      enable_valve_pos_manual;		//手动阀定位使能
//    uint16_t      valve_pos_manual_comm[MAX_EEVNUM];			//手动设置阀位置(通信)
//    uint16_t      valve_pos_manual_alarm;			//手动设定阀位置(报警)     //@2017-08-21
//    uint16_t      valve_opened_Standby;			//待机时阀开启状态
  uint16_t    minVol;				//最小电压
  uint16_t    maxVol;				//最大电压
	uint16_t		pressMAXBar;            //压力传感器量程最大值
	uint16_t		pressMINBar;            //压力传感器量程最小值
//	uint16_t    pressfactId;		    //压力传感器工厂代码
//	uint16_t    pressType;				//压力传感器类型(电流或电压型)   
  uint16_t    refVol;				//参考电压
//	uint16_t 	  eevfactId;				//电子膨胀阀工厂代码
//	uint16_t    eevproType;				//电子膨胀阀类型
//	uint16_t    excSpeed;			    //励磁速度 pps
//	
//	uint16_t    excAllOpenSteps;		//全开脉冲
//	uint16_t    excOpenValveMinSteps;   //阀开脉冲--最小步数      //@2017-08-21
//	uint16_t    excOpenValveMinDegree;  //关阀时阀开度的最小极限值
//	uint16_t    eevHoldTime;				//unit 100ms      
	uint16_t    restore_factory_setting;
}conf_general_st;

typedef struct 
{
	uint16_t  ain;
	uint16_t  din;
	uint16_t  dout;
	uint16_t  eev;							//电子膨胀阀
}dev_mask_st;

typedef struct
{
	uint16_t     eev_ctrl_mode;			    //电子膨胀阀的控制方式
	uint16_t     prop_gain;					//PID算法的设定 比例增益
	uint16_t     integ_time;			    //积分时间
	uint16_t     diff_time;					//微分时间
	uint16_t     samp_time;
	uint16_t     dead_zone;
	uint16_t     pre_adjust_time;		//为了平衡压力
} algorithm_st;

typedef struct
{
	uint16_t 	  id;              //告警编号
	uint16_t	  delay;           //告警激活延迟参数
	uint16_t	  enable_mode;    //告警模式
	uint16_t 	  alarm_param;	   //告警阈值
	uint16_t      alarm_param2;		
	uint16_t      alarm_mange_resp; 	//驱动器对传感器警报的反应    //@2017-08-21
}alarm_acl_conf_st;

//Config parameters array index definition
typedef enum _CHARCFGPARAMETERID_
{
	CFGYEAR,		//出厂年	缺省08
	CFGMONTH,		//出厂月	缺省01
	CFGDAY,			//出厂日	缺省01

	CFGPSETID,		/* 冷凝压力调节最小值，单位为bar, 缺省13，范围 11～15 */
	CFGPBANDID,		/* 冷凝压力调节范围，单位为bar, 缺省 5， 范围 4～6 */
	CFGMINVID,		/* 风机供电电压最小值，单位为输入电压的百分比, 缺省30，范围 30～50 */
	CFGMAXVID,		/* 风机供电电压最大值，单位为输入电压的百分比, 缺省100，范围 60～100 */
	CFGFANNUMID,		//风机数量， 范围1～2，缺省1。
	CFGPSENSORTYPEID,	//压力传感器类型，缺省 1，1：电压型;2：电流型
	CFGCOMMODE,		//通信控制， 范围0～1，缺省0，0:压力；1-通信给定。
	CFGCOMFANOUT,	//通信输出，范围30～100，缺省0。
	
	CFG_CDATA_LEN
}CHARCFGPARAMETERIDINDEX;

//system configuration
typedef struct
{
	conf_general_st 			 general;
	uint16_t               g_u8CfgParameter[CFG_CDATA_LEN];
	dev_mask_st						dev_mask;
	algorithm_st					algorithm;
  alarm_acl_conf_st		alarm[ACL_TOTAL_NUM];
}config_st;

//system information
typedef struct
{
	uint16_t	 			status_reg_num;													
	uint16_t	 			config_reg_num;
	uint16_t	 			software_ver;
	uint16_t	 			hardware_ver;
	uint16_t	 			serial_no[4];
	uint16_t	 			man_date[2];
}sys_info_st;

typedef struct
{
	uint16_t 		permission_level;		//user authentication level
	//uint16_t 		running_mode;			//automatic, manual or testing
	uint16_t 		sys_error_bitmap;		//system error status
}status_general_st;

typedef struct
{
		uint16_t critical_cnt;
		uint16_t major_cnt;
		uint16_t mioor_cnt;
		uint16_t total_cnt;
}alarm_state_cnt_st;

typedef struct
{
	uint16_t vapor_pressure[AI_SENSOR_NUM][GET_AVG + 1];
	uint16_t vapor_pressure_counter[AI_SENSOR_NUM];
	uint16_t vapor_temp[AI_SENSOR_NUM][GET_AVG + 1];
	uint16_t vapor_temp_counter[AI_SENSOR_NUM];
	uint16_t suction_temp[AI_NTC_NUM][GET_AVG + 1];
	uint16_t suction_temp_counter[AI_NTC_NUM];
	uint16_t superHeat[MAX_EEVNUM];
}environmen_st;

typedef struct
{
	uint16_t dir;			 //当前周期增加步数
	uint16_t inc;			 //当前周期增加步数
}valve_cur;

typedef struct
{
	uint16_t valve_opening_cur[MAX_EEVNUM];		//计算出来的阀开度
	uint16_t valve_steps_cur[MAX_EEVNUM];				//当前绝对阀步数
	valve_cur cur_steps[MAX_EEVNUM];			 //当前周期增加步数
	uint16_t valve_phase[MAX_EEVNUM];				//阀相位
	uint16_t valve_ctrl_signal[MAX_EEVNUM];	    //阀启停信号
	uint16_t valve_ctrl_status[MAX_EEVNUM];			//控制状态
}valve_st;

	typedef enum _P_RANGE_POINT_
	{
		P_SET_POINT,		//压力下限设置点
		P_OFF_POINT,		//关闭电压输出压力点：下限设置点减1BAR
		P_FLAT_POINT,		//进入平台区的压力
		P_FLAT2_POINT,		//恢复最大设定输出电压的压力，平台区加3BAR
		P_STEP_POINT,		//进入阶跃区的压力
		P_STEP_BAND,		//调节范围
		MAXPOINTNUM
	} P_RANGE_POINT;
	
typedef struct
{
	uint16_t u16FanVout100;		//实际输出
	uint16_t Fan_out;		//计算出来的风机输出
	uint16_t u16PressuePoint[MAXPOINTNUM]; //放大100倍的有关压力参数
	uint16_t ACCye[3];		//风机周期，斩波时间
  uint16_t u16Runtime[2][2];  //使用时间
}Fan_st;//风机状态

#define COMERR_5S		5	

typedef struct
{
		sys_info_st					sys_info;
		status_general_st			general;												//3													//25
		uint16_t 					ain[AI_MAX_CNT];										//10
		uint16_t					din_bitmap[2];									//1
		uint16_t					dout_bitmap;									//1
		uint16_t					status_remap;								//4
		uint16_t					alarm_bitmap;								//6	
		alarm_state_cnt_st          alarm_status_cnt;
//		environmen_st			    environmen;
		Fan_st						Fan;
		uint16_t					work_mode;	  
		uint16_t					Com_error[2];	   	
		uint16_t					Cal_Time;	  	
		uint16_t					Test_Buff[35];									//TEST
}status_st;

typedef struct
{
	config_st config;
	status_st status;	
}sys_reg_st;

typedef struct 
{
	uint16_t 	id;
	uint16_t*	reg_ptr;
	uint16_t	min;
	uint16_t	max;
	uint16_t	dft;	
	uint8_t		permission;
	uint8_t		rw;
	uint8_t(*chk_ptr)(uint16_t pram);
}conf_reg_map_st;

#define ABNORMAL_VALUE    0x7FFF//异常值
#define ABNORMAL_VALUE2    0//异常值
typedef struct 
{
		uint16_t 	id;
		uint16_t*	reg_ptr;
		uint16_t	dft;
		//uint8_t		rw;
}sts_reg_map_st;

//enum
//{
//		COOL_TYPE_MODULE_WIND=0,//常规风冷
//		COOL_TYPE_MODULE_WATER,//常规冷冻水
//		COOL_TYPE_MODULE_MIX,
//		COOL_TYPE_COLUMN_WIND,//列间风冷
//		COOL_TYPE_COLUMN_WATER,//列间冷冻水
//		COOL_TYPE_HUMIDITY=9,//恒湿机
//};

enum
{
		ALARM_COM = 0,
		ALARM_NTC,
};

enum				//机组工作状态
{		
		PWR_STS_BPOS = 0,		        //开关机
		FAN_STS_BPOS,                   //风机
		HEATING_STS_BPOS,				//制热
		COOLING_STS_BPOS,				//制冷
		HUMING_STS_BPOS,				//加湿
		DEMHUM_STS_BPOS,				//除湿
		COOL_VALVE_BPOS,				//水阀
		TEAM_STANDALONE_STS_BPOS =8,	//单机/群控
		TEAM_STS_BPOS,					//群控正常/失败
		ALARM_STUSE_BPOS =14,           //告警状态位
		ALARM_BEEP_BPOS =15             //告警音标志位
};

enum
{
		HUM_RELATIVE=0,
		HUM_ABSOLUTE		
};

//compressor
typedef struct
{
	uint16_t type;
	uint16_t dehum_level;
	uint16_t startup_delay;
	uint16_t stop_delay;
	uint16_t min_runtime;
	uint16_t min_stoptime;
	uint16_t startup_lowpress_shield;
	uint16_t alter_mode;
	uint16_t alter_time;
	uint16_t start_interval;
	uint16_t ev_ahead_start_time;
	uint16_t ev_Delay_shut_time;
	uint16_t speed_upper_lim;	
	uint16_t speed_lower_lim;		
	uint16_t ec_comp_start_req;	
  uint16_t startup_freq;	
  uint16_t high_press_threshold;
  uint16_t high_press_hyst;
  uint16_t ret_oil_period;
  uint16_t ret_oil_freq;
  uint16_t low_freq_switch_period;
  uint16_t low_freq_threshold;
  uint16_t step;
  uint16_t step_period;
  uint16_t Exhaust_temperature;//排气温度
  uint16_t Exhaust_temperature_hystersis;//排气温度回差
}compressor_st;

//降频
enum
{
		HIGH_PRESS=0,
		EXHAUST_TEMP,
};



typedef struct
{
	char inv_hipress_flag;
	char inv_hipress_tmp;
	char inv_hipress_stop_flag;
	uint32_t inv_start_time[3];
	uint32_t inv_stop_time[3];
	uint16_t inv_alarm_counter;
	uint16_t avg_hi_press[4];//高压压力
	uint8_t counter_hipress;
	uint8_t counter_hi_Temperature;
	uint16_t Inv_hi_Temperature[4];//排气温度
	uint8_t Inv_hi_temp_Flag;//排气温度降频标识
	uint8_t Inv_hi_temp_Count;//排气温度计数
	uint8_t Inv_hi_temp_Stop;//排气温度停机
	uint32_t Inv_hi_temp_Starttime[3];//排气温度降频时间
	uint32_t Inv_hi_temp_Stoptime[3];//排气温度降频恢复时间
}inv_compress_alarm_st;

//compressor
typedef struct
{
	uint16_t auto_mode_en;
	uint16_t max_opening;
	uint16_t min_opening;
	uint16_t set_opening;
	uint16_t start_req;
	uint16_t mod_priority;
	uint16_t action_delay;
	uint16_t temp_act_delay;
  	uint16_t trace_mode;
  	uint16_t act_threashold;
}watervalve_st;

//humidifier 

// dehum_dev
typedef struct
{
	uint16_t stop_dehum_temp;
	uint16_t Delay;	//风机转换延时,Alair,20161113
}dehum_st;

///////////////////////////////////////////////////////////////
//system status 
///////////////////////////////////////////////////////////////

typedef struct
{
	uint32_t	din_data;
	uint32_t	din_mask;
}din_st;


///////////////////////////////////////////////////////////////
//system output status 
///////////////////////////////////////////////////////////////

typedef struct
{
	int16_t comp1_runtime_day;
	int16_t comp1_runtime_min;
	int16_t comp2_runtime_day;
	int16_t comp2_runtime_min;
	int16_t fan1_runtime_day;
	int16_t fan1_runtime_min;
	int16_t fan2_runtime_day;
	int16_t fan2_runtime_min;
	int16_t fan3_runtime_day;
	int16_t fan3_runtime_min;
	int16_t heater1_runtime_day;
	int16_t heater1_runtime_min;
	int16_t heater2_runtime_day;
	int16_t heater2_runtime_min;
	int16_t humidifier_runtime_day;	
	int16_t humidifier_runtime_min;
}sys_runtime_log_st;


///////////////////////////////////////////////////////////////
//alarms definition
///////////////////////////////////////////////////////////////

//alarms: acl definition
/*
@id:			alarm id
@delay:		trigger&clear delay 
@timeout:	delay timeout count down
@trigger_time:	alarm trigger time
@enable mode:	alarm enable mode
`0x00:		enable
`0x01:		suspend
`0x02:		forbid
@enable mask:	alarm enable mask
'0x03:	all mode enable
'0x02:	enable or forbid 
'0x01:	enable or suspend
'0x00:	only enable
@alarm_param:	related paramter(eg. threshold)
@void (*alarm_proc): designated alarm routine check function
*/
//typedef struct alarm_acl_td
//{
//	uint16_t 					id;
//	uint16_t					delay;
//	uint16_t 					timeout;
//	uint16_t 					state;
//	time_t						trigger_time;
//	uint16_t					enable_mode;
//	uint16_t					enable_mask;
//	uint16_t 					alarm_param;
//	uint16_t (*alarm_proc)(struct alarm_acl_td* str_ptr);	
//}alarm_acl_st;

//typedef struct alarm_acl_td
//{
//	uint16_t 					id;
//	uint16_t 					state;
//	time_t						trigger_time;
//	uint16_t (*alarm_proc)(struct alarm_acl_td* str_ptr);	
//}alarm_acl_status_st;

//team struct definition
//team status
/*
@temp:	sampled temperature
@hum:		sampled humidity
@alarm_sts:
	`bit0:cri_alarm:	critical alarm, which will cause online set to go offline
		0:	no alarm
		1:	alarm active
	`bit1:norm_alarm:	sampled temperature
		0:	no alarm
		1:	alarm active
	`bit2:hitemp_alarm:
		0:	no alarm
		1:	alarm active
	`bit7:alarm_flag:
		0:	no alarm
		1:	alarm active
@comp_num:
	`compressor_num:	number of compressors
	`heater_num:			number of heaters
@run_state:
	0:	standby
	1:	running
@offline_count:
	0:	standby
	1:	running
*/

//team configuration
/*
@temp_set:							set temperature
@temp_precision_set:		set temperature precisiton
@temp_deadband_set:			set temperature deadband
@hum_set:								set humidity
@hum_precision_set:			set humidity precisiton
@hum_deadband_set:			set humidity deadband
@hitemp_alarm_set:			set high temperature alarm threshold
@lotemp_alarm_set:			set low temperature alarm threshold
@hihum_alarm_set:				set high humidity alarm threshold
@lohum_alarm_set:				set low humidity alarm threshold
*/
typedef struct {
	int16_t 					temp_set;
	int16_t						temp_precision_set;
	int16_t	 					temp_deadband_set;
	int16_t 					hum_set;
	int16_t						hum_precision_set;
	int16_t	 					hum_deadband_set;
	int16_t	 					hitemp_alarm_set;
	int16_t	 					lotemp_alarm_set;
	int16_t	 					hihum_alarm_set;
	int16_t	 					lohum_alarm_set;
}team_param_st;

typedef struct {
	int16_t 					temp_req;
	int16_t						hum_req;
	int8_t	 					temp_ctrl_mode;
	int8_t	 					hum_ctrl_mode;
	int8_t	 					team_mode;
	int8_t						run_enable;
}team_req_st;

//system memory map
//system memory configuration map
typedef struct sys_conf_map
{
	int16_t 					id;
	void*						str_ptr;
	int16_t						length;
}sys_conf_map_st;


typedef struct sys_status_map
{
	int16_t 					id;
	int16_t	*					addr;
	int16_t						length;
}sys_status_map_st;

typedef struct
{
	uint16_t mioor_alarm_tmp;
	uint16_t critical_alarm_tmp;
}ext_tmp_st;

typedef struct
{
		uint16_t 			dev_sts;
		uint16_t 			conductivity;
		uint16_t 			hum_current;
		uint16_t 			water_level;
}mbm_hum_st;

//modbus master data structure
typedef struct
{
	uint16_t 			dev_sts;
	uint16_t      di_bit_map;
	uint16_t      press;
	uint16_t      temp;
	uint16_t      fan_speed;
	uint16_t      do_bit_map_r;
	uint16_t      compress_speed_r;
	uint16_t      do_bit_map_w;
	uint16_t      compress_speed_w;
}mbm_outsidec_st;

#endif //	__SYS_CONF


