#ifndef __EVENT_RECORD_H__
#define __EVENT_RECORD_H__
#include "sys_conf.h"
//#include "cyc_buff.h"

//������¼

#define CRITICAL_ALARM_lEVEL     0x00//���ظ澯
#define MAJOR_ALARM_LEVEL        0x01//һ��澯
#define MIOOR_ALARM_LEVEL        0x02//��ʾ�澯



#define ALARM_FIFO_DEPTH  30

#define ALARM_STATUS_LEN    (sizeof(alram_node) - 4)

#define ALARM_RECORD_LEN    sizeof(alarm_log_st)

#define EVENT_RECORD_LEN    sizeof(event_log_st)
#define TEMP_HUM_RECORD_LEN  4*60

//�¼���¼
#define EVE_MAX_CNT 500 //�¼����¼������

#define EVENT_FIFO_DEPTH 64



//yxq
#pragma pack (1)

// typedef struct
// {
//	  uint16_t startpiont;
//	  uint16_t endpiont;
//	  uint16_t cnt;
// }record_pt_st;


typedef struct
{
	uint32_t trigger_time;
	uint16_t alarm_value;
	uint16_t alarm_id;
}alarm_table_st;


//�¼���¼�ṹ�壻
typedef struct
{
	uint16_t event_id;
	uint32_t time;
	uint16_t user_id;
	uint16_t former_data;
	uint16_t new_data;
} event_log_st;

//������¼�ṹ��
typedef struct
{
	uint16_t alarm_id;
	uint32_t trigger_time;
	uint32_t end_time;
	uint16_t rev;
}alarm_log_st;

typedef struct node
{
	uint32_t trigger_time;
	uint16_t  alarm_id;
	uint16_t alarm_value;
	struct node* next;
}alram_node;


#pragma pack ()


typedef enum
{
	USER_DEFAULT =0,
	USER_MODEBUS_SLAVE,
	USER_CPAD,
	USER_ADMIN,
}user_ID;


typedef enum
{
	ALARM_TRIGER=0,
	ALARM_END,
	
}alarm_enum;

enum
{
	EVENT_TYPE=0,//�������¼���¼
	ALARM_TYPE,//�����¼���¼
	TEM_HUM_TYPE,
};


enum
{
	HOUR_TIMER=0,
	DATE_TIMER,
	WEEK_TIMER,
	MONTH_TIMER,
	TIMER_TOTAL_NUM,
};

void  init_alarm_log(void);


//�澯״̬�ȹغ����ӿ�

void chain_init(void);

uint8_t  node_append(uint16_t alarm_id,uint16_t alarm_value);

uint8_t node_delete(uint16_t alarm_id);
void add_alarmlog_fifo( uint16_t alarm_id, alarm_enum flg,uint16_t  alarm_value);

uint8_t get_alarm_status(uint8_t*status_data,uint16_t start_num,uint8_t len);

#endif //__ALARM_ACL_H__

