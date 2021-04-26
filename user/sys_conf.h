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
	MAN_MODE,//�ֶ�ģʽ
};

enum
{
	SYNC_MODE = 0,//ͬ��
	ALONE_MODE,		//����
};

enum
{
	BIT0 = 0x01,//�ֶ�ģʽ
	BIT1 = 0x02,//ͬ��
};

enum
{
	PID_ALOGORITHM = 0,
	STEP_ALOGORITHM
};

enum		//����״̬
{
	STATUS_INIT = 0,		     //��ʼ��
	STATUS_CLOSE,        //�ر�
	STATUS_OFF,         //����
	STATUS_POS,         //��λ
	STATUS_WAIT,        //�ȴ�
    STATUS_PRE_ADJUST,  //Ԥ����
	STATUS_ON,			//����
	STATUS_HOLD
};

//�ɼ�ȡ��
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
	FORCE_CLOSE_VALVE,    		//ǿ�ƹرշ���
	VALVE_OPEN_FIXED_POS		//���������̶�λ��
};    //@2017-08-21 

#define ALARM_TOTAL_WORD 1
typedef struct
{		
	uint16_t		Cool_Type;				//��������
	uint16_t    refrigType;			    //���������
	
	uint16_t		alarm_bypass_en;		//ȡ���澯ʹ��
	uint16_t		Dehumidity_Supperheart;		//��ʪģʽ���ȶ�
	uint16_t		alarm_remove_bitmap;	//�澯�Ƴ�
	
	uint16_t		ntc_cali[AI_NTC_NUM];	//NTCУ׼ֵ
	uint16_t		ai_cali[AI_SENSOR_NUM];  //������У׼ֵ
	
	uint16_t 		surv_baudrate;			//
	uint16_t 		surv_addr;				//
	
//   uint16_t        super_heat[MAX_EEVNUM];	//���ȶȵ��趨ֵ
//   uint16_t        valve_action_steps_alarm;			    //����ʱ��/�صķ�����(5%)
//    uint16_t      enable_valve_pos_manual;		//�ֶ�����λʹ��
//    uint16_t      valve_pos_manual_comm[MAX_EEVNUM];			//�ֶ����÷�λ��(ͨ��)
//    uint16_t      valve_pos_manual_alarm;			//�ֶ��趨��λ��(����)     //@2017-08-21
//    uint16_t      valve_opened_Standby;			//����ʱ������״̬
  uint16_t    minVol;				//��С��ѹ
  uint16_t    maxVol;				//����ѹ
	uint16_t		pressMAXBar;            //ѹ���������������ֵ
	uint16_t		pressMINBar;            //ѹ��������������Сֵ
//	uint16_t    pressfactId;		    //ѹ����������������
//	uint16_t    pressType;				//ѹ������������(�������ѹ��)   
  uint16_t    refVol;				//�ο���ѹ
//	uint16_t 	  eevfactId;				//�������ͷ���������
//	uint16_t    eevproType;				//�������ͷ�����
//	uint16_t    excSpeed;			    //�����ٶ� pps
//	
//	uint16_t    excAllOpenSteps;		//ȫ������
//	uint16_t    excOpenValveMinSteps;   //��������--��С����      //@2017-08-21
//	uint16_t    excOpenValveMinDegree;  //�ط�ʱ�����ȵ���С����ֵ
//	uint16_t    eevHoldTime;				//unit 100ms      
	uint16_t    restore_factory_setting;
}conf_general_st;

typedef struct 
{
	uint16_t  ain;
	uint16_t  din;
	uint16_t  dout;
	uint16_t  eev;							//�������ͷ�
}dev_mask_st;

typedef struct
{
	uint16_t     eev_ctrl_mode;			    //�������ͷ��Ŀ��Ʒ�ʽ
	uint16_t     prop_gain;					//PID�㷨���趨 ��������
	uint16_t     integ_time;			    //����ʱ��
	uint16_t     diff_time;					//΢��ʱ��
	uint16_t     samp_time;
	uint16_t     dead_zone;
	uint16_t     pre_adjust_time;		//Ϊ��ƽ��ѹ��
} algorithm_st;

typedef struct
{
	uint16_t 	  id;              //�澯���
	uint16_t	  delay;           //�澯�����ӳٲ���
	uint16_t	  enable_mode;    //�澯ģʽ
	uint16_t 	  alarm_param;	   //�澯��ֵ
	uint16_t      alarm_param2;		
	uint16_t      alarm_mange_resp; 	//�������Դ����������ķ�Ӧ    //@2017-08-21
}alarm_acl_conf_st;

//Config parameters array index definition
typedef enum _CHARCFGPARAMETERID_
{
	CFGYEAR,		//������	ȱʡ08
	CFGMONTH,		//������	ȱʡ01
	CFGDAY,			//������	ȱʡ01

	CFGPSETID,		/* ����ѹ��������Сֵ����λΪbar, ȱʡ13����Χ 11��15 */
	CFGPBANDID,		/* ����ѹ�����ڷ�Χ����λΪbar, ȱʡ 5�� ��Χ 4��6 */
	CFGMINVID,		/* ��������ѹ��Сֵ����λΪ�����ѹ�İٷֱ�, ȱʡ30����Χ 30��50 */
	CFGMAXVID,		/* ��������ѹ���ֵ����λΪ�����ѹ�İٷֱ�, ȱʡ100����Χ 60��100 */
	CFGFANNUMID,		//��������� ��Χ1��2��ȱʡ1��
	CFGPSENSORTYPEID,	//ѹ�����������ͣ�ȱʡ 1��1����ѹ��;2��������
	CFGCOMMODE,		//ͨ�ſ��ƣ� ��Χ0��1��ȱʡ0��0:ѹ����1-ͨ�Ÿ�����
	CFGCOMFANOUT,	//ͨ���������Χ30��100��ȱʡ0��
	
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
	uint16_t dir;			 //��ǰ�������Ӳ���
	uint16_t inc;			 //��ǰ�������Ӳ���
}valve_cur;

typedef struct
{
	uint16_t valve_opening_cur[MAX_EEVNUM];		//��������ķ�����
	uint16_t valve_steps_cur[MAX_EEVNUM];				//��ǰ���Է�����
	valve_cur cur_steps[MAX_EEVNUM];			 //��ǰ�������Ӳ���
	uint16_t valve_phase[MAX_EEVNUM];				//����λ
	uint16_t valve_ctrl_signal[MAX_EEVNUM];	    //����ͣ�ź�
	uint16_t valve_ctrl_status[MAX_EEVNUM];			//����״̬
}valve_st;

	typedef enum _P_RANGE_POINT_
	{
		P_SET_POINT,		//ѹ���������õ�
		P_OFF_POINT,		//�رյ�ѹ���ѹ���㣺�������õ��1BAR
		P_FLAT_POINT,		//����ƽ̨����ѹ��
		P_FLAT2_POINT,		//�ָ�����趨�����ѹ��ѹ����ƽ̨����3BAR
		P_STEP_POINT,		//�����Ծ����ѹ��
		P_STEP_BAND,		//���ڷ�Χ
		MAXPOINTNUM
	} P_RANGE_POINT;
	
typedef struct
{
	uint16_t u16FanVout100;		//ʵ�����
	uint16_t Fan_out;		//��������ķ�����
	uint16_t u16PressuePoint[MAXPOINTNUM]; //�Ŵ�100�����й�ѹ������
	uint16_t ACCye[3];		//������ڣ�ն��ʱ��
  uint16_t u16Runtime[2][2];  //ʹ��ʱ��
}Fan_st;//���״̬

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

#define ABNORMAL_VALUE    0x7FFF//�쳣ֵ
#define ABNORMAL_VALUE2    0//�쳣ֵ
typedef struct 
{
		uint16_t 	id;
		uint16_t*	reg_ptr;
		uint16_t	dft;
		//uint8_t		rw;
}sts_reg_map_st;

//enum
//{
//		COOL_TYPE_MODULE_WIND=0,//�������
//		COOL_TYPE_MODULE_WATER,//�����䶳ˮ
//		COOL_TYPE_MODULE_MIX,
//		COOL_TYPE_COLUMN_WIND,//�м����
//		COOL_TYPE_COLUMN_WATER,//�м��䶳ˮ
//		COOL_TYPE_HUMIDITY=9,//��ʪ��
//};

enum
{
		ALARM_COM = 0,
		ALARM_NTC,
};

enum				//���鹤��״̬
{		
		PWR_STS_BPOS = 0,		        //���ػ�
		FAN_STS_BPOS,                   //���
		HEATING_STS_BPOS,				//����
		COOLING_STS_BPOS,				//����
		HUMING_STS_BPOS,				//��ʪ
		DEMHUM_STS_BPOS,				//��ʪ
		COOL_VALVE_BPOS,				//ˮ��
		TEAM_STANDALONE_STS_BPOS =8,	//����/Ⱥ��
		TEAM_STS_BPOS,					//Ⱥ������/ʧ��
		ALARM_STUSE_BPOS =14,           //�澯״̬λ
		ALARM_BEEP_BPOS =15             //�澯����־λ
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
  uint16_t Exhaust_temperature;//�����¶�
  uint16_t Exhaust_temperature_hystersis;//�����¶Ȼز�
}compressor_st;

//��Ƶ
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
	uint16_t avg_hi_press[4];//��ѹѹ��
	uint8_t counter_hipress;
	uint8_t counter_hi_Temperature;
	uint16_t Inv_hi_Temperature[4];//�����¶�
	uint8_t Inv_hi_temp_Flag;//�����¶Ƚ�Ƶ��ʶ
	uint8_t Inv_hi_temp_Count;//�����¶ȼ���
	uint8_t Inv_hi_temp_Stop;//�����¶�ͣ��
	uint32_t Inv_hi_temp_Starttime[3];//�����¶Ƚ�Ƶʱ��
	uint32_t Inv_hi_temp_Stoptime[3];//�����¶Ƚ�Ƶ�ָ�ʱ��
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
	uint16_t Delay;	//���ת����ʱ,Alair,20161113
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


