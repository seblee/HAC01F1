/*********************************************************
  Copyright (C), 2014, Sunrise Group. Co., Ltd.
  File name:      	eev_proc.c
  Author: gongping	Version: 0.7       Date:  2014-12-05
  Description:    	Main entry, system threads initialization
  Others:         	n.a
  Function List:  	water_level_sts_get(void)
                    calc_conductivity(void);
										calc_humcurrent(void);
  Variable List:  	n.a
  Revision History:         
  Date:           Author:          Modification:
	2014-12-05      gongping         file create
*********************************************************/
#include "cmsis_os.h"  
#include "adc.h"
#include "dac.h"
#include <math.h>
#include "daq.h"
#include "dio.h"
#include "eev.h"
#include "pid.h"
#include "global_var.h"
#include "eev_proc.h"
#include "sys_conf.h"
__IO uint16_t adcval[4];
volatile uint16_t com_change_excSpeed = 0;
volatile uint16_t valve_ctrl_stage[MAX_EEVNUM] = {STATUS_INIT,STATUS_INIT};
daq_reg_st daq_reg_st_inst;
uint8_t valveStepManFlag[MAX_EEVNUM];

float integral_coefficient;
extern uint8_t is_inited_pidCalc;

static void eev_fsm(uint8_t index);
static void eev_para_update(uint8_t index);

void eev_para_update_process(void)
{
	if((g_sys.status.work_mode >> 0x01) == ALONE_MODE)      //@bit1
	{
		if((g_sys.config.dev_mask.eev &(0x0001<<EEV1)) != 0)
		{
			eev_para_update(EEV1);
		}
		
		if((g_sys.config.dev_mask.eev &(0x0001<<EEV2)) != 0)
		{
			eev_para_update(EEV2);
		}
	}			//同步
	else
	{
		eev_para_update(EEV1);
	}
}


void eev_para_update(uint8_t index)
{
	extern sys_reg_st g_sys;
	if((g_sys.config.dev_mask.eev >> index) & 0x0001)
	{
		g_sys.status.valve.valve_phase[index] = getBeatIndex(index);
		g_sys.status.valve.valve_steps_cur[index] = getFinalPluse(index);
		g_sys.status.valve.valve_opening_cur[index] = getOpenValveDegree(index);
	}
}

static void change_excSpeed(void)
{
	extern sys_reg_st g_sys;
	if(com_change_excSpeed != g_sys.config.general.excSpeed)
	{
		com_change_excSpeed = g_sys.config.general.excSpeed;
		eev1_timerInit(g_sys.config.general.excSpeed);
		osDelay(10); 
		eev2_timerInit(g_sys.config.general.excSpeed);
	}
}

void daq_gvar_update(void)
{
	extern sys_reg_st	g_sys; 
	ai_sts_update();											//update g_ain_inst
//	temp_avg_process();
//	press_avg_process();
//	vapor_temp_avg_process();
}
//手动调节
void eev_manual_process(uint8_t index)        
{
	uint8_t dir_temp;
	uint16_t u16Current_Step = 0,u16Delta = 0;
	u16Current_Step = getFinalPluse(index);
	if((g_sys.status.work_mode >> 0x01) == SYNC_MODE)
	{
		if(index == EEV2)
		{
			g_sys.config.general.valve_pos_manual_comm[index] = g_sys.config.general.valve_pos_manual_comm[EEV1];
		}
	}

	if(u16Current_Step != g_sys.config.general.valve_pos_manual_comm[index])
	{
		if(u16Current_Step > g_sys.config.general.valve_pos_manual_comm[index])
		{
			dir_temp = DIRCLOSE;
		}
		else
		{
			dir_temp = DIROPEN;
		}
		u16Delta = ABS(u16Current_Step - g_sys.config.general.valve_pos_manual_comm[index]);
		setEevMovInfo(dir_temp,u16Delta,RUN,index);
	}
	else
	{
		dir_temp = DIRNULL;
		u16Delta = 0;
		setEevMovInfo(dir_temp,u16Delta,STOP,index);
	}	
}

void eev_handle(uint8_t index)
{
	extern sys_reg_st g_sys;
	int16_t diffErr[MAX_EEVNUM];
	static int16_t diffErr_Last[MAX_EEVNUM] = {0,0};
	uint16_t SuperHeart_Err;//,Steps;
	uint16_t delta;//,Steps;
	int16_t Open_Per,Open_Step,SH_Last;
	uint16_t MinSteps;//,Steps;
	
	static uint16_t num = 0;
    if(((g_sys.config.dev_mask.eev >> index) & 0x0001)&&(g_sys.status.valve.valve_ctrl_signal[index] == VALVE_ON))
    {
		super_heat_avg_process();
//			if(g_sys.config.general.enable_valve_pos_manual==ENABLE)
		if((g_sys.status.work_mode&BIT0)==MAN_MODE)//手动模式
		{
			if((g_sys.status.work_mode >> 0x01) == ALONE_MODE)   
			{
				if(valveStepManFlag[index])
				{
					eev_manual_process(index);
					valveStepManFlag[index] = 0;
				}
			}
			else
			{
				if(valveStepManFlag[EEV1])
				{
					eev_manual_process(index);
					eev_manual_process(EEV2);
					valveStepManFlag[EEV1] = 0;
				}
			}
		}
		else   //自动调节模式
		{
			if(g_sys.status.environmen.superHeat[index] == ABNORMAL_VALUE)
			{
				return;			//过热度异常
			}
			if(g_sys.status.alarm_bitmap & (ACL_GROUP1_MASKBIT << index))			//有报警不执行PID
			{
				return;
			}
			switch(g_sys.config.algorithm.eev_ctrl_mode)
			{
					case PID_ALOGORITHM:
					if((g_sys.config.general.Dehumidity_Supperheart>0)&&(g_sys.config.general.Dehumidity_Supperheart<0x7FFF))//除湿模式
					{
						diffErr[index]= (int16_t)g_sys.status.environmen.superHeat[index]- g_sys.config.general.Dehumidity_Supperheart; 			
					}
					else
					{
						diffErr[index]= (int16_t)g_sys.status.environmen.superHeat[index]- g_sys.config.general.super_heat[index]; 
					}		
					g_sys.status.Test_Buff[5]=0;	
							g_sys.status.Test_Buff[1]=diffErr_Last[index];
							g_sys.status.Test_Buff[2]=diffErr[index];					
					SH_Last = (int16_t)diffErr[index]-(int16_t)diffErr_Last[index];				
					diffErr_Last[index] = diffErr[index];					
					SuperHeart_Err = ABS(diffErr[index]);
					if(SuperHeart_Err >= g_sys.config.algorithm.dead_zone)   	//在死区范围之外,大于死区才调节
					{
											g_sys.status.Test_Buff[5]|=0x01;
						if(++num >= g_sys.config.algorithm.samp_time)
//						if(++num >= g_sys.status.Cal_Time)
						{
							g_sys.status.Test_Buff[5]|=0x02;
							num = 0;	
							Open_Per = pidCalc(&pid_inst[index],diffErr[index],g_sys.config.algorithm.dead_zone);     //PID 增加的阀开度
							Open_Step = Open_Per * g_sys.config.general.excAllOpenSteps / 1000;  
							delta = ABS(Open_Step);
//							g_sys.status.Test_Buff[0]=temp;
							g_sys.status.Test_Buff[3]=SH_Last;
							if(SuperHeart_Err >= 40)//无限制
							{
								if(delta >= 30)           //限制
								{
										delta = 30;
								}
								else
								{
										delta = delta;									
								}
								g_sys.status.Test_Buff[5]|=0x04;
							}	
							else if(SuperHeart_Err > 30)
							{
								if(delta >= 7)           //限制
								{
										delta = 7;
								}
							}							
							else if(SuperHeart_Err > 20)
							{
								if(delta >= 5)           //限制
								{
									delta = 5;
								}
							}
							else
							{
								if(delta >= 3)			//限制
								{
									delta = 3;
								}
							}
							if(SH_Last < 0)    //过热度变小，进行关阀操作   减小的趋势
							{
								g_sys.status.Test_Buff[5]|=0x08;
								 if(getFinalPluse(index) > getPluseNumofOpenValve(g_sys.config.general.excOpenValveMinDegree))												//关阀不能低于%20
								 {
									 if(getFinalPluse(index) - delta < getPluseNumofOpenValve(g_sys.config.general.excOpenValveMinDegree))						
									 {
										delta = getFinalPluse(index) - getPluseNumofOpenValve(g_sys.config.general.excOpenValveMinDegree);
									 }
								 }
								 else
								 {
									delta = 0;
								 }								 
								 if(getOpenValveDegree(index) > g_sys.config.general.excOpenValveMinDegree)
  								 {
  									 g_sys.status.valve.cur_steps[index].inc = delta;
									 setEevMovInfo(DIRCLOSE,delta,RUN,index);
  								 }
  								 else
  								 {
  									 g_sys.status.valve.cur_steps[index].inc = 0;
									 setEevMovInfo(DIRNULL,0,STOP,index);
  								 }
  								 if((g_sys.status.work_mode >> 0x01) == SYNC_MODE)
  								 {
  									 if(((g_sys.config.dev_mask.eev >> EEV2) & 0x0001)&&(g_sys.status.valve.valve_ctrl_signal[EEV2] == VALVE_ON))
  									 {  										 
										 if(getOpenValveDegree(EEV2) > g_sys.config.general.excOpenValveMinDegree)
  										 {
  											 g_sys.status.valve.cur_steps[EEV2].inc = delta;
											 setEevMovInfo(DIRCLOSE,delta,RUN,EEV2);
  										 }
  										 else
  										 {
  											 g_sys.status.valve.cur_steps[EEV2].inc = 0;
											 setEevMovInfo(DIRNULL,0,STOP,EEV2);
  										 }
  									 }
  								 }
							}
							else
							if(SH_Last > 0)    //过热度变大，进行开阀操作    增大的趋势
							{
								g_sys.status.Test_Buff[5]|=0x10;

								if(delta + getFinalPluse(index) > g_sys.config.general.excAllOpenSteps)			//开阀步数需求太大限制
								{
																	g_sys.status.Test_Buff[5]|=0x20;
									delta = g_sys.config.general.excAllOpenSteps - getFinalPluse(index);
								}
//																g_sys.status.Test_Buff[3]=delta;
								g_sys.status.valve.cur_steps[index].inc = delta;
								setEevMovInfo(DIROPEN,delta,RUN,index);						
								if((g_sys.status.work_mode >> 0x01) == SYNC_MODE)
								{
																	g_sys.status.Test_Buff[5]|=0x40;
									if(((g_sys.config.dev_mask.eev >> EEV2) & 0x0001)&&(g_sys.status.valve.valve_ctrl_signal[EEV2] == VALVE_ON))
									{
										if(delta + getFinalPluse(EEV2) > g_sys.config.general.excAllOpenSteps)			//开阀步数需求太大限制
										{
											delta = g_sys.config.general.excAllOpenSteps - getFinalPluse(EEV2);
										}
										g_sys.status.valve.cur_steps[EEV2].inc = delta;
										setEevMovInfo(DIROPEN,delta,RUN,EEV2);	
									}
								}
							}
							else      //不变的情况
							{
																g_sys.status.Test_Buff[5]|=0x100;
								if(diffErr[index] > 0)			//大于设点  开阀
								{
																									g_sys.status.Test_Buff[5]|=0x200;
									if(delta + getFinalPluse(index) > g_sys.config.general.excAllOpenSteps)			//开阀步数需求太大限制
									{
										delta = g_sys.config.general.excAllOpenSteps - getFinalPluse(index);
																									g_sys.status.Test_Buff[5]|=0x400;
									}
																	g_sys.status.Test_Buff[4]=delta;
									g_sys.status.valve.cur_steps[index].inc = delta;
									setEevMovInfo(DIROPEN,delta,RUN,index);						
									if((g_sys.status.work_mode >> 0x01) == SYNC_MODE)
									{
										if(((g_sys.config.dev_mask.eev >> EEV2) & 0x0001)&&(g_sys.status.valve.valve_ctrl_signal[EEV2] == VALVE_ON))
										{
											if(delta + getFinalPluse(EEV2) > g_sys.config.general.excAllOpenSteps)			//开阀步数需求太大限制
											{
												delta = g_sys.config.general.excAllOpenSteps - getFinalPluse(EEV2);
											}
											g_sys.status.valve.cur_steps[EEV2].inc = delta;
											setEevMovInfo(DIROPEN,delta,RUN,EEV2);	
										}
									}
								}
								else
								if(diffErr[index] < 0)	        //小于设点  关阀
								{
																									g_sys.status.Test_Buff[5]|=0x1000;
									MinSteps=getPluseNumofOpenValve(g_sys.config.general.excOpenValveMinDegree);

									if(getFinalPluse(index) > MinSteps)												//关阀不能低于%20
									 {
										 if(getFinalPluse(index) - delta < MinSteps)						
										 {
											delta = getFinalPluse(index) - MinSteps;
										 }
									 }
									 else
									 {
										delta = 0;
									 }									 
									 if(getOpenValveDegree(index) > g_sys.config.general.excOpenValveMinDegree)
									 {
										 g_sys.status.valve.cur_steps[index].inc = delta;
										 setEevMovInfo(DIRCLOSE,delta,RUN,index);
									 }
									 else
									 {
										 g_sys.status.valve.cur_steps[index].inc = 0;
										 setEevMovInfo(DIRNULL,0,STOP,index);
									 }
									 if((g_sys.status.work_mode >> 0x01) == SYNC_MODE)
									 {
										 if(((g_sys.config.dev_mask.eev >> EEV2) & 0x0001)&&(g_sys.status.valve.valve_ctrl_signal[EEV2] == VALVE_ON))
										 {
											 if(getOpenValveDegree(EEV2) > g_sys.config.general.excOpenValveMinDegree)
											 {
												 g_sys.status.valve.cur_steps[EEV2].inc = delta;
												 setEevMovInfo(DIRCLOSE,delta,RUN,EEV2);
											 }
											 else
											 {
												 g_sys.status.valve.cur_steps[EEV2].inc = 0;
												 setEevMovInfo(DIRNULL,0,STOP,EEV2);
											 }
										 }
									 }																
								}
							}
						}
					}
					else     //在死区范围内
					{	
						g_sys.status.valve.cur_steps[index].inc = 0;
						setEevMovInfo(DIRNULL,0,STOP,index);
						if((g_sys.status.work_mode >> 0x01) == SYNC_MODE)
						{
							g_sys.status.valve.cur_steps[EEV2].inc = 0;
							setEevMovInfo(DIRNULL,0,STOP,EEV2);
						}						
					}					
					break;
					
				   /*
				   case STEP_ALOGORITHM:
				   if(++num >= 1)				// 1s执行一次
				   {
						num = 0;
						diffErr[index]= g_sys.config.general.super_heat[index] - (int16_t)g_sys.status.environmen.superHeat[index];
						if(ABS(diffErr[index]) < DEADZONE)
						{
							setEevMovInfo(DIRNULL,0,STOP,index);
							return;
						}
						if(diffErr[index] < 0)
						{
							if(eevPack[index].eevMoveInfo.finalPluse < g_sys.config.general.excAllOpenSteps)
							{
								setEevMovInfo(DIROPEN,1,RUN,index);
							}
						}
						else
						{
							if(eevPack[index].eevMoveInfo.finalPluse != 0)
							{
								setEevMovInfo(DIRCLOSE,1,RUN,index);
							}						
						}
					}
					break;
				*/
				
				default:	
				break;
			}
		}
	}
}

static void eev_fsm_process(void)
{
	//单路时根据配置位谁控制谁;双路时只要配置位统一为第一路
	if((g_sys.status.work_mode >> 0x01) == ALONE_MODE)
	{
		if((g_sys.config.dev_mask.eev &(0x0001<<EEV1)) != 0)
		{
			eev_fsm(EEV1);
		}
		
		if((g_sys.config.dev_mask.eev &(0x0001<<EEV2)) != 0)
		{
			eev_fsm(EEV2);
		}
	}
	else
	if((g_sys.status.work_mode >> 0x01) == SYNC_MODE)
	{
		eev_fsm(EEV1);
		if(g_sys.status.valve.valve_ctrl_status[EEV2] != STATUS_ON)
		{
			eev_fsm(EEV2);
		}
	}	
}


void eev_fsm(uint8_t index)
{
	extern sys_reg_st g_sys;
	static uint8_t hold_off_cnt[MAX_EEVNUM] = {0,0};
	static uint8_t is_inited[MAX_EEVNUM] = {0,0};
	int16_t diffTemp = 0;
	uint16_t preTemp = 0;
	uint8_t index_temp;
	uint16_t delta = 0;
	
	if((g_sys.config.dev_mask.eev &(0x0001<<index)) != 0)
	{
		switch(valve_ctrl_stage[index])
		{
			case STATUS_INIT:
			setEevMovInfo(DIRCLOSE,g_sys.config.general.excAllOpenSteps,RUN,index);		//电子膨胀阀的开度先关闭至0开度
			eevPack[index].eevMoveInfo.finalPluse=0x00;
			valve_ctrl_stage[index] = STATUS_CLOSE;         //关阀状态
			break;

			case STATUS_CLOSE:
			if((eevPack[index].eevMoveInfo.now_status == STOP) && (eevPack[index].eevMoveInfo.now_dir == DIRNULL))
			{
				valve_ctrl_stage[index] = STATUS_OFF;		//已经处于全关状态 
				if(is_inited[index] == 0)
				{
				      is_inited[index] = 0x01;
					  hold_off_cnt[index] = g_sys.config.general.eevHoldTime;
				}
				else
				{
					  hold_off_cnt[index] = HOLD_TIME3S_CNT;
				}		
			}
			else
			{
				hold_off_cnt[index] = 0;
			}
			break;

			case STATUS_OFF:

			if(hold_off_cnt[index] != 0)
			{
				hold_off_cnt[index]--;
			}
			else
			{

				if(g_sys.status.valve.valve_ctrl_signal[index] == VALVE_ON)		//有开阀信号先开阀至全开阀脉冲的一半
				{
//					eevPack[index].eevMoveInfo.beatIndex = stepA;
//					setEevMovInfo(DIROPEN,(g_sys.config.general.excAllOpenSteps >> 1),RUN,index);
//					valve_ctrl_stage[index] = STATUS_POS;              //阀定位  
						if(GetEevMovInfo(index))//还在动作
						{
								break;
						}
						if(getFinalPluse(index) <= (g_sys.config.general.excAllOpenSteps >> 1))//50%
						{
								delta=(g_sys.config.general.excAllOpenSteps >> 1)-getFinalPluse(index);
								eevPack[index].eevMoveInfo.beatIndex = stepA;
								setEevMovInfo(DIROPEN,delta,RUN,index);		
								valve_ctrl_stage[index] = STATUS_POS;              //阀定位  
						}
						else
						{
								valve_ctrl_stage[index] = STATUS_INIT;							
								hold_off_cnt[index] = 0;		
						}
				}
				else
				{	
					if(GetEevMovInfo(index))//还在动作
					{
							break;
					}
					if((g_sys.config.general.valve_opened_Standby>=0)&&(g_sys.config.general.valve_opened_Standby<=100))
					{
							if(getFinalPluse(index) < getPluseNumofOpenValve(g_sys.config.general.valve_opened_Standby))//待机33%
							{
									delta=getPluseNumofOpenValve(g_sys.config.general.valve_opened_Standby)-getFinalPluse(index);
									eevPack[index].eevMoveInfo.beatIndex = stepA;
									setEevMovInfo(DIROPEN,delta,RUN,index);		
									hold_off_cnt[index] = HOLD_TIME3S_CNT;
							}
							else if(getFinalPluse(index) > getPluseNumofOpenValve(g_sys.config.general.valve_opened_Standby))//待机33%
							{
								delta=getFinalPluse(index)-getPluseNumofOpenValve(g_sys.config.general.valve_opened_Standby);
								eevPack[index].eevMoveInfo.beatIndex = stepA;
								setEevMovInfo(DIRCLOSE,delta,RUN,index);	
								hold_off_cnt[index] = HOLD_TIME3S_CNT;	
							}							
					}
					valve_ctrl_stage[index] = STATUS_OFF;
				}
			}
			break;

			case STATUS_POS:
			if(g_sys.status.valve.valve_ctrl_signal[index] == VALVE_ON)
			{
				if((eevPack[index].eevMoveInfo.now_status == STOP) && (eevPack[index].eevMoveInfo.now_dir == DIRNULL))
				{
					valve_ctrl_stage[index] = STATUS_WAIT;
					hold_off_cnt[index] = HOLD_TIME3S_CNT;
				}
			}
			else
			{
				valve_ctrl_stage[index] = STATUS_CLOSE;
				setEevMovInfo(DIRCLOSE,(eevPack[index].eevMoveInfo.curPluse),RUN,index);
			}
			break;
			
			case STATUS_WAIT:
			if(hold_off_cnt[index] != 0)
			{
				hold_off_cnt[index]--;
			}
			else
			{
				if(g_sys.status.valve.valve_ctrl_signal[index] == VALVE_ON)
				{
					valve_ctrl_stage[index] = STATUS_PRE_ADJUST;  
					if((g_sys.status.work_mode&BIT0)==AUTO_MODE)//自动模式
					{
							hold_off_cnt[index] = g_sys.config.algorithm.pre_adjust_time;//预调节
					}		
				}
				else
				{
					valve_ctrl_stage[index] = STATUS_CLOSE;
					setEevMovInfo(DIRCLOSE,(eevPack[index].eevMoveInfo.curPluse),RUN,index);
				}
			}
			break;
			case STATUS_PRE_ADJUST:     //预调节控制
			if(g_sys.status.valve.valve_ctrl_signal[index] == VALVE_ON)
			{
				if(hold_off_cnt[index] != 0)
				{
					hold_off_cnt[index]--;
					super_heat_avg_process();
					index_temp = index;
					if((g_sys.status.work_mode >> 0x01) == SYNC_MODE)
					{
						if(index == EEV2)				//用index=0作为过热度比较，用index=1进行控制阀
						{
							index_temp = EEV1;
						}
					}
					
					if(g_sys.status.environmen.superHeat[index_temp] == ABNORMAL_VALUE)
					{
						 return;			//过热度异常
					}
					if(g_sys.status.alarm_bitmap & (ACL_GROUP1_MASKBIT << index_temp))			//有报警不执行PID
					{
						return;
					}	
					if((g_sys.config.general.Dehumidity_Supperheart>0)&&(g_sys.config.general.Dehumidity_Supperheart<0x7FFF))//除湿模式
					{
						diffTemp = (int16_t)g_sys.status.environmen.superHeat[index_temp]- g_sys.config.general.Dehumidity_Supperheart; 				
					}
					else
					{
						diffTemp = (int16_t)g_sys.status.environmen.superHeat[index_temp]- g_sys.config.general.super_heat[index_temp]; 
					}				 
					if(ABS(diffTemp) > g_sys.config.algorithm.dead_zone)
					{
						preTemp = ABS(diffTemp)/10;
						if(preTemp >= 3)
						{
							preTemp = 3;
						}
						if(diffTemp > 0)
						{
							setEevMovInfo(DIROPEN,preTemp,RUN,index);
						}
						else
						{
							setEevMovInfo(DIRCLOSE,preTemp,RUN,index);
						}
					}								
				}
				else
				{
					valve_ctrl_stage[index] = STATUS_ON;  
				}
			}
			else
			{
				valve_ctrl_stage[index] = STATUS_CLOSE;
				setEevMovInfo(DIRCLOSE,(eevPack[index].eevMoveInfo.curPluse),RUN,index);
			}
			break;
			case STATUS_ON:				//开始调节控制了
			if(g_sys.status.valve.valve_ctrl_signal[index] == VALVE_ON)
			{
				eev_handle(index);
			}
			else
			{
//				valve_ctrl_stage[index] = STATUS_CLOSE;
//				setEevMovInfo(DIRCLOSE,(eevPack[index].eevMoveInfo.curPluse),RUN,index);
				valve_ctrl_stage[index] = STATUS_INIT;
				hold_off_cnt[index] = 0;
			}
			break;

			default:
			valve_ctrl_stage[index] = STATUS_INIT;
			hold_off_cnt[index] = 0;
			break;
		}
		g_sys.status.valve.valve_ctrl_status[index] = valve_ctrl_stage[index];
	}
}

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
void eev_proc(void const *argument)
{
	extern sys_reg_st g_sys;
//	com_change_excSpeed = g_sys.config.general.excSpeed;
	while(1)
	{		
//		work_mode_update();						   //工作模式的更新
//		eev_para_update_process();
//		valve_ctrl_update();						   //阀启停信号的更新
//		eev_fsm_process();
//		change_excSpeed();						   //修改励磁速度
		osDelay(EEV_PROC_DLY);
	}
}


