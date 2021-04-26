#ifndef __EEV_PROC_H
#define	__EEV_PROC_H
#include <stdint.h>

#define  HOLD_TIME3S_CNT		2u

enum
{
	disalbe = 0,
	enable
};

enum
{
	Fixed=0,
	Inverter ,
};

typedef struct
{
		uint16_t conductivity;
		uint16_t hum_current;
		uint16_t water_level;
}daq_reg_st;	

void daq_gvar_update(void);
#endif/*__EEV_PROC_H*/
