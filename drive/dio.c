#include "dio.h"
#include "eev_proc.h"
const pin_map_st in_pin_map_inst[DI_MAX_CNT]=//鏁板瓧杈撳叆Pin_Map
{
	{GPIO_Pin_6,GPIOA},		//SW1
	{GPIO_Pin_7,GPIOA},		//SW2
  {GPIO_Pin_0,GPIOB},		//SW3
	{GPIO_Pin_1,GPIOF},		//DI1
	{GPIO_Pin_0,GPIOF},		//DI2
};


const pin_map_st out_pin_map_inst[DO_MAX_CNT]=
{
	{GPIO_Pin_1, GPIOB},	//LED_Alarm
	{GPIO_Pin_12, GPIOA}	//LED_Ctrl
};

uint16_t do_set(int16_t pin_id, BitAction value)
{
		if(pin_id < DO_MAX_CNT)
		{
			GPIO_WriteBit(out_pin_map_inst[pin_id].pin_base,out_pin_map_inst[pin_id].pin_id,value);
			return 1;
		}
		else
		{
			return 0;
		}
}

void drv_dio_bsp_init(void)
{
	uint8_t i;
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOF, ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	for(i = 0;i < DI_MAX_CNT;i++)
	{
		if(i < DI1)
		{
			GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		}
		else
		{
			GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		}
		GPIO_InitStruct.GPIO_Pin = in_pin_map_inst[i].pin_id;
		GPIO_Init(in_pin_map_inst[i].pin_base, &GPIO_InitStruct);
	}
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed =GPIO_Speed_Level_3;
	for(i = 0;i < DO_MAX_CNT;i++)
	{		
		GPIO_InitStruct.GPIO_Pin = out_pin_map_inst[i].pin_id;
		GPIO_Init(out_pin_map_inst[i].pin_base,&GPIO_InitStruct);
		do_set(i,Bit_SET);				//只有报警时才打开
	}
}

/**
  * @brief  raw digital input data read
  * @param  none
  * @retval 18 channels data, each bit stands for one channel
  */
//数字输入状态更新
static uint32_t di_read(void)
{
    extern sys_reg_st g_sys;
    uint32_t read_bitmap;
    uint16_t i;
    read_bitmap = 0;
    for (i = 0; i <DI_MAX_CNT; i++)
    {
        read_bitmap |= GPIO_ReadInputDataBit(in_pin_map_inst[DI_MAX_CNT - 1 - i].pin_base, in_pin_map_inst[DI_MAX_CNT - 1 - i].pin_id);
        if (i < (DI_MAX_CNT - 1))
        {
            read_bitmap = read_bitmap << 1;
        }
    }
		g_sys.status.din_bitmap[0]=read_bitmap;
		read_bitmap^=DI_MASK;//常开
    return read_bitmap;
}

void DI_update(void)
{
    extern sys_reg_st g_sys;
		g_sys.status.din_bitmap[1]=di_read();
    return ;	
}


void drv_dio_init(void)
{
    drv_dio_bsp_init();
    return ;
}

//void work_mode_update(void)
//{
//	extern sys_reg_st g_sys;
//	uint16_t work_modetemp = SYNC_MODE;        //11:D_SIMU，10:D_COMM 01:S_SIMU 00:S_COMM
//	uint16_t run_modetemp = AUTO_MODE;
//	g_sys.status.work_mode = 0x00;
//	if(GPIO_ReadInputDataBit(in_pin_map_inst[SW1].pin_base,in_pin_map_inst[SW1].pin_id) != Bit_RESET)  //SW1: 0-同步、1-独立
//	{
//		work_modetemp = 0x00;	     //@ bit1
//	}
//	else	
//	{
//		work_modetemp = 0x02;	      
//	}
//	g_sys.status.work_mode&=0xFFFD;
//	g_sys.status.work_mode |= work_modetemp;
//	if(g_sys.config.general.enable_valve_pos_manual == enable)//0-自动、1-手动
//	{
//		run_modetemp = MAN_MODE;		//@ bit0
//	}
//	else
//	{
//		run_modetemp = AUTO_MODE;
//	}
//	g_sys.status.work_mode&=0xFFFE;
//	g_sys.status.work_mode |= run_modetemp;
//}

//void valve_ctrl_update(void)
//{
//    extern sys_reg_st g_sys;
//	uint8_t i;
//	for(i = DI1;i < DI_MAX_CNT - 2;i++)
//	{
//		if((g_sys.config.dev_mask.din & (0x0001<< i)) != 0)
//		{
//			if(GPIO_ReadInputDataBit(in_pin_map_inst[i].pin_base,in_pin_map_inst[i].pin_id) != Bit_RESET)  
//			{
//				g_sys.status.valve.valve_ctrl_signal[i] = VALVE_ON;			
//			}
//			else
//			{
//				g_sys.status.valve.valve_ctrl_signal[i] = VALVE_OFF;
//			}
//		}
//		else
//		{
//			g_sys.status.valve.valve_ctrl_signal[i] = VALVE_OFF;
//		}
//	}
//}




