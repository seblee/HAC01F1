#include "event_record.h"
#include"I2c_bsp.h"
#include "fifo.h"
#include <string.h>
#include "daq.h"
#include "sys_status.h"
#include "cmsis_os.h"
#define  INVALID_HUM_TEMP  0xf001;
// alarm status chian
static alram_node head_node ;

//chain

//operationall event record
static fifo8_cb_td eventlog_fifo_inst;

//Alarm record

static fifo8_cb_td  alarmlog_fifo;


//tem_hum 
typedef struct
{
	uint8_t base;
	uint8_t c1;
	uint8_t c2;
}cnt_st;

osPoolId   MemPool_Id;
osPoolDef(MemPool,ACL_TOTAL_NUM,alram_node);

void  chain_init(void)
{
	extern sys_reg_st	g_sys; 	
	
	MemPool_Id = osPoolCreate(osPool(MemPool));
	if(MemPool_Id == NULL)
	{
		//内存分配失败！
		return;
	}
	head_node.next = NULL;
	head_node.alarm_value = 0;//报警计数
	g_sys.status.alarm_status_cnt.total_cnt = 0;	
	g_sys.status.alarm_status_cnt.major_cnt = 0;
	g_sys.status.alarm_status_cnt.critical_cnt = 0;
	g_sys.status.alarm_status_cnt.mioor_cnt = 0;
}

////计算二进制1个数
//static uint8_t Calc_Bit1(uint16_t Num)
//{
//			uint8 Count=0;
//	
//			while(Num!= 0)
//			{
//					Count ++;
//					Num&= (Num - 1);
//			}		
//			return Count;
//}	
static void calc_alrarm_cnt(void)
{
	alram_node *p_node2;
	uint8_t critical_cnt,major_cnt,mioor_cnt;
	uint16_t alarm_id;
	extern sys_reg_st		g_sys;

	p_node2 = head_node.next;
	critical_cnt=0;
	major_cnt = 0;
	mioor_cnt = 0;
	if(head_node.next == NULL)
	{
		;
	}
	else
	{
		while(p_node2 !=  NULL)
		{				
			alarm_id = p_node2->alarm_id;
			alarm_id = alarm_id>>8;
			if((alarm_id&0x03)== CRITICAL_ALARM_lEVEL)
			{
				critical_cnt++;	
			}
			else if(((alarm_id&0x03 )== MAJOR_ALARM_LEVEL))
			{
				major_cnt++;
			}
			else
			{
				mioor_cnt++;
			}
			p_node2=p_node2->next;
		}
	}

	g_sys.status.alarm_status_cnt.total_cnt = critical_cnt + major_cnt + mioor_cnt; 
	g_sys.status.alarm_status_cnt.mioor_cnt = mioor_cnt;
	
	if(g_sys.status.alarm_status_cnt.total_cnt >0)
	{
		sys_set_remap_status(ALARM_STUSE_BPOS,1);
	}
	else
	{
		sys_set_remap_status(ALARM_STUSE_BPOS,0);
	}
	
	if((g_sys.status.alarm_status_cnt.major_cnt < major_cnt)||(g_sys.status.alarm_status_cnt.critical_cnt < critical_cnt))
	{
		sys_set_remap_status(ALARM_BEEP_BPOS,1);//严重告警
	}
	g_sys.status.alarm_status_cnt.major_cnt = major_cnt;
	g_sys.status.alarm_status_cnt.critical_cnt = critical_cnt;

	if((g_sys.status.alarm_status_cnt.major_cnt == 0)&&(g_sys.status.alarm_status_cnt.critical_cnt ==0))
	{
		sys_set_remap_status(ALARM_BEEP_BPOS,0);
	}
}
//增加节点
uint8_t  node_append(uint16_t alarm_id,uint16_t alarm_value)
{
	extern sys_reg_st		g_sys; 	
	alram_node *p_node1;
	//time_t now;
  
	p_node1 = (alram_node*)osPoolAlloc(MemPool_Id);
	if(p_node1 == NULL)
	{
		return 0;
	}
	//get_local_time(&now);
	p_node1->next = head_node.next ;
	//节点数量计数
	p_node1->alarm_id = alarm_id;
	p_node1->alarm_value = alarm_value;
	//p_node1->trigger_time = now;
	head_node.next = p_node1;
	//计算报警状态总数和报警总类 
	calc_alrarm_cnt();
	return 1;
}

//删除节点

uint8_t node_delete  (uint16_t alarm_id)
{
	alram_node *p_node1,*p_node2,*p_node3;
	uint8_t cnt;
	extern sys_reg_st		g_sys;
 	
	p_node1 = NULL;
	p_node2 = head_node.next;
	p_node3 = &head_node;
	
	if(head_node.next == NULL)
	{
		return(0);
	}
	//节点数量
	cnt = g_sys.status.alarm_status_cnt.total_cnt;
	while(cnt)
	{
		cnt--;
		if(p_node2->alarm_id == alarm_id)
		{
			p_node1 = p_node2;
			break;
		}
		else
		{
			if(p_node2->next != NULL)
			{
				p_node3 = p_node2;
				p_node2 = p_node2->next;
			}
			else
			{
				break;
			}
		}
	}

	if(p_node1 != NULL)
	{
		p_node3->next = p_node1->next;
		osPoolFree(MemPool_Id,p_node1);
		calc_alrarm_cnt();		
		return(1);
	}
	return(0);	
}

uint8_t get_alarm_status(uint8_t*status_data,uint16_t start_num,uint8_t len)
{
		uint8_t read_len;
		uint8_t index;
		alram_node *start_node_pt;
		extern sys_reg_st					g_sys;
	
		read_len = 0;
		index = 0;
		if(g_sys.status.alarm_status_cnt.total_cnt <= start_num)
		{
				return(0);
		}
		start_node_pt = head_node.next;
	
		while(start_num--)
		{
			if(start_node_pt == NULL)
			{
				return(0);
			}
			if(start_node_pt->next == NULL)
			{
					return(0);
			}
			start_node_pt = start_node_pt->next;
		}
		read_len = 0;
		for(index = 0;index < len;index++)
		{
			read_len++;
			*(status_data++) = start_node_pt->alarm_id>>8;
			*(status_data++) = start_node_pt->alarm_id;
			*(status_data++) = (start_node_pt->trigger_time>>24);
			*(status_data++) = (start_node_pt->trigger_time>>16);
			*(status_data++) = (start_node_pt->trigger_time>>8);
			*(status_data++) = (start_node_pt->trigger_time);
			*(status_data++) =( start_node_pt->alarm_value>>8);
			*(status_data++) = start_node_pt->alarm_value;
			
		//	rt_kprintf("\n start_node->alarm_id = %d ,start_node->alarm_value= %d\n",start_node->alarm_id,start_node->alarm_value);
			
			if(start_node_pt->next == NULL)
			{
					return(read_len);
			}
			start_node_pt = start_node_pt->next;
			
		}

		return(read_len);
}



/*********************************************************
	add Event_Record FiFo

	FLG =0 ;ALARM Trigger;

	FLG =1 ALARM_END;
	
***********************************************************/

void add_alarmlog_fifo( uint16_t alarm_id, alarm_enum flg,uint16_t  alarm_value)
{
//	// fifo
//	alarm_log_st alarmLog_inst;
//	//获取系统时间
//	time_t now;
//	get_local_time(&now);
//	if(flg == ALARM_END)
//	{
//		alarmLog_inst.end_time = now;
//		alarmLog_inst.trigger_time = 0;	
//	}
//	else
//	{
//		alarmLog_inst.trigger_time = now;	
//		alarmLog_inst.end_time = 0xFFFFFFFF;
//	}

//	alarmLog_inst.alarm_id = alarm_id;
//	alarmLog_inst.rev = alarm_value;
//	
//	//压入FIFO;
//	if(fifo8_push(&alarmlog_fifo,(uint8_t*)&alarmLog_inst) == 0)
//	{
//		rt_kprintf("\nfifo16_push ERRO\n");
//	}
//	
//	
	return;
}



