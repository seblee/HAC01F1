/*
 * FreeModbus Libary: BARE Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: demo.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "cmsis_os.h"  
#include "mb.h"
#include "mbport.h"
#include "string.h"
#include "mb_cb.h"
#include "eev_proc.h" 
#include "flash.h"
#include "i2c_bsp.h"
#include "sys_conf.h"
#include "global_var.h"
//#include "core_cmFunc.h"
static uint16_t mbs_read_reg(uint16_t read_addr);

/* ----------------------- Defines ------------------------------------------*/


/* ----------------------- Static variables ---------------------------------*/

//mb_reg_st mb_reg_inst;

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    return MB_ENOREG;
}

/*********************************************************
  * @name   eMBRegHoldingCB
	* @brief  modbus holding register operation call back function
	* @calls  None       
  * @called modbus protocal stack
  * @param  None
  * @retval The return value can be:
						@arg MB_ENOERR: 
            @arg MB_ENORES: 
*********************************************************/
eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{ 
	extern	conf_reg_map_st conf_reg_map_inst[];
	eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    uint16_t cmd_value;
	uint8_t temp = 0;
	usAddress--;   //FreeModbus功能函数中已经加1，为保证与缓冲区首地址一致，故减1
    //if((usAddress >= REG_HOLDING_START)&&(usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ))
	if(usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS )
    {
        iRegIndex = ( USHORT )( usAddress - REG_HOLDING_START );
        switch ( eMode )
        {
        // Pass current register values to the protocol stack. 
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                cmd_value = mbs_read_reg(iRegIndex);
				*pucRegBuffer++ = ( unsigned char )( cmd_value >> 8 );
			    *pucRegBuffer++ = ( unsigned char )( cmd_value & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

        // Update current register values with new values from the protocol stack.
        case MB_REG_WRITE:
			
            while( usNRegs > 0 )
            {
				if ((usAddress + usNRegs) <= (REG_HOLDING_START+CPAD_REG_HOLDING_WRITE_NREGS))
				{
					if((usAddress + usNRegs) >= (REG_HOLDING_START + CONFIG_REG_MAP_OFFSET))
					{
						cmd_value=(*pucRegBuffer) << 8;
						cmd_value+=*(pucRegBuffer+1);
						//写入保持寄存器中同时跟新到内存和flash保存 
						if(reg_map_write(conf_reg_map_inst[iRegIndex-CONFIG_REG_MAP_OFFSET].id,&cmd_value,1)==CPAD_ERR_NOERR)
						{									

							if(iRegIndex-CONFIG_REG_MAP_OFFSET == FACTORY_RESET)//复位默认参数
							{
								temp = (uint8_t)(*conf_reg_map_inst[FACTORY_RESET].reg_ptr);
								if(temp == 0x5A)
								{									
									__set_PRIMASK(1);
									I2C_EE_BufWrite(&temp,0,1);
									(*conf_reg_map_inst[FACTORY_RESET].reg_ptr) = 0;
									NVIC_SystemReset();	//复位
								}
							}
							iRegIndex++;
							usNRegs--;
						}
						else
						{
							
							eStatus = MB_ENORES;
							break;//	 while( usNRegs > 0 )
						}
					}
				}
				else
				{
					eStatus = MB_ENOREG;	
				}
            }
			
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}

/*********************************************************
  * @name   restore_factory
	* @brief  reset system parameters to default values
	* @calls  cmd_config_reg()          
  * @called mb_reg_default_load(),
						eMBClose(),
            eMBDisable(),
            eMBInit(),
						eMBEnable();
  * @param  None
  * @retval None
*********************************************************/
/*
static eMBErrorCode restore_factory(void)
{
		eMBErrorCode eStatus = MB_ENOERR;
		osDelay(50);
		eStatus = eMBDisable();
		if(eStatus != MB_ENOERR)
		{
				return eStatus;
		}	
	  eStatus = eMBClose();
		if(eStatus != MB_ENOERR)
		{
				return eStatus;
		}		
		//mb_reg_default_load();   
		eStatus = eMBInit( MB_RTU, mb_get_device_addr(), 0, mb_get_baudrate(), MB_PAR_NONE );
		if(eStatus != MB_ENOERR)
		{
				return eStatus;
		}	
		eStatus = eMBEnable();
		return eStatus;
}
*/
/*********************************************************
  * @name   sys_reset
	* @brief  system software reset
	* @calls  cmd_config_reg()          
  * @called eMBDisable(),
						eMBClose(),
						NVIC_SystemReset();
  * @param  None
  * @retval None
*********************************************************/
/*
static void sys_reset(void)
{
		osDelay(50);
		eMBDisable();
    eMBClose();
	  NVIC_SystemReset();	
}
*/
/*********************************************************
  * @name   cmd_config_reg
	* @brief  modbus configuration register command execution
	* @calls  mb_cmd_resolve()          
  * @called restore_factory(),
						sys_reset(),
            factory_mode_deactivate(),
						factory_mode_activate;
  * @param  None
  * @retval None
*********************************************************/
/*
static void cmd_config_reg(uint16_t regdata)
{
    switch(regdata)
		{
		    case (CMD_MB_RESET_DEFAULT):		//Reset system to default parameter
		    {
		        restore_factory();
						osDelay(50);
						//save_current_settings();
				    break;
		    }
		    case (CMD_MB_SAVE_FLASH):				//Save current modbus register content into flashrom
		    {
		        //save_current_settings();
				    break;
		    }				
		    case (CMD_MB_SYS_RESET):				//Reset system
		    {
		        sys_reset();
				    break;
		    }				
		    case (CMD_MB_USER_MODE):				//Switch to user mode, in which only limited modbus registers can be accessed 
		    {
		        //factory_mode_deactivate();
				    break;
		    }
		    case (CMD_MB_FACTORY_MODE):			//Switch to factory mode, in which all modbus registers can be accessed
		    {
		        //factory_mode_activate();
				    break;
		    }				
				default:
				{
				    break;
				}
		}
}
*/
/*********************************************************
  * @name   mb_get_baudrate
	* @brief  get current modbus communication baudrate
	* @calls  cmd_mb_stack_restart(),
						restore_factory(),
  * @called None
  * @param  None
  * @retval baudrate
*********************************************************/
ULONG mb_get_baudrate(uint16_t baudrate)
{
    ULONG ulBaudRate = 0;
	switch(baudrate)
	{
		case BAUD_4800:
				ulBaudRate = 4800;
				break;
		case BAUD_9600:
				ulBaudRate = 9600;
				break;
		case BAUD_19200:
				ulBaudRate = 19200;
				break;
		case BAUD_38400:
				ulBaudRate = 38400;
				break;
		case BAUD_57600:
				ulBaudRate = 57600;
				break;
		case BAUD_115200:
				ulBaudRate = 115200;
				break;
		default:
				ulBaudRate = 9600;
			break;
	}
//	ulBaudRate=9600;
	return ulBaudRate;
}

/*********************************************************
  * @name   mb_get_device_addr
	* @brief  get current modbus communication address
	* @calls  cmd_mb_stack_restart(),
						restore_factory(),          
  * @called None
  * @param  None
  * @retval device modbus slave address
*********************************************************/
uint8_t mb_get_device_addr(void)
{
    extern sys_reg_st					g_sys;
	return (UCHAR)g_sys.config.general.surv_addr;
//	return (1);
}

/*********************************************************
  * @name   mb_reg_status_calc
	* @brief  caculate board global status
	* @calls  
  * @called mb_reg_update()
  * @param  None
	* @retval	bit0:	high water level alarm
						bit1:	high humidifier current alarm
						bit2:	high water conductivity alarm 
*********************************************************/
/*
static uint16_t mb_reg_alarm_calc(void)
{
		uint16_t reg_status;
		extern daq_reg_st daq_reg_st_inst;
		reg_status = 0;
		if(daq_reg_st_inst.water_level != 0)
		{
				reg_status |= 0x0001;
		}
		if((int16_t)daq_reg_st_inst.hum_current >= ALARM_HC_TH)
		{
				reg_status |= 0x0002;
		}
		if((int16_t)daq_reg_st_inst.conductivity < ALARM_WQ_TH)
		{
				reg_status |= 0x0004;
		}
		return reg_status;
}
*/

/*********************************************************
  * @name   mb_reg_update
	* @brief  update modbus user registers according to internal status and data aquired
	* @calls  mb_reg_init(),
						com_proc(),
  * @called None
  * @param  None
  * @retval None
*********************************************************/
void mb_reg_update(void)
{
	 /*
	extern daq_reg_st daq_reg_st_inst;
		mb_reg_inst.usRegHoldingBuf[20] = mb_reg_alarm_calc();
    mb_reg_inst.usRegHoldingBuf[21] = daq_reg_st_inst.conductivity;
    mb_reg_inst.usRegHoldingBuf[22] = daq_reg_st_inst.hum_current;
    mb_reg_inst.usRegHoldingBuf[23] = daq_reg_st_inst.water_level;
	*/
}

/*********************************************************
  * @name   mb_reg_init
	* @brief  modbus registers initialization, load data from flashrom if is programed, otherwise, set modbus registers with default value
	* @calls  mb_reg_default_load(),
						com_proc(),          
  * @called mb_reg_update()
  * @param  None
  * @retval None
*********************************************************/

/*********************************************************
  * @name   mb_cmd_resolve
	* @brief  scan writable modbus registers, if modified, call according handler
	* @calls  com_proc();          
  * @called cmd_mb_stack_restart(),
						cmd_config_reg(),
						save_current_settings();
  * @param  None
  * @retval None
*********************************************************/
/*
void mb_cmd_resolve(void)
{
		uint16_t i;
		for(i=0;i<REG_HOLDING_NREGS;i++)
		{
				if(mb_reg_inst.ucRegHoldingBufStatus[i] != 0)
				{						
						switch(i)
						{				
								case (11):	//address change
								{
										cmd_mb_stack_restart();
										break;
								}

								{
										cmd_mb_stack_restart();
										break;
								}
								case (19):	//conf reg command resolving
								{
										cmd_config_reg(mb_reg_inst.usRegHoldingBuf[i]);
										mb_reg_inst.usRegHoldingBuf[i] = 0;
										break;
								}								
								default:
								{
										break;
								}								
						}
				}
				mb_reg_inst.ucRegHoldingBufStatus[i] = 0;
		}
}
*/
static uint16_t mbs_read_reg(uint16_t read_addr)
{
	extern	conf_reg_map_st conf_reg_map_inst[];
	extern  sts_reg_map_st status_reg_map_inst[];
	//if((read_addr >= CONFIG_REG_MAP_OFFSET)&&(read_addr<CONF_REG_MAP_NUM + CONFIG_REG_MAP_OFFSET))
	if(read_addr<CONF_REG_MAP_NUM + CONFIG_REG_MAP_OFFSET)
	{
		 return(*(conf_reg_map_inst[read_addr- CONFIG_REG_MAP_OFFSET].reg_ptr));
	}
	else if((STATUS_REG_MAP_OFFSET <= read_addr)&&(read_addr<( STATUS_REG_MAP_OFFSET+STATUS_REG_MAP_NUM)))
	{
		 return(*(status_reg_map_inst[read_addr-STATUS_REG_MAP_OFFSET].reg_ptr));	
	}
	else 
	{
		return(ABNORMAL_VALUE);
	}
}
