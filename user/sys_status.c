#include "sys_conf.h"
void sys_set_remap_status(uint8_t sbit_pos, uint8_t bit_action)
{
		extern sys_reg_st			g_sys;
		if(bit_action == 1)
		{
				g_sys.status.status_remap |= (0x0001<<sbit_pos);		
		}
		else
		{
				g_sys.status.status_remap &= ~(0x0001<<sbit_pos);
		}
}

uint16_t sys_get_remap_status(uint8_t rbit_pos)
{
		extern sys_reg_st			g_sys;
		return ((g_sys.status.status_remap >> rbit_pos) & 0x0001);		
}


