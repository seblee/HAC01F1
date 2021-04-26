#ifndef		__DIO_H
#define 	__DIO_H

#include <stdint.h>
#include "stm32f0xx.h"
#include "sys_conf.h"

#define DI_MAX_CNT	5u
#define DO_MAX_CNT  2u
#define DI_MASK			0x1F

enum
{
	SW1,		//内部上拉
	SW2,
	SW3,
	DI1,
	DI2,
};
#define Para_M			0x03
#define StartEnable	0x04
#define FanOn				0x08

typedef struct
{
    uint16_t pin_id;
    void *pin_base;
} pin_map_st;


extern void drv_dio_bsp_init(void);
extern void work_mode_update(void);
extern void valve_ctrl_update(void);
extern void drv_dio_init(void);
extern void DI_update(void);
#endif




