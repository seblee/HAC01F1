#ifndef __ALAMRS_H__
#define __ALAMRS_H__

#include "sys_conf.h"

#define ACL_GROUP1_MASKBIT		0x1555u


#define ACL_INACTIVE 		0
#define ACL_PREACTIVE 	1
#define ACL_ACTIVE 			2
#define ACL_POSTACTIVE	3

#define ACL_ENABLE			0
#define ACL_SUPPRESS		1
#define ACL_DISABLE			2

//alarm acl def
enum
{
//
	ACL_ACFREQUENCYALM = 0,	//频率
	ACL_PSENSORALM,					//压力传感器
	ACL_COMMON,							//通信异常
	ACL_EEPROMALM,					//EEPROM
	ACL_TOTAL_NUM,
};
#define ACL_ACF		0x01//
#define ACL_PSE		0x02//
#define ACL_ACOM	0x04//
#define ACL_APC		0x07//

//************************Public global variable definition************************
	   //Alarm array index definition
	    typedef enum _ALARMID_
	    {
 	       PHASELOSTALM,
 	       SCROVTALM,
 	       FAN1OVTALM,
					FAN2OVTALM,
 	       PSENSORALM,
 	       EEPROMALM,
 	       SCRT2SENSORALM,
 	       ACFREQUENCYALM,
				

 	       MAXALMID
	     } ALARMIDINDEX;
	/* 告警量寄存器 */
	extern UCHAR g_u8AlmStatus[MAXALMID];

		//Alarm value constant, same definition as YDN23
		#define NORMAL 0
		#define ALARM  0xf0

//************************Interface function declaration******************************
	// 告警信息初始化
	void InitAlarm(void);

    //Alarm information deal with
    void AlarmDeal(void);


void alarm_acl_init(void);
void alarm_acl_exe(uint8_t index);

uint8_t get_alarm_bitmap(uint8_t alarm_id);

uint8_t clear_alarm(void);

#endif //__ALAMRS_H__
