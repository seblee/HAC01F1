#ifndef __SYS_STATUS
#define	__SYS_STATUS
#include "sys_def.h"

void sys_set_remap_status(uint8_t sbit_pos, uint8_t bit_action);
uint16_t sys_get_remap_status(uint8_t reg_no,uint8_t rbit_pos);

#endif //	__SYS_CONF
